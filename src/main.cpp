#include <cmath>
#include <algorithm>
#include <iostream>
#include <memory>
#include <numbers>
#include <string>
#include <vector>

#include <clip.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>
#include <nfd.hpp>
#include <stb_image.h>
#include <font_source_han_sans_tc_regular_bsae85.h>

#include "algorithms.h"
#include "image.h"
#include "image_window.h"
#include "utility.h"

std::vector<std::shared_ptr<ImageWindow>> image_windows;
static const ImVec4 color_error(1.f, 0.f, 0.f, 1.f);

constexpr int convolution_kernel_template_num = 6;

const char *convolution_kernel_template_name[] = {
    "Identity",
    "Edge detection (4-way)",
    "Edge detection (8-way)",
    "Sharpen",
    "Gaussian blur 3x3",
    "emboss"
};

const int convolution_kernel_template_size[] = {3, 3, 3, 3, 3, 3};

const float convolution_kernel_template_elem[][9] = {
    {
        0.f, 0.f, 0.f,
        0.f, 1.f, 0.f,
        0.f, 0.f, 0.f
    }, {
         0.f, -1.f,  0.f,
        -1.f,  4.f, -1.f,
         0.f, -1.f,  0.f
    }, {
        -1.f, -1.f, -1.f,
        -1.f,  8.f, -1.f,
        -1.f, -1.f, -1.f
    }, {
         0.f, -1.f,  0.f,
        -1.f,  5.f, -1.f,
         0.f, -1.f,  0.f
    }, {
        0.0625f, 0.125f, 0.0625f,
        0.125f,  0.25f,  0.125f,
        0.0625f, 0.125f, 0.0625f
    }, {
        -2.f, -1.f, 0.f,
        -1.f,  1.f, 1.f,
         0.f,  1.f, 2.f
    }
};

static void glfw_error_callback(int error, const char *mesage) {
    fprintf(stderr, "GLFW Error [%d]: %s\n", error, mesage);
}

inline void display_image_helper(const std::shared_ptr<Image> image, const std::string &title="") {
    image_windows.emplace_back(std::make_shared<ImageWindow>(image, title));
}

std::string open_image_path() {
    if (NFD::Init() != NFD_OKAY)
        return std::string();

    constexpr nfdfiltersize_t filter_list_size = 6;
    const nfdfilteritem_t filter_list[filter_list_size] = {
        {"All supported image", "bmp,jpg,jpeg,gif,png,ppm,pgm"},
        {"BMP image", "bmp"},
        {"JPG image", "jpg,jpeg"},
        {"GIF image", "gif"},
        {"PNG image", "png"},
        {"PPM image", "ppm,pgm"}};

    nfdchar_t *nfd_filepath = nullptr;
    nfdresult_t result = NFD::OpenDialog(nfd_filepath, filter_list, filter_list_size, nullptr);

    if (result == NFD_ERROR)
        std::cout << "NFD ERROR: " << NFD::GetError() << std::endl;

    if (result == NFD_CANCEL)
        return std::string();

    std::string filename(nfd_filepath);
    NFD::FreePath(nfd_filepath);

    NFD::Quit();
    return filename;
}

std::string save_image_path() {
    if (NFD::Init() != NFD_OKAY)
        return std::string();

    constexpr nfdfiltersize_t filter_list_size = 3;
    const nfdfilteritem_t filter_list[filter_list_size] = {
        {"All supported image", "jpg,png"},
        {"JPG image", "jpg"},
        {"PNG image", "png"}};

    nfdchar_t *nfd_filepath = nullptr;
    nfdresult_t result = NFD::SaveDialog(nfd_filepath, filter_list, filter_list_size);

    if (result == NFD_ERROR)
        std::cout << "NFD ERROR: " << NFD::GetError() << std::endl;

    if (result == NFD_CANCEL)
        return std::string();

    std::string filename(nfd_filepath);
    NFD::FreePath(nfd_filepath);

    NFD::Quit();
    return filename;
}

void handle_open_image_from_file() {
    std::string filepath = open_image_path();
    if (filepath.empty()) {
        std::cout << "Open image canceled." << std::endl;
        return;
    }

    std::cout << "Open image: \"" << filepath << "\"" << std::endl;
    std::shared_ptr<Image> image = std::make_shared<Image>(filepath);
    if (!image->good()) {
        std::cout << "Error: Open image \"" << filepath << "\" failed!" << std::endl;
        return;
    }
    display_image_helper(image, filepath);
}

void handle_save_iamge(const std::shared_ptr<Image> image) {
    std::string filepath = save_image_path();
    if (filepath.empty())
        return;
    std::cout << "Save image: \"" << filepath << "\"" << std::endl;
    const bool result = image->saveToFile(filepath);
    if (!result) {
        std::cout << "Error: Save image \"" << filepath << "\" failed!" << std::endl;
        return;
    }
}

void handle_open_image_from_clipboard() {
    if (!clip::has(clip::image_format())) {
        std::cout << "Error: Clipboard doesn't contain an image!" << std::endl;
        return;
    }

    clip::image img;
    if (!clip::get_image(img)) {
        std::cout << "Error: Getting image from clipboard failed!" << std::endl;
        return;
    }

    const clip::image_spec spec = img.spec();
    std::shared_ptr<Image> image = std::make_shared<Image>(spec.width, spec.height);

    // save pixel data
    for (int y = 0; y < spec.height; ++y) {
        const uint8_t *p = (uint8_t *) (img.data() + y * spec.bytes_per_row);
        for (int x = 0; x < spec.width; ++x) {
            const uint64_t pixel = (*((uint64_t *) p));
            image->pixel(x, y)[Image::R] = (pixel & spec.red_mask) >> spec.red_shift;
            image->pixel(x, y)[Image::G] = (pixel & spec.green_mask) >> spec.green_shift;
            image->pixel(x, y)[Image::B] = (pixel & spec.blue_mask) >> spec.blue_shift;
            if (spec.alpha_mask)
                image->pixel(x, y)[Image::A] = (pixel & spec.alpha_mask) >> spec.alpha_shift;
            p += spec.bits_per_pixel / 8;
        }
    }

    image->loadToTexture();
    display_image_helper(image, "clipboard");
}

void handle_copy_image_title(const std::shared_ptr<ImageWindow> image_window) {
    clip::set_text(image_window->getDisplayedTitle());
}

void handle_copy_image_to_clipboard(const std::shared_ptr<Image> image) {
    const uint8_t *data = image->data();
    clip::image_spec spec;
    spec.width = image->getImageWidth();
    spec.height = image->getImageHeight();
    spec.bits_per_pixel = 32;
    spec.bytes_per_row = spec.width * 4;
    spec.red_mask    = 0x000000ff;
    spec.green_mask  = 0x0000ff00;
    spec.blue_mask   = 0x00ff0000;
    spec.alpha_mask  = 0xff000000;
    spec.red_shift   = 0;
    spec.green_shift = 8;
    spec.blue_shift  = 16;
    spec.alpha_shift = 24;
    const clip::image img(data, spec);
    clip::set_image(img);
}

void handle_gray_histogram(const std::shared_ptr<Image> image) {
    std::cout << "compute histogram" << std::endl;
    std::shared_ptr<Image> gray_image = std::make_shared<Image>();
    float histogram[256] = {};
    generate_gray_image_and_histogram(image, gray_image, histogram);
    display_image_helper(gray_image, "gray image");

    std::cout << "generate histogram image" << std::endl;
    const std::shared_ptr<Image> histogram_image = generate_histogram_image(histogram);
    display_image_helper(histogram_image, "gray histogram");
}

void handle_gaussian_noise(const std::shared_ptr<Image> image, int sigma) {
    const float sigma_normalized = sigma / 255.f;

    // generate noise
    const int num_pixels = image->getImageWidth() * image->getImageHeight();
    float *noise = new float[num_pixels];
    generate_gaussian_noise(noise, num_pixels, sigma_normalized);

    // generate image with noise added
    std::shared_ptr<Image> image_with_noise = std::make_shared<Image>(image->getImageWidth(), image->getImageHeight());
    for (int y = 0; y < image->getImageHeight(); ++y) {
        for (int x = 0; x < image->getImageWidth(); ++x) {
            const int i = y * image->getImageWidth() + x;
            image_with_noise->pixel(x, y)[Image::R] = clamp(image->pixel(x, y)[Image::R] + 255 * noise[i], 0.f, 255.f);
            image_with_noise->pixel(x, y)[Image::G] = clamp(image->pixel(x, y)[Image::G] + 255 * noise[i], 0.f, 255.f);
            image_with_noise->pixel(x, y)[Image::B] = clamp(image->pixel(x, y)[Image::B] + 255 * noise[i], 0.f, 255.f);
            image_with_noise->pixel(x, y)[Image::A] = image->pixel(x, y)[Image::A];
        }
    }
    image_with_noise->loadToTexture();
    display_image_helper(image_with_noise, "image with noise");

    // draw histogram of noise
    for (int i = 0; i < num_pixels; ++i)
        noise[i] += 0.5f;
    float histogram[256] = {};
    generate_histogram_from_array(noise, num_pixels, histogram);
    std::shared_ptr<Image> noise_histogram_image = generate_histogram_image(histogram);
    display_image_helper(noise_histogram_image, "noise histogram");

    // clean
    delete [] noise;
}

void handle_resize_image(const std::shared_ptr<Image> image, int width, int height) {
    std::shared_ptr<Image> resized_image = std::make_shared<Image>(*image);
    resized_image->resize(width, height);
    display_image_helper(resized_image, "resized image");
}

void handle_haar_wavelet_transform(const std::shared_ptr<Image> image, int level, float scale=1.f) {
    // check
    if (level < 0) {
        std::cout << "level can't be negative!" << std::endl;
        return;
    }

    // to grey
    std::shared_ptr<Image> in_image = std::make_shared<Image>(*image);
    generate_gray_image_and_histogram(image, in_image, nullptr);

    // resize image
    in_image->resize(
        nearest_power_of_2(in_image->getImageWidth()),
        nearest_power_of_2(in_image->getImageHeight()));

    std::shared_ptr<Image> out_image = haar_wavelet_transform(in_image, level, scale);
    out_image->loadToTexture();

    display_image_helper(out_image, "haar wavelet result");
}

void handle_histogram_equalization(const std::shared_ptr<Image> image) {
    // input
    std::shared_ptr<Image> input_image = std::make_shared<Image>();
    float histogram[256] = {};
    generate_gray_image_and_histogram(image, input_image, histogram);
    const std::shared_ptr<Image> input_image_histogram = generate_histogram_image(histogram);
    display_image_helper(input_image, "histogram equalization - input");
    display_image_helper(input_image_histogram, "histogram equalization - input histogram");

    // process
    std::shared_ptr<Image> output_image = histogram_equalization(input_image);

    // output
    generate_gray_image_and_histogram(output_image, nullptr, histogram);
    const std::shared_ptr<Image> output_image_histogram = generate_histogram_image(histogram);
    display_image_helper(output_image, "histogram equalization - output");
    display_image_helper(output_image_histogram, "histogram equalization - output histogram");
}

void handle_convolution(const std::shared_ptr<Image> image, int kernel_size, const std::shared_ptr<float[]> kernel,
        ConvolutionEdgeHandlingMethod edge_handling_method) {
    std::shared_ptr<Image> result = image_convolution(image, kernel_size, kernel.get(), edge_handling_method);
    display_image_helper(result, "convolution result");
}

int main(int argc, const char **argv) {

    // init GLFW

    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()){
        std::cout << "GLFW initialization failed!" << std::endl;
        return 1;
    }

    GLFWwindow *window = glfwCreateWindow(1280, 720, "Advanced Image Processor", nullptr, nullptr);

    if (window == nullptr){
        std::cout << "GLFW window initialization failed!" << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // init ImGUI

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 8.f;
    style.ChildRounding = 6.f;
    style.FrameRounding = 6.f;
    style.PopupRounding = 6.f;
    style.GrabRounding = 6.f;
    style.ChildRounding = 6.f;

    constexpr float font_size = 18.f;
    ImFont* font = io.Fonts->AddFontFromMemoryCompressedBase85TTF(
            font_source_han_sans_tc_regular_compressed_data_base85, font_size, nullptr,
            io.Fonts->GetGlyphRangesChineseFull());

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    const ImVec4 clear_color(0.45f, 0.55f, 0.60f, 1.00f);

    // init image variables

    std::shared_ptr<Image> default_image = std::make_shared<Image>("image.png");
    if (default_image->good())
        display_image_helper(default_image, "default image.png");

    std::shared_ptr<Image> formula_image = std::make_shared<Image>(400, 500);
    for (int y = 0; y < formula_image->getImageHeight(); ++y) {
        for (int x = 0; x < formula_image->getImageWidth(); ++x) {
            // fill some color
            formula_image->pixel(x, y)[Image::R] = x * 2 % 40 + y * 57 % 203 - y * int(exp(x) * 4) % 23;
            formula_image->pixel(x, y)[Image::G] = x * 54 % 87 + int(log(y) * 13) % 147 - y * x * 2 % 76;
            formula_image->pixel(x, y)[Image::B] = int(atan(x) * 73) % 143 + y * 46 % 86 - y * x * 54 % 31;
        }
    }
    formula_image->loadToTexture();
    display_image_helper(formula_image, "default math formula image");

    // window loop

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // main menu bar

        static bool show_imgui_demo_window = false;
        static bool show_fps = true;

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open image from file...")) { handle_open_image_from_file(); }
                if (ImGui::MenuItem("Open image from clipboard")) { handle_open_image_from_clipboard(); }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) { glfwSetWindowShouldClose(window, GLFW_TRUE); }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Debug")) {
                ImGui::Checkbox("Show FPS", &show_fps);
                ImGui::Checkbox("Show ImGUI Demo Window", &show_imgui_demo_window);
                ImGui::EndMenu();
            }
            if (show_fps) {
                ImGui::PushID("show fps");
                ImGui::InvisibleButton("", ImVec2(-80, 20));
                ImGui::Separator();
                ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, .5f), "FPS: %.1f", io.Framerate);
                ImGui::PopID();
            }
            ImGui::EndMainMenuBar();
        }

        if (show_imgui_demo_window) {
            ImGui::ShowDemoWindow(&show_imgui_demo_window);
        }

        int image_window_index = 0;
        while (image_window_index < image_windows.size()) {
            std::shared_ptr<ImageWindow> image_window = image_windows[image_window_index];

            // image window
            if (image_window->is_first_seen)
                ImGui::SetNextWindowPos(image_window->computeDefaultPosition());

            const bool is_expanded = ImGui::Begin(
                image_window->getRenderedTitle().c_str(),
                &image_window->is_open,
                ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_HorizontalScrollbar);

            // check should close
            if (!image_window->is_open) {
                image_windows.erase(image_windows.begin() + image_window_index);
                ImGui::End();
                continue;
            }

            if (is_expanded) {
                // menu bar
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("File")) {
                        if (ImGui::MenuItem("Save as...")) {
                            handle_save_iamge(image_window->getImage());
                        }
                        ImGui::Separator();
                        if (ImGui::MenuItem("Copy title to clipboard")) {
                            handle_copy_image_title(image_window);
                        }
                        if (ImGui::MenuItem("Copy image to clipboard")) {
                            handle_copy_image_to_clipboard(image_window->getImage());
                        }
                        ImGui::Separator();
                        if (ImGui::MenuItem("Close")) { image_window->is_open = false; }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Zoom")) {
                        ImGui::RadioButton("Original",      (int *) &image_window->scale_type, ImageWindow::SCALE_ORIGINAL);
                        ImGui::RadioButton("Fill",          (int *) &image_window->scale_type, ImageWindow::SCALE_FILL);
                        ImGui::RadioButton("Fit Width",     (int *) &image_window->scale_type, ImageWindow::SCALE_FIT_WIDTH);
                        ImGui::RadioButton("Fit Height",    (int *) &image_window->scale_type, ImageWindow::SCALE_FIT_HEIGHT);
                        ImGui::RadioButton("Fit Window",    (int *) &image_window->scale_type, ImageWindow::SCALE_FIT_WINDOW);
                        ImGui::RadioButton("Custom Scale:", (int *) &image_window->scale_type, ImageWindow::SCALE_CUSTOM_SCALE);
                        ImGui::SameLine();

                        bool is_custom_scale = false;
                        float scale_factor = image_window->scale_factor;
                        constexpr float scale_button_step = 1.25f;
                        bool error = false;
                        if (ImGui::Button("-")) {
                            is_custom_scale = true;
                            scale_factor /= scale_button_step;
                        }
                        ImGui::SameLine();
                        ImGui::PushItemWidth(80.f);
                        float scale_factor_percent = scale_factor * 100.f;
                        if (ImGui::DragFloat("##zoom_scale", &scale_factor_percent,
                                4.f, 0.001f, 10000.f, "%.1f%%", ImGuiSliderFlags_Logarithmic)) {
                            is_custom_scale = true;
                            scale_factor = scale_factor_percent / 100.f;
                            if (scale_factor <= 0) error = true;
                        }
                        ImGui::PopItemWidth();
                        ImGui::SameLine();
                        if (ImGui::Button("+")) {
                            is_custom_scale = true;
                            scale_factor *= scale_button_step;
                        }
                        if (is_custom_scale) {
                            if (error) {
                                ImGui::TextColored(color_error, "Error: scale must > 0");
                            } else {
                                image_window->scale_type = ImageWindow::SCALE_CUSTOM_SCALE;
                                image_window->scale_factor = scale_factor;
                            }
                        }

                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Operations")) {
                        if (ImGui::BeginMenu("Resize")) {
                            static int new_size[2] = {256, 256};
                            bool error = false;
                            ImGui::InputInt2("W x H", new_size);
                            if (new_size[0] <= 0 || new_size[1] <= 0) {
                                ImGui::TextColored(color_error, "Error: width and height must > 0");
                                error = true;
                            }
                            if (error) ImGui::BeginDisabled();
                            if (ImGui::Button("Apply")) {
                                handle_resize_image(image_window->getImage(), new_size[0], new_size[1]);
                            }
                            if (error) ImGui::EndDisabled();
                            ImGui::EndMenu();
                        }
                        if (ImGui::MenuItem("Gray Histogram")) {
                            handle_gray_histogram(image_window->getImage());
                            ImGui::EndMenu();
                        }
                        if (ImGui::BeginMenu("Gaussian Noise")) {
                            static int sigma = 32;
                            constexpr float drag_speed = 0.2f;
                            bool error = false;
                            ImGui::DragScalar("sigma", ImGuiDataType_U8, &sigma, drag_speed);
                            if (sigma < 0) {
                                ImGui::TextColored(color_error, "Error: sigma must >= 0");
                                error = true;
                            }
                            if (error) ImGui::BeginDisabled();
                            if (ImGui::Button("Apply")) {
                                handle_gaussian_noise(image_window->getImage(), sigma);
                            }
                            if (error) ImGui::EndDisabled();
                            ImGui::EndMenu();
                        }
                        if (ImGui::BeginMenu("HAAR Wavelet Transform")) {
                            static int level = 2;
                            static float scale = 1.f;
                            constexpr float drag_speed = 0.4f;
                            bool error = false;
                            ImGui::InputInt("level", &level);
                            if (level < 0) {
                                ImGui::TextColored(color_error, "Error: level must >= 0");
                                error = true;
                            }
                            ImGui::DragFloat("scale", &scale, drag_speed, 1.f, 256.f, nullptr, ImGuiSliderFlags_Logarithmic);
                            if (scale <= 0) {
                                ImGui::TextColored(color_error, "Error: scale must > 0");
                                error = true;
                            }
                            if (error) ImGui::BeginDisabled();
                            if (ImGui::Button("Apply")) {
                                handle_haar_wavelet_transform(image_window->getImage(), level, scale);
                            }
                            if (error) ImGui::EndDisabled();
                            ImGui::EndMenu();
                        }
                        if (ImGui::MenuItem("Histogram Equalization")) {
                            handle_histogram_equalization(image_window->getImage());
                        }
                        if (ImGui::BeginMenu("Convolution")) {
                            static int template_id = 0;
                            static ConvolutionEdgeHandlingMethod edge_handling_method = ConvolutionEdgeHandlingMethod::EXTEND;
                            static int kernel_size = 3;
                            static std::shared_ptr<float[]> kernel;
                            static bool need_load_template = true;
                            static bool need_reset_kernel = true;
                            int new_kernel_size = kernel_size;
                            constexpr int input_width = 80;
                            if (ImGui::BeginCombo("template", convolution_kernel_template_name[template_id])) {
                                for (int i = 0; i < convolution_kernel_template_num; ++i) {
                                    const bool is_selected = (template_id == i);
                                    if (ImGui::Selectable(convolution_kernel_template_name[i], is_selected)) {
                                        template_id = i;
                                        new_kernel_size = convolution_kernel_template_size[template_id];
                                        need_reset_kernel = true;
                                        need_load_template = true;
                                    }
                                    if (is_selected)
                                        ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }
                            int new_edge_handling_method_int = (int) edge_handling_method;
                            if (ImGui::Combo("edge handling", &new_edge_handling_method_int, "extend\0wrap\0mirror\0\0")) {
                                edge_handling_method = (ConvolutionEdgeHandlingMethod) new_edge_handling_method_int;
                            }
                            if (ImGui::InputInt("kernel size", &new_kernel_size, 2)) {
                                if (new_kernel_size > 0 && new_kernel_size % 2 == 1) {
                                    need_reset_kernel = true;
                                } else {
                                    need_reset_kernel = false;
                                    new_kernel_size = kernel_size;
                                }
                            }
                            if (need_reset_kernel) {
                                std::shared_ptr<float[]> new_kernel(new float[new_kernel_size * new_kernel_size]());
                                // copy kernel from template
                                if (need_load_template) {
                                    for (int i = 0; i < new_kernel_size * new_kernel_size; ++i) {
                                        new_kernel[i] = convolution_kernel_template_elem[template_id][i];
                                    }
                                    need_load_template = false;

                                // copy existing kernel to the new one
                                } else if (kernel != nullptr) {
                                    const int offset = (new_kernel_size - kernel_size) / 2;
                                    for (int y = 0; y < new_kernel_size; ++y) {
                                        const int old_y = y - offset;
                                        for (int x = 0; x < new_kernel_size; ++x) {
                                            const int old_x = x - offset;
                                            if (old_x >= 0 && old_x < kernel_size && old_y >= 0 && old_y < kernel_size)
                                                new_kernel[y * new_kernel_size + x] = kernel[old_y * kernel_size + old_x];
                                        }
                                    }
                                }
                                kernel = new_kernel;
                                kernel_size = new_kernel_size;
                                need_reset_kernel = false;
                            }
                            const ImGuiTableFlags table_flags =
                                    ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX |
                                    ImGuiTableFlags_NoPadOuterX;
                            ImGui::Text("kernel:");
                            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.f, 0.f));
                            if (ImGui::BeginTable("kernel", kernel_size, table_flags)) {
                                for (int y = 0; y < kernel_size; y++) {
                                    ImGui::TableNextRow();
                                    for (int x = 0; x < kernel_size; x++) {
                                        ImGui::TableSetColumnIndex(x);
                                        const std::string label = "##" + std::to_string(y) + "-" + std::to_string(x);
                                        ImGui::PushItemWidth(input_width);
                                        ImGui::InputFloat(label.c_str(), &kernel[y * kernel_size + x]);
                                        ImGui::PopItemWidth();
                                    }
                                }
                                ImGui::EndTable();
                            }
                            ImGui::PopStyleVar();
                            if (ImGui::Button("Apply")) {
                                handle_convolution(image_window->getImage(), kernel_size, kernel, edge_handling_method);
                            }
                            ImGui::EndMenu();
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // set initial zoom mode
                const ImVec2 image_size = ImVec2(
                    image_window->getImage()->getImageWidth(), image_window->getImage()->getImageHeight());
                ImVec2 render_size;
                if (image_window->is_first_seen) {
                    constexpr float max_image_ratio = 0.75f;
                    const ImVec2 max_size = ImVec2(
                        max_image_ratio * ImGui::GetMainViewport()->WorkSize.x,
                        max_image_ratio * ImGui::GetMainViewport()->WorkSize.y);
                    render_size = compute_max_target_size(image_size, max_size);
                    if (render_size.x != image_size.x && render_size.y != image_size.y) {
                        image_window->scale_type = ImageWindow::SCALE_FIT_WINDOW;
                    }
                } else {
                    render_size = image_window->computeImageRenderSize(ImVec2(
                        ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x,
                        ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y));
                }
                // update scale factor
                image_window->scale_factor = render_size.x / image_size.x;

                // draw image
                ImGui::Image((void *)(intptr_t)image_window->getImage()->getTextureId(), render_size);
            }
            ImGui::End();

            if (image_window->is_first_seen)
                image_window->is_first_seen = false;
            ++image_window_index;
        }

        // rendering

        int window_w = 0;
        int window_h = 0;
        glfwGetFramebufferSize(window, &window_w, &window_h);
        glViewport(0, 0, window_w, window_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    // clenaup ImGUI

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // cleanup GLFW

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

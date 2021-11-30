#include <cmath>
#include <algorithm>
#include <iostream>
#include <memory>
#include <numbers>
#include <string>
#include <vector>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>
#include <nfd.hpp>
#include <stb_image.h>
#include <font_source_han_sans_tc_regular_bsae85.h>

#include "image.h"
#include "image_window.h"
#include "utility.h"

std::vector<std::shared_ptr<ImageWindow>> image_windows;

static void glfw_error_callback(int error, const char *mesage) {
    fprintf(stderr, "GLFW Error [%d]: %s\n", error, mesage);
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

void handle_open_image() {
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
    image_windows.emplace_back(std::make_shared<ImageWindow>(image, filepath));
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

uint8_t to_gray_average(const uint8_t *pixel) {
    int a = round((((int) pixel[Image::R]) + ((int) pixel[Image::G]) + ((int) pixel[Image::B])) / 3.0f);
    // std::cout << a << std::endl;
    return a;
}

void generate_gray_image_and_histogram(const std::shared_ptr<Image> image, std::shared_ptr<Image> out_image, float *histogram) {
    if (out_image != nullptr)
        out_image->init(image->getImageWidth(), image->getImageHeight());

    int level_count[256] = {};

    // transform to gray scale
    for (int y = 0; y < image->getImageHeight(); ++y) {
        for (int x = 0; x < image->getImageWidth(); ++x) {
            const uint8_t gray_level = to_gray_average(image->pixel(x, y));
            ++level_count[gray_level];
            if (out_image != nullptr) {
                out_image->pixel(x, y)[Image::R] = gray_level;
                out_image->pixel(x, y)[Image::G] = gray_level;
                out_image->pixel(x, y)[Image::B] = gray_level;
            }
        }
    }
    if (out_image != nullptr) {
        out_image->loadToTexture();
    }

    // normalize histogram
    if (histogram != nullptr) {
        const int max_num = *std::max_element(level_count, level_count + 256);

        for (int i = 0; i < 256; ++i) {
            histogram[i] = (double) level_count[i] / max_num;
        }
    }
}

std::shared_ptr<Image> generate_histogram_image(const float *histogram) {
    constexpr int image_size = 300;
    std::shared_ptr<Image> image = std::make_shared<Image>(image_size, image_size);

    // draw the histogram
    constexpr int histogram_start = (image_size - 256) / 2;
    constexpr int histogram_end = histogram_start + 256;
    for (int level = 0; level < 256; ++level) {
        const int h = histogram_start + 256 * (1.0f - histogram[level]);
        // draw background
        for (int y = histogram_start; y < h; ++y) {
            image->pixel(histogram_start + level, y)[Image::R] = 127;
            image->pixel(histogram_start + level, y)[Image::G] = 127;
            image->pixel(histogram_start + level, y)[Image::B] = 127;
        }
        // draw bar
        for (int y = h; y < histogram_end; ++y) {
            image->pixel(histogram_start + level, y)[Image::R] = 0;
            image->pixel(histogram_start + level, y)[Image::G] = 0;
            image->pixel(histogram_start + level, y)[Image::B] = 0;
        }
    }
    image->loadToTexture();
    return image;
}

void handle_gray_histogram(const std::shared_ptr<Image> image) {
    std::cout << "compute histogram" << std::endl;
    std::shared_ptr<Image> gray_image = std::make_shared<Image>();
    float histogram[256] = {};
    generate_gray_image_and_histogram(image, gray_image, histogram);
    image_windows.emplace_back(std::make_shared<ImageWindow>(gray_image, "gray image"));

    std::cout << "generate histogram image" << std::endl;
    const std::shared_ptr<Image> histogram_image = generate_histogram_image(histogram);
    image_windows.emplace_back(std::make_shared<ImageWindow>(histogram_image, "gray histogram"));
}

void generate_gaussian_noise(float *out_noise, int count, float sigma) {
    using std::numbers::pi;

    for (int i = 0; i < count; i += 2) {
        // generate uniform random number pairs
        const double r = random_float();
        const double phi = random_float();

        // Box-Muller transform
        const double ln_r = (r == 0.) ? 0. : log(r);
        const double z1 = sigma * cos(2. * pi * phi) * sqrt(-2. * ln_r);
        const double z2 = sigma * sin(2. * pi * phi) * sqrt(-2. * ln_r);

        // save noise
        out_noise[i] = z1;
        if (i + 1 < count)
            out_noise[i + 1] = z2;
    }
}

void generate_histogram_from_array(const float *noise, int count, float *histogram) {
    int level_count[256] = {};

    // transform to gray scale
    for (int i = 0; i < count; ++i) {
        const uint8_t gray_level = 255 * clamp(noise[i], 0.f, 1.f);
        ++level_count[gray_level];
    }

    // normalize histogram
    const int max_num = *std::max_element(level_count, level_count + 256);
    for (int i = 0; i < 256; ++i) {
        histogram[i] = (double) level_count[i] / max_num;
    }
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
    image_windows.emplace_back(std::make_shared<ImageWindow>(image_with_noise, "image with noise"));

    // draw histogram of noise
    for (int i = 0; i < num_pixels; ++i)
        noise[i] += 0.5f;
    float histogram[256] = {};
    generate_histogram_from_array(noise, num_pixels, histogram);
    std::shared_ptr<Image> noise_histogram_image = generate_histogram_image(histogram);
    image_windows.emplace_back(std::make_shared<ImageWindow>(noise_histogram_image, "noise histogram"));

    // clean
    delete [] noise;
}

void handle_resize_image(const std::shared_ptr<Image> image, int width, int height) {
    std::shared_ptr<Image> resized_image = std::make_shared<Image>(*image);
    resized_image->resize(width, height);
    image_windows.emplace_back(std::make_shared<ImageWindow>(resized_image, "resized image"));
}

std::shared_ptr<Image> haar_wavelet_transform(const std::shared_ptr<Image> image, int level, float scale=1.f) {
    if (level < 0) return nullptr;
    if (level == 0) return std::make_shared<Image>(*image);

    int cur_w = image->getImageWidth();
    int cur_h = image->getImageHeight();

    std::shared_ptr<Image> in_image = image;
    std::shared_ptr<Image> out_image = image;

    for (int current_level = 0; current_level < level; ++current_level) {
        out_image = std::make_shared<Image>(*in_image);
        const int half_w = cur_w / 2;
        const int half_h = cur_h / 2;

        for (int y = 0; y < half_h; ++y) {
            for (int x = 0; x < half_w; ++x) {
                const uint8_t a = in_image->pixel(2 * x    , 2 * y    )[Image::R];
                const uint8_t b = in_image->pixel(2 * x + 1, 2 * y    )[Image::R];
                const uint8_t c = in_image->pixel(2 * x    , 2 * y + 1)[Image::R];
                const uint8_t d = in_image->pixel(2 * x + 1, 2 * y + 1)[Image::R];

                const uint8_t ll = clamp((int) (   ((int) a + (int) b + (int) c + (int) d) / 4                ), 0, 255);
                const uint8_t hl = clamp((int) (abs((int) a - (int) b + (int) c - (int) d) / 4 * scale        ), 0, 255);
                const uint8_t lh = clamp((int) (abs((int) a + (int) b - (int) c - (int) d) / 4 * scale        ), 0, 255);
                const uint8_t hh = clamp((int) (abs((int) a - (int) b - (int) c + (int) d) / 4 * scale * scale), 0, 255);

                out_image->pixel(         x,          y)[Image::R] = ll;  // LL (left-top)
                out_image->pixel(half_w + x,          y)[Image::R] = hl;  // HL (right-top)
                out_image->pixel(         x, half_h + y)[Image::R] = lh;  // LH (left-bottom)
                out_image->pixel(half_w + x, half_h + y)[Image::R] = hh;  // HH (right-bottom)
            }
        }

        cur_w = half_w;
        cur_h = half_h;
        in_image = out_image;
    }

    // fill other color in pixels
    for (int y = 0; y < out_image->getImageHeight(); ++y) {
        for (int x = 0; x < out_image->getImageWidth(); ++x) {
            const uint8_t color = out_image->pixel(x, y)[Image::R];
            out_image->pixel(x, y)[Image::G] = color;
            out_image->pixel(x, y)[Image::B] = color;
        }
    }

    return out_image;
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

    image_windows.emplace_back(std::make_shared<ImageWindow>(out_image, "haar wavelet result"));
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
        image_windows.emplace_back(std::make_shared<ImageWindow>(default_image, "default image.png"));

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
    image_windows.emplace_back(std::make_shared<ImageWindow>(formula_image, "default math formula image"));

    // window loop

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // menu bar

        static bool show_imgui_demo_window = false;

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open image...")) { handle_open_image(); }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) { glfwSetWindowShouldClose(window, GLFW_TRUE); }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Debug")) {
                ImGui::Checkbox("Show ImGUI Demo Window", &show_imgui_demo_window);
                ImGui::EndMenu();
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
            ImGui::SetNextWindowPos(image_window->computeDefaultPosition(), ImGuiCond_Appearing);
            ImGui::SetNextWindowCollapsed(!image_window->is_expanded);
            image_window->is_expanded = ImGui::Begin(
                image_window->getUiTitle(),
                &image_window->is_open,
                ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_HorizontalScrollbar);

            // check should close
            if (!image_window->is_open) {
                image_windows.erase(image_windows.begin() + image_window_index);
                ImGui::End();
                continue;
            }

            if (image_window->is_expanded) {
                // menu bar
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("File")) {
                        if (ImGui::MenuItem("Save as...")) { handle_save_iamge(image_window->getImage()); }
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

                        constexpr float scale_button_step = 1.25f;
                        if (ImGui::Button("-")) {
                            image_window->scale_type = ImageWindow::SCALE_CUSTOM_SCALE;
                            image_window->scale_factor /= scale_button_step;
                        }
                        ImGui::SameLine();
                        ImGui::PushItemWidth(80.f);
                        if (ImGui::DragFloat("##zoom_scale", &image_window->scale_factor,
                                4.f, 0.001f, 10000.f, nullptr, ImGuiSliderFlags_Logarithmic)) {
                            image_window->scale_type = ImageWindow::SCALE_CUSTOM_SCALE;
                        }
                        ImGui::PopItemWidth();
                        ImGui::SameLine();
                        if (ImGui::Button("+")) {
                            image_window->scale_type = ImageWindow::SCALE_CUSTOM_SCALE;
                            image_window->scale_factor *= scale_button_step;
                        }

                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Operations")) {
                        if (ImGui::BeginMenu("Resize")) {
                            static int new_size[2] = {256, 256};
                            ImGui::InputInt2("W x H", new_size);
                            if (ImGui::Button("Apply")) {
                                handle_resize_image(image_window->getImage(), new_size[0], new_size[1]);
                            }
                            ImGui::EndMenu();
                        }
                        if (ImGui::MenuItem("Gray Histogram")) {
                            handle_gray_histogram(image_window->getImage());
                            ImGui::EndMenu();
                        }
                        if (ImGui::BeginMenu("Gaussian Noise")) {
                            static int sigma = 32;
                            constexpr float drag_speed = 0.2f;
                            ImGui::DragScalar("sigma", ImGuiDataType_U8, &sigma, drag_speed);
                            if (ImGui::Button("Apply")) {
                                handle_gaussian_noise(image_window->getImage(), sigma);
                            }
                            ImGui::EndMenu();
                        }
                        if (ImGui::BeginMenu("HAAR Wavelet Transform")) {
                            static int level = 2;
                            static float scale = 1.f;
                            constexpr float drag_speed = 0.4f;
                            ImGui::InputInt("level", &level);
                            ImGui::DragFloat("scale", &scale, drag_speed, 1.f, 256.f, nullptr, ImGuiSliderFlags_Logarithmic);
                            if (ImGui::Button("Apply")) {
                                handle_haar_wavelet_transform(image_window->getImage(), level, scale);
                            }
                            ImGui::EndMenu();
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // draw image
                ImGui::Image((void *)(intptr_t)image_window->getImage()->getTextureId(),
                    image_window->computeImageRenderSize(ImVec2(
                        ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x,
                        ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y)));
            }
            ImGui::End();

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

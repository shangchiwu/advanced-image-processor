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
#include <nfd.h>
#include <stb_image.h>

#include "image.h"
#include "image_window.h"
#include "utility.h"

std::vector<std::shared_ptr<ImageWindow>> image_windows;

static void glfw_error_callback(int error, const char *mesage) {
    fprintf(stderr, "GLFW Error [%d]: %s\n", error, mesage);
}

std::string open_image_path() {
    const char *file_types = "bmp;gif;jpg;png;ppm";
    nfdchar_t *nfd_filepath = nullptr;
    NFD_OpenDialog(file_types, nullptr, &nfd_filepath);

    if (nfd_filepath == nullptr)
        return std::string();

    std::string filename(nfd_filepath);
    free(nfd_filepath);
    return filename;
}

std::string save_image_path(const std::string &file_type) {
    const char *file_types = file_type.c_str();
    nfdchar_t *nfd_filepath = nullptr;
    NFD_SaveDialog(file_types, nullptr, &nfd_filepath);

    if (nfd_filepath == nullptr)
        return std::string();

    std::string filename(nfd_filepath);
    free(nfd_filepath);
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
    image_windows.emplace_back(std::make_shared<ImageWindow>(image));
}

void handle_save_iamge(const std::shared_ptr<Image> image, const std::string &file_type) {
    std::string filepath = save_image_path(file_type);
    if (filepath.empty())
        return;
    std::cout << "Save image [" << file_type << "]: \"" << filepath << "\"" << std::endl;
    image->saveToFile(filepath, file_type);
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
    image_windows.emplace_back(std::make_shared<ImageWindow>(gray_image));

    std::cout << "generate histogram image" << std::endl;
    const std::shared_ptr<Image> histogram_image = generate_histogram_image(histogram);
    image_windows.emplace_back(std::make_shared<ImageWindow>(histogram_image));
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
    image_windows.emplace_back(std::make_shared<ImageWindow>(image_with_noise));

    // draw histogram of noise
    for (int i = 0; i < num_pixels; ++i)
        noise[i] += 0.5f;
    float histogram[256] = {};
    generate_histogram_from_array(noise, num_pixels, histogram);
    std::shared_ptr<Image> noise_histogram_image = generate_histogram_image(histogram);
    image_windows.emplace_back(std::make_shared<ImageWindow>(noise_histogram_image));

    // clean
    delete [] noise;
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
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    const ImVec4 clear_color(0.45f, 0.55f, 0.60f, 1.00f);

    // init image variables

    std::shared_ptr<Image> default_image = std::make_shared<Image>("image.png");
    if (default_image->good())
        image_windows.emplace_back(std::make_shared<ImageWindow>(default_image));

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
    image_windows.emplace_back(std::make_shared<ImageWindow>(formula_image));

    // window loop

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // menu bar

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open image...")) {
                    handle_open_image();
                }

                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) { glfwSetWindowShouldClose(window, GLFW_TRUE); }

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        for (int i = 0; i < image_windows.size(); ++i) {
            std::shared_ptr<ImageWindow> image_window = image_windows[i];

            // image window
            ImGui::SetNextWindowCollapsed(!image_window->is_expanded);
            image_window->is_expanded = ImGui::Begin(
                std::to_string(image_window->getImage()->getTextureId()).c_str(),
                &image_window->is_open, ImGuiWindowFlags_MenuBar);

            // check should close
            if (!image_window->is_open) {
                image_windows.erase(image_windows.begin() + i);
                continue;
            }

            if (image_window->is_expanded) {
                // menu bar
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Save")) {
                        if (ImGui::MenuItem("JPG")) { handle_save_iamge(image_window->getImage(), std::string("jpg")); }
                        if (ImGui::MenuItem("PNG")) { handle_save_iamge(image_window->getImage(), std::string("png")); }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Analyze")) {
                        if (ImGui::MenuItem("Gray Histogram")) { handle_gray_histogram(image_window->getImage()); }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Operation")) {
                        if (ImGui::BeginMenu("Gaussian Noise")) {
                            static int sigma = 32;
                            constexpr float drag_speed = 0.2f;
                            ImGui::DragScalar("sigma", ImGuiDataType_U8, &sigma, drag_speed);
                            if (ImGui::Button("Apply")) {
                                handle_gaussian_noise(image_window->getImage(), sigma);
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

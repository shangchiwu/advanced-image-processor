#include <math.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>
#include <nfd.h>
#include <stb_image.h>

#include "image.h"

std::vector<std::shared_ptr<Image>> images;

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
    if (filepath.empty())
        return;
    std::cout << "Open image: \"" << filepath << "\"" << std::endl;
    images.emplace_back(std::make_shared<Image>(filepath));
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

std::shared_ptr<float[]> generate_gray_histogram(const std::shared_ptr<Image> image) {
    // count each level
    int level_count[256] = {};

    for (int y = 0; y < image->getImageHeight(); ++y) {
        for (int x = 0; x < image->getImageWidth(); ++x) {
            ++level_count[to_gray_average(image->pixel(x, y))];
        }
    }

    // normalize
    std::shared_ptr<float[]> histogram(new float[256]);
    const int max_num = *std::max_element(level_count, level_count + 256);

    for (int i = 0; i < 256; ++i) {
        histogram[i] = (double) level_count[i] / max_num;
    }

    return histogram;
}

std::shared_ptr<Image> generate_histogram_image(const std::shared_ptr<float[]> histogram) {
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
    const std::shared_ptr<float[]> histogram = generate_gray_histogram(image);
    std::cout << "generate histogram image" << std::endl;
    const std::shared_ptr<Image> histogram_image = generate_histogram_image(histogram);
    images.emplace_back(histogram_image);
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

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    const ImVec4 clear_color(0.45f, 0.55f, 0.60f, 1.00f);

    // init image variables

    images.emplace_back(std::make_shared<Image>("image.png"));
    images.emplace_back(std::make_shared<Image>(400, 500));

    for (int y = 0; y < images.back()->getImageHeight(); ++y) {
        for (int x = 0; x < images.back()->getImageWidth(); ++x) {
            // fill some color
            images.back()->pixel(x, y)[Image::R] = x * 2 % 40 + y * 57 % 203 - y * int(exp(x) * 4) % 23;
            images.back()->pixel(x, y)[Image::G] = x * 54 % 87 + int(log(y) * 13) % 147 - y * x * 2 % 76;
            images.back()->pixel(x, y)[Image::B] = int(atan(x) * 73) % 143 + y * 46 % 86 - y * x * 54 % 31;
        }
    }
    images.back()->loadToTexture();

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

        for (std::shared_ptr<Image> image : images) {
            ImGui::Begin(std::to_string(image->getTextureId()).c_str(), nullptr, ImGuiWindowFlags_MenuBar);

            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Save")) {
                    if (ImGui::MenuItem("JPG")) { handle_save_iamge(image, std::string("jpg")); }
                    if (ImGui::MenuItem("PNG")) { handle_save_iamge(image, std::string("png")); }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Analyze")) {
                    if (ImGui::MenuItem("Gray Histogram")) { handle_gray_histogram(image); }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            ImGui::Image((void *)(intptr_t)image->getTextureId(), ImVec2(image->getImageWidth(), image->getImageHeight()));
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

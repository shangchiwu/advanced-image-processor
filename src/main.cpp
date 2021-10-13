#include <iostream>
#include <string>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>
#include <nfd.h>

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

    // window loop

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // main window

        int window_w = 0;
        int window_h = 0;
        glfwGetFramebufferSize(window, &window_w, &window_h);

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(window_w, window_h));

        // menu bar

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open image...")) {
                    std::cout << "Open \"" << open_image_path() << "\"" << std::endl;
                }

                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) { glfwSetWindowShouldClose(window, GLFW_TRUE); }
            }
        }

        // rendering

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

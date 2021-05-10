
#include "header.h"
#include "libvk.h"

#include <GLFW/glfw3.h>
#include <iostream>

int main(int argc, char** argv) {
    try {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(_WINDOW_WIDTH, _WINDOW_HEIGHT, _WINDOW_TITLE, NULL, NULL);

        VulkanAppInitArgs args = {};
        args.appName = _WINDOW_TITLE;
        args.appVersion = 1;

        VulkanApp app;
        if (app.init(args)) {
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
            }
        }

        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

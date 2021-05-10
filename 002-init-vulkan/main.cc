
#include "header.h"
#include "libvk.h"

#include <GLFW/glfw3.h>
#include <iostream>

int main(int argc, char** argv) {
    try {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(_WINDOW_WIDTH, _WINDOW_HEIGHT, _WINDOW_TITLE, NULL, NULL);

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        VulkanAppInitArgs args = {};
        args.appName = _WINDOW_TITLE;
        args.appVersion = 1;
        for (uint32_t i = 0; i < glfwExtensionCount; i++) {
            args.extensions.push_back(glfwExtensions[i]);
        }

        const auto& layers = VulkanApp::enumerateLayers();
        for (auto& l : layers) {
            std::cout << "layer: " << l.layerName;
            if (_DEBUG && strstr(l.layerName, "validation")) {
                args.layers.push_back(l.layerName);
                std::cout << "  USED";
            }
            std::cout << std::endl;
        }

        const auto& extensions = VulkanApp::enumerateExtensions();
        for (auto& e : extensions) {
            std::cout << "extension: " << e.extensionName;
            if (_DEBUG && strstr(e.extensionName, "debug")) {
                args.extensions.push_back(e.extensionName);
                std::cout << "  USED";
            }
            std::cout << std::endl;
        }

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


#include "header.h"
#include "libvk.h"

#include <GLFW/glfw3.h>
#include <iostream>

const std::string hr = "------------------------------------------------------------\n";
const std::string tab(int n) {
    std::string str;
    for (int i = 0; i < n; i++) {
        str += "    ";
    }
    return str;
}

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
        std::cout << std::endl << "layers count: " << layers.size() << std::endl;
        std::cout << hr;
        for (auto& l : layers) {
            std::cout << tab(1) << l.layerName;
            if (_DEBUG && strstr(l.layerName, "validation")) {
                args.layers.push_back(l.layerName);
                std::cout << "  USED";
            }
            std::cout << std::endl;
        }

        const auto& extensions = VulkanApp::enumerateExtensions();
        std::cout << std::endl << "extensions count: " << extensions.size() << std::endl;
        std::cout << hr;
        for (auto& e : extensions) {
            std::cout << tab(1) << e.extensionName;
            if (_DEBUG && strstr(e.extensionName, "debug")) {
                args.extensions.push_back(e.extensionName);
                std::cout << "  USED";
            }
            std::cout << std::endl;
        }

        VulkanApp app;
        app.init(args);

        const auto& devices = app.enumeratePhysicalDevices();
        std::cout << std::endl << "physical devices count: " << devices.size() << std::endl;
        std::cout << hr;
        for (auto& dev : devices) {
            std::cout << tab(1) << dev.props.deviceName << std::endl;
        }

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
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

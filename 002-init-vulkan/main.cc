
#include "header.h"
#include "libvk.h"

#include <GLFW/glfw3.h>
#include <iostream>

const std::string hr = "------------------------------------------------------------\n";
const std::string tab(int n) {
    std::string str;
    for (int i = 0; i < n; i++) {
        str += "  ";
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

        const auto& extensions = VulkanApp::enumerateExtensions();
        std::cout << std::endl << "instance extensions count: " << extensions.size() << std::endl;
        std::cout << hr;
        for (auto& e : extensions) {
            std::cout << tab(1) << e.extensionName;
            if (_DEBUG && strstr(e.extensionName, "debug")) {
                args.extensions.push_back(e.extensionName);
                std::cout << "  USED";
            }
            if (strstr(e.extensionName, "swapchain")) {
                args.extensions.push_back(e.extensionName);
                std::cout << "  USED";
            }
            std::cout << std::endl;
        }

        const auto& layers = VulkanApp::enumerateLayers();
        std::cout << std::endl << "instance layers count: " << layers.size() << std::endl;
        std::cout << hr;
        for (auto& l : layers) {
            std::cout << tab(1) << l.layerName;
            if (_DEBUG && strstr(l.layerName, "validation")) {
                args.layers.push_back(l.layerName);
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
            std::cout << tab(2) << "queue families: " << dev.queueFamilies.size() << std::endl;
            for (size_t i = 0; i < dev.queueFamilies.size(); i++) {
                std::cout << tab(3) << i << ">" << std::endl;
                auto& queueFamily = dev.queueFamilies.at(i);
                std::cout << tab(4) << "queue count: " << queueFamily.queueCount << std::endl;
                auto flags = queueFamily.queueFlags;
                std::cout << tab(4) << "queue flags:";
                if (flags & VK_QUEUE_GRAPHICS_BIT)std::cout << " GRAPHICS";
                if (flags & VK_QUEUE_COMPUTE_BIT) std::cout << " COMPUTE";
                if (flags & VK_QUEUE_TRANSFER_BIT) std::cout << " TRANSFER";
                if (flags & VK_QUEUE_SPARSE_BINDING_BIT) std::cout << " SPARSE";
                if (flags & VK_QUEUE_PROTECTED_BIT)std::cout << " PROTECTED";
                std::cout << std::endl;
            }
        }

        auto& physicalDevice = devices.at(0);

        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(app.instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }

        uint32_t graphicsQueueFamilyIndex = -1;
        uint32_t presentQueueFamilyIndex = -1;
        for (auto& qf : physicalDevice.queueFamilies) {
            for (size_t i = 0; i < physicalDevice.queueFamilies.size(); i++) {
                const auto& qf = physicalDevice.queueFamilies.at(i);
                if (graphicsQueueFamilyIndex == -1 && qf.queueFlags & VK_QUEUE_GRAPHICS_BIT != 0)
                    graphicsQueueFamilyIndex = i;
                if (presentQueueFamilyIndex == -1 && physicalDevice.checkSurfaceSupport(surface, i))
                    presentQueueFamilyIndex = i;
                if (graphicsQueueFamilyIndex != -1 && presentQueueFamilyIndex != -1) {
                    break;
                }
            }
        }
        std::cout << hr << std::endl;
        std::cout << "Selected Graphics Queue Family: " << graphicsQueueFamilyIndex << std::endl;
        std::cout << "Selected Present Queue Family: " << presentQueueFamilyIndex << std::endl;

        auto deviceExtensions = physicalDevice.enumerateExtensions();
        auto deviceLayers = physicalDevice.enumerateLayers();

        VulkanLogicalDeviceInitArgs logicalDeviceInitArgs;
        logicalDeviceInitArgs.queueFamilyIndex = graphicsQueueFamilyIndex;
        std::cout << std::endl << "device extensions count: " << deviceExtensions.size() << std::endl;
        std::cout << hr;
        for (auto& e : deviceExtensions)
        {
            std::cout << tab(1) << e.extensionName;
            if (_DEBUG && strstr(e.extensionName, "debug")) {
                logicalDeviceInitArgs.extensions.push_back(e.extensionName);
                std::cout << "  USED";
            }
            if (strstr(e.extensionName, "swapchain")) {
                logicalDeviceInitArgs.extensions.push_back(e.extensionName);
                std::cout << "  USED";
            }
            std::cout << std::endl;
        }
        
        std::cout << std::endl << "device layers count: " << deviceLayers.size() << std::endl;
        std::cout << hr;
        for (auto& l : deviceLayers)
        {
            std::cout << tab(1) << l.layerName;
            if (_DEBUG && strstr(l.layerName, "validation")) {
                logicalDeviceInitArgs.layers.push_back(l.layerName);
                std::cout << "  USED";
            }
            std::cout << std::endl;
        }
        std::cout<< std::endl;

        auto logicalDevice = physicalDevice.createLogicalDevice(logicalDeviceInitArgs);
        std::cout << "LogicalDevice created: " << (size_t)logicalDevice.device << std::endl;

        const auto& swapchainSupport = physicalDevice.checkSwapchainSupport(surface);
        const VkSurfaceFormatKHR* format = &swapchainSupport.formats[0];
        for (auto& f : swapchainSupport.formats) {
            if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                format = &f;
                break;
            }
        }
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        for (auto& p : swapchainSupport.presentModes) {
            if (p == VK_PRESENT_MODE_MAILBOX_KHR) {
                presentMode = p;
                break;
            }
        }
        VkExtent2D extent = swapchainSupport.capabilities.currentExtent;
        extent.width = std::clamp(extent.width,
            swapchainSupport.capabilities.minImageExtent.width,
            swapchainSupport.capabilities.maxImageExtent.width
        );
        extent.height = std::clamp(extent.height,
            swapchainSupport.capabilities.minImageExtent.height,
            swapchainSupport.capabilities.maxImageExtent.height
        );

        VulkanSwapchainArgs swapchainArgs;
        swapchainArgs.surface = surface;
        swapchainArgs.minImageCount = swapchainSupport.capabilities.minImageCount + 1;
        swapchainArgs.extent = extent;
        swapchainArgs.format = *format;
        swapchainArgs.presentMode = presentMode;
        swapchainArgs.queueFamilyIndices = {
            graphicsQueueFamilyIndex,
            presentQueueFamilyIndex
        };
        swapchainArgs.preTransform = swapchainSupport.capabilities.currentTransform;
        VulkanSwapchain swapchain = logicalDevice.createSwapchain(swapchainArgs);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }

        physicalDevice.destroyLogicalDevice(logicalDevice);

        vkDestroySurfaceKHR(app.instance, surface, nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

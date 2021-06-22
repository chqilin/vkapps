
#include "header.h"
#include "libvk.h"
#include "utils.h"

#include <GLFW/glfw3.h>

class VulkanApp {
    GLFWwindow* window = nullptr;
    VulkanInstance instance;
    VulkanPhysicalDevice physicalDevice;
    VkSurfaceKHR surface = nullptr;
    VulkanLogicalDevice logicalDevice;
    VulkanSwapchain swapchain;
    VulkanGraphicsPipeline pipeline;
    VulkanFrameBufferObject frameBuffers;
    std::vector<VkCommandBuffer> commandBuffers;
    VkSemaphore onImageAvailable = nullptr;
    VkSemaphore onRenderFinished = nullptr;

public:
    void init()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        this->window = glfwCreateWindow(_WINDOW_WIDTH, _WINDOW_HEIGHT, _WINDOW_TITLE, NULL, NULL);
        
        glfwSetWindowUserPointer(window, this);
        glfwSetWindowSizeCallback(window, VulkanApp::onWindowResize);

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        VulkanInstanceArgs args = {};
        args.appName = _WINDOW_TITLE;
        args.appVersion = 1;
        for (uint32_t i = 0; i < glfwExtensionCount; i++) {
            args.extensions.push_back(glfwExtensions[i]);
        }

        const auto& extensions = VulkanInstance::enumerateExtensions();
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

        const auto& layers = VulkanInstance::enumerateLayers();
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

        this->instance = VulkanInstance::createInstance(args);

        const auto& devices = instance.enumeratePhysicalDevices();
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
        if (devices.empty())
        {
            throw std::runtime_error("Physical Devices not found.");
        }
        this->physicalDevice = devices.at(0);

        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(instance.handle, window, nullptr, &surface) != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to create window surface!");
        }
        this->surface = surface;

        uint32_t graphicsQueueFamilyIndex = -1;
        uint32_t presentQueueFamilyIndex = -1;
        for (size_t i = 0; i < physicalDevice.queueFamilies.size(); i++) {
            const auto& qf = physicalDevice.queueFamilies.at(i);
            if (graphicsQueueFamilyIndex == -1 && (qf.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
                graphicsQueueFamilyIndex = i;
            if (presentQueueFamilyIndex == -1 && physicalDevice.checkSurfaceSupport(surface, i))
                presentQueueFamilyIndex = i;
            if (graphicsQueueFamilyIndex != -1 && presentQueueFamilyIndex != -1) {
                break;
            }
        }

        std::cout << hr << std::endl;
        std::cout << "Selected Graphics Queue Family: " << graphicsQueueFamilyIndex << std::endl;
        std::cout << "Selected Present Queue Family: " << presentQueueFamilyIndex << std::endl;

        auto deviceExtensions = physicalDevice.enumerateExtensions();
        auto deviceLayers = physicalDevice.enumerateLayers();

        VulkanLogicalDeviceArgs logicalDeviceInitArgs;
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
        std::cout << std::endl;

        this->logicalDevice = physicalDevice.createLogicalDevice(logicalDeviceInitArgs);
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
        this->swapchain = logicalDevice.createSwapchain(swapchainArgs);

        VulkanGraphicsPipelineArgs pipelineArgs = {};
        pipelineArgs.vert = readFile("./shader.vert.spv");
        pipelineArgs.frag = readFile("./shader.frag.spv");
        pipelineArgs.viewport = {
            0, 0,
            (float)extent.width, (float)extent.height,
            0.0f, 1.0f
        };
        pipelineArgs.scissor = {
            {0, 0},
            extent
        };
        pipelineArgs.colorFormat = swapchain.format;
        auto pipeline = logicalDevice.createGraphicsPipeline(pipelineArgs);
        std::cout << "GraphicsPipeline created: " << (size_t)pipeline.handle << std::endl;

        this->frameBuffers = logicalDevice.createFrameBufferObject({
            pipeline.renderPass,
            swapchain.imageViews,
            extent.width,
            extent.height
            });

        this->commandBuffers = logicalDevice.beginCommandBuffers(pipeline, frameBuffers);
        logicalDevice.endCommandBuffers(commandBuffers);

        this->onImageAvailable = logicalDevice.createSemaphore();
        this->onRenderFinished = logicalDevice.createSemaphore();
    }

    void quit()
    {
        logicalDevice.destroySemaphore(onRenderFinished);
        logicalDevice.destroySemaphore(onImageAvailable);
        logicalDevice.destroyFrameBufferObject(frameBuffers);
        logicalDevice.destroyGraphicsPipeline(pipeline);
        logicalDevice.destroySwapchain(swapchain);
        physicalDevice.destroyLogicalDevice(logicalDevice);
        if (surface != nullptr)
        {
            vkDestroySurfaceKHR(instance.handle, surface, nullptr);
            surface = nullptr;
        }
        VulkanInstance::destroyInstance(instance);
        if (window != nullptr)
        {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
    }

    void exec()
    {
        VulkanPresentArgs presentArgs;
        presentArgs.swapchain = swapchain.handle;
        presentArgs.commandBuffers = commandBuffers;
        presentArgs.onImageAvailable = onImageAvailable;
        presentArgs.onRenderFinished = onRenderFinished;

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            logicalDevice.present(presentArgs);
        }

        vkDeviceWaitIdle(logicalDevice.device);
    }

    static void onWindowResize(GLFWwindow* window, int width, int height) {
        void* userPointer = glfwGetWindowUserPointer(window);
        if (userPointer == nullptr || width <= 0 || height <= 0)
            return;
        auto& app = *(VulkanApp*)userPointer;
    }
};

int main(int argc, char** argv) {
    VulkanApp app;
    try {
        app.init();
        app.exec();
        app.quit();
        return 0;
    }
    catch (const std::exception& e) {
        app.quit();
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

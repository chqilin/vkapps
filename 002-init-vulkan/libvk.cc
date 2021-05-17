
#include "libvk.h"
#include <iostream>

static VkBool32 VKAPI_PTR globalDebugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t object,
    size_t location,
    int32_t messageCode,
    const char* pLayerPrefix,
    const char* pMessage,
    void* pUserData)
{
    std::cerr << "validation layer: " << pMessage << std::endl;
    return VK_FALSE;
}

VulkanSwapchain VulkanLogicalDevice::createSwapchain(const VulkanSwapchainArgs& args)
{
    VkSwapchainCreateInfoKHR sci = {};
    sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sci.pNext = nullptr;
    sci.surface = args.surface;
    sci.minImageCount = args.minImageCount;
    sci.imageFormat = args.format.format;
    sci.imageColorSpace = args.format.colorSpace;
    sci.imageExtent = args.extent;
    sci.imageArrayLayers = 1;
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t indices[] = {
        args.queueFamilyIndices.graphicsQueueFamilyIndex,
        args.queueFamilyIndices.presentQueueFamilyIndex
    };
    if (indices[0] != indices[1])
    {
        sci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        sci.queueFamilyIndexCount = 2;
        sci.pQueueFamilyIndices = indices;
    }
    else
    {
        sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        sci.queueFamilyIndexCount = 0;
        sci.pQueueFamilyIndices = nullptr;
    }

    sci.preTransform = args.preTransform;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    sci.presentMode = args.presentMode;
    sci.clipped = VK_TRUE;

    sci.oldSwapchain = VK_NULL_HANDLE;

    VulkanSwapchain swapchain = {};
    if (VK_SUCCESS != vkCreateSwapchainKHR(device, &sci, nullptr, &swapchain.handle))
    {
        throw std::runtime_error("create swapchain failed.");
    }

    swapchain.format = args.format.format;
    swapchain.extent = args.extent;

    uint32_t swapchainImageCount = 0;
    vkGetSwapchainImagesKHR(device, swapchain.handle, &swapchainImageCount, nullptr);
    swapchain.images.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(device, swapchain.handle, &swapchainImageCount, swapchain.images.data());

    return swapchain;
}

void VulkanLogicalDevice::destroySwapchain(const VulkanSwapchain& swapchain)
{
    if (swapchain.handle != nullptr)
    {
        vkDestroySwapchainKHR(device, swapchain.handle, nullptr);
    }

}

std::vector<VkExtensionProperties> VulkanPhysicalDevice::enumerateExtensions() const
{
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> list(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, list.data());
    return list;
}

std::vector<VkLayerProperties> VulkanPhysicalDevice::enumerateLayers() const
{
    uint32_t count = 0;
    vkEnumerateDeviceLayerProperties(device, &count, nullptr);
    std::vector<VkLayerProperties> list(count);
    vkEnumerateDeviceLayerProperties(device, &count, list.data());
    return list;
}

VulkanLogicalDevice VulkanPhysicalDevice::createLogicalDevice(const VulkanLogicalDeviceInitArgs& args) const
{
    float queuePriority = 1.0f;

    VkPhysicalDeviceFeatures features = {};

    VkDeviceQueueCreateInfo qci = {};
    qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci.pNext = nullptr;
    qci.queueFamilyIndex = args.queueFamilyIndex;
    qci.queueCount = 1;
    qci.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo dci = {};
    dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.pNext = nullptr;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &qci;
    dci.pEnabledFeatures = &features;

    dci.enabledExtensionCount = args.extensions.size();
    dci.ppEnabledExtensionNames = args.extensions.data();

    dci.enabledLayerCount = args.layers.size();
    dci.ppEnabledLayerNames = args.layers.data();

    VkDevice logicalDevice = nullptr;
    if (VK_SUCCESS != vkCreateDevice(device, &dci, nullptr, &logicalDevice))
    {
        throw std::runtime_error("create logical device failed.");
    }

    VkQueue queue = nullptr;
    vkGetDeviceQueue(logicalDevice, args.queueFamilyIndex, 0, &queue);

    VulkanLogicalDevice ret = {};
    ret.device = logicalDevice;
    ret.queue = queue;

    return ret;
}

VulkanSwapchainSupport VulkanPhysicalDevice::checkSwapchainSupport(VkSurfaceKHR surface) const
{
    VulkanSwapchainSupport support;
    support.surface = surface;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &support.capabiliteis);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    support.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, support.formats.data());

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    support.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, support.presentModes.data());

    return support;
}

std::vector<VkExtensionProperties> VulkanApp::enumerateExtensions()
{
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> list(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, list.data());
    return list;
}

std::vector<VkLayerProperties> VulkanApp::enumerateLayers()
{
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> list(count);
    vkEnumerateInstanceLayerProperties(&count, list.data());
    return list;
}

VulkanApp::VulkanApp()
    : instance(nullptr)
    , extensionFactory()
    , debugCallback(nullptr)
{}

VulkanApp::~VulkanApp()
{
    this->quit();
}

void VulkanApp::init(const VulkanAppInitArgs& args)
{
    VkResult ret = VK_SUCCESS;

    VkApplicationInfo app = {};
    app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pNext = nullptr;
    app.pApplicationName = args.appName;
    app.applicationVersion = args.appVersion;
    app.pEngineName = "libvk";
    app.engineVersion = 1;
    app.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo create{};
    create.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create.pApplicationInfo = &app;
    create.enabledExtensionCount = (uint32_t)args.extensions.size();
    create.ppEnabledExtensionNames = args.extensions.data();
    create.enabledLayerCount = (uint32_t)args.layers.size();
    create.ppEnabledLayerNames = args.layers.data();

    ret = vkCreateInstance(&create, nullptr, &instance);
    if (ret != VK_SUCCESS)
    {
        throw std::runtime_error("init vulkan instance failed.");
    }

    extensionFactory.init(instance);

    bool enabledDebug = false;
    for (auto& layer : args.layers)
    {
        if (strstr(layer, "validation"))
        {
            enabledDebug = true;
            break;
        }
    }
    if (enabledDebug)
    {
        VkDebugReportCallbackCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.pfnCallback = globalDebugCallback;
        createInfo.pUserData = nullptr;

        auto func = extensionFactory.vkCreateDebugReportCallbackEXT;
        ret = func(instance, &createInfo, nullptr, &debugCallback);
        if (ret != VK_SUCCESS)
        {
            throw std::runtime_error("create vulkan debug callback failed.");
        }
    }
}

void VulkanApp::quit()
{
    if (debugCallback != nullptr)
    {
        extensionFactory.vkDestroyDebugReportCallbackEXT(instance, debugCallback, nullptr);
        debugCallback = nullptr;
    }
    if (instance != nullptr)
    {
        vkDestroyInstance(instance, nullptr);
        instance = nullptr;
    }
}

std::vector<VulkanPhysicalDevice> VulkanApp::enumeratePhysicalDevices()
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, devices.data());

    std::vector<VulkanPhysicalDevice> list(count);
    for (size_t i = 0; i < count; i++)
    {
        auto& p = list.at(i);
        p.device = devices.at(i);
        vkGetPhysicalDeviceProperties(p.device, &p.props);
        vkGetPhysicalDeviceFeatures(p.device, &p.features);

        uint32_t qfCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(p.device, &qfCount, nullptr);
        p.queueFamilies.resize(qfCount);
        vkGetPhysicalDeviceQueueFamilyProperties(p.device, &qfCount, p.queueFamilies.data());
    }

    return list;
}

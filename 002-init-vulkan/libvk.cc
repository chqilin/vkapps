
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

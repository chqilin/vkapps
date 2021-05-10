
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

std::vector<VkExtensionProperties> enumerateExtensions()
{
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> list(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, list.data());
    return list;
}

std::vector<VkLayerProperties> enumerateLayers()
{
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> list(count);
    vkEnumerateInstanceLayerProperties(&count, list.data());
    return list;
}

std::vector<VulkanPhysicalDevice> VulkanApp::enumeratePhysicalDevices()
{
    std::vector<VulkanPhysicalDevice> list;
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

bool VulkanApp::init(const VulkanAppInitArgs& args)
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
        return false;
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
            return false;
        }
    }

    return true;
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

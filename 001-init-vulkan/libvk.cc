
#include "libvk.h"
#include <iostream>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
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
    , debugMessenger(nullptr)
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
        return false;

    if(!extensionFactory.init(instance))
        return false;

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
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.pNext = nullptr;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;

        auto func = extensionFactory.vkCreateDebugUtilsMessengerEXT;
        ret = func(instance, &createInfo, nullptr, &debugMessager);
        if (ret != VK_SUCCESS)
            return false;
    }

    return true;
}

void VulkanApp::quit()
{
    if(debugMessenger != nullptr)
    {
        extensionFactory.vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        debugMessenger = nullptr;
    }
    if (instance != nullptr)
    {
        vkDestroyInstance(instance, nullptr);
        instance = nullptr;
    }
}

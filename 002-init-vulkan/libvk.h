#ifndef _LIBVK_H_
#define _LIBVK_H_

#include <vulkan/vulkan.hpp>
#include <vector>
#include <map>
#include <string>

template<typename Func>
Func resolveVulkanEXT(VkInstance instance, const char* name, Func& ptr)
{
    ptr = (Func)vkGetInstanceProcAddr(instance, name);
    return ptr;
}

#define _DEF_EXT_FUNC(extfunc) \
    if(nullptr == resolveVulkanEXT(instance, (#extfunc), (extfunc))) \
        throw std::runtime_error("vulkan extension not found: "#extfunc)

struct VulkanExtensionFactory
{
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;

    void init(VkInstance instance)
    {
        _DEF_EXT_FUNC(vkCreateDebugReportCallbackEXT);
        _DEF_EXT_FUNC(vkDestroyDebugReportCallbackEXT);
    }
};

class VulkanPhysicalDevice
{
public:

};

struct VulkanAppInitArgs
{
    const char* appName;
    uint32_t appVersion;
    std::vector<const char*> extensions;
    std::vector<const char*> layers;
};

class VulkanApp
{
public:
    static std::vector<VkExtensionProperties> enumerateExtensions();
    static std::vector<VkLayerProperties> enumerateLayers();
    static std::vector<VulkanPhysicalDevice> enumeratePhysicalDevices();

public:
    VulkanApp();
    virtual ~VulkanApp();
    bool init(const VulkanAppInitArgs& args);
    void quit();

private:
    VkInstance instance;
    VulkanExtensionFactory extensionFactory;
    VkDebugReportCallbackEXT debugCallback;
};

#endif//_LIBVK_H_
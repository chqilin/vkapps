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

struct VulkanSwapchainSupport
{
    VkSurfaceKHR surface;
    bool support;
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices
{
    uint32_t graphicsQueueFamilyIndex;
    uint32_t presentQueueFamilyIndex;
};

struct VulkanSwapchainArgs
{
    VkSurfaceKHR surface = nullptr;
    VkExtent2D extent;
    int minImageCount;
    VkSurfaceFormatKHR format;
    VkPresentModeKHR presentMode;
    QueueFamilyIndices queueFamilyIndices;
    VkSurfaceTransformFlagBitsKHR preTransform;
};

struct VulkanSwapchain
{
    VkSwapchainKHR handle = nullptr;
    VkFormat format;
    VkExtent2D extent;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
};

struct VulkanGraphicsPipelineArgs
{
    std::vector<char> vert;
    std::vector<char> frag;
    VkViewport viewport;
    VkRect2D scissor;
    VkFormat colorFormat;
};

struct VulkanGraphicsPipeline
{
    VkPipeline handle = nullptr;
    VkShaderModule vert = nullptr;
    VkShaderModule frag = nullptr;
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineLayout layout;
    VkRenderPass renderPass;
};

struct VulkanFrameBufferArgs
{
    VkRenderPass renderPass;
    std::vector<VkImageView> imageViews;
    uint32_t width;
    uint32_t height;
};

struct VulkanFrameBufferObject
{
    std::vector<VkFramebuffer> handles;
};

struct VulkanLogicalDeviceArgs
{
    uint32_t queueFamilyIndex;
    std::vector<const char*> extensions;
    std::vector<const char*> layers;
};

struct VulkanPresentArgs
{
    VkSwapchainKHR swapchain;
    std::vector<VkCommandBuffer> commandBuffers;
    VkSemaphore onImageAvailable;
    VkSemaphore onRenderFinished;
};

struct VulkanLogicalDevice
{
    VkDevice device = nullptr;
    VkQueue queue = nullptr;
    VkCommandPool commandPool = nullptr;

    VulkanSwapchain createSwapchain(const VulkanSwapchainArgs& args) const;
    void destroySwapchain(VulkanSwapchain& swapchain) const;

    VkShaderModule createShaderModule(const std::vector<char>& code) const;
    void destroyShaderModule(VkShaderModule shader) const;

    VulkanGraphicsPipeline createGraphicsPipeline(const VulkanGraphicsPipelineArgs& args) const;
    void destroyGraphicsPipeline(VulkanGraphicsPipeline& pipeline) const;

    VulkanFrameBufferObject createFrameBufferObject(const VulkanFrameBufferArgs& args) const;
    void destroyFrameBufferObject(VulkanFrameBufferObject& fbo) const;

    VkSemaphore createSemaphore() const;
    void destroySemaphore(VkSemaphore& semaphore) const;

    std::vector<VkCommandBuffer> beginCommandBuffers(VulkanGraphicsPipeline& pipeline, VulkanFrameBufferObject& fbo) const;
    void endCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers) const;
    void present(const VulkanPresentArgs& args) const;
};

struct VulkanPhysicalDevice
{
    VkPhysicalDevice device = nullptr;
    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceLimits limits;
    VkPhysicalDeviceMemoryProperties memoryProps;
    std::vector<VkQueueFamilyProperties> queueFamilies;

    std::vector<VkExtensionProperties> enumerateExtensions() const;
    std::vector<VkLayerProperties> enumerateLayers() const;
    VulkanLogicalDevice createLogicalDevice(const VulkanLogicalDeviceArgs& args) const;
    void destroyLogicalDevice(VulkanLogicalDevice& logicalDevice) const;
    bool checkSurfaceSupport(VkSurfaceKHR surface, uint32_t queueFamilyIndex) const;
    VulkanSwapchainSupport checkSwapchainSupport(VkSurfaceKHR surface) const;
};

struct VulkanInstanceArgs
{
    const char* appName = nullptr;
    uint32_t appVersion = 0;
    std::vector<const char*> extensions;
    std::vector<const char*> layers;
};

struct VulkanInstance
{
    static std::vector<VkExtensionProperties> enumerateExtensions();
    static std::vector<VkLayerProperties> enumerateLayers();

    static VulkanInstance createInstance(const VulkanInstanceArgs& args);
    static void destroyInstance(VulkanInstance& instance);

    VkInstance handle = nullptr;
    VulkanExtensionFactory extensionFactory;
    VkDebugReportCallbackEXT debugCallback = nullptr;

    std::vector<VulkanPhysicalDevice> enumeratePhysicalDevices();
};

#endif//_LIBVK_H_

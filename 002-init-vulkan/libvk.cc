
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

VulkanSwapchain VulkanLogicalDevice::createSwapchain(const VulkanSwapchainArgs& args) const
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

    swapchain.imageViews.resize(swapchainImageCount);
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        VkImageViewCreateInfo ivci = {};
        ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivci.pNext = nullptr;
        ivci.image = swapchain.images.at(i);
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.format = swapchain.format;

        ivci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        ivci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        ivci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        ivci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ivci.subresourceRange.baseMipLevel = 0;
        ivci.subresourceRange.levelCount = 1;
        ivci.subresourceRange.baseArrayLayer = 0;
        ivci.subresourceRange.layerCount = 1;

        if (VK_SUCCESS != vkCreateImageView(device, &ivci, nullptr, &swapchain.imageViews[i]))
        {
            throw std::runtime_error("create image-view for swapchain failed.");
        }
    }

    return swapchain;
}

void VulkanLogicalDevice::destroySwapchain(VulkanSwapchain& swapchain) const
{
    for (auto& imageView : swapchain.imageViews)
    {
        if (imageView != nullptr)
        {
            vkDestroyImageView(device, imageView, nullptr);
            imageView = nullptr;
        }
    }
    if (swapchain.handle != nullptr)
    {
        vkDestroySwapchainKHR(device, swapchain.handle, nullptr);
        swapchain.handle = nullptr;
    }

}

VkShaderModule VulkanLogicalDevice::createShaderModule(const std::vector<char>& code) const
{
    VkShaderModuleCreateInfo smci = {};
    smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    smci.pNext = nullptr;
    smci.codeSize = code.size();
    smci.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule shader;
    if (VK_SUCCESS != vkCreateShaderModule(device, &smci, nullptr, &shader))
    {
        throw std::runtime_error("create shader module failed.");
    }

    return shader;
}

void VulkanLogicalDevice::destroyShaderModule(VkShaderModule shader) const
{
    if (shader != nullptr)
    {
        vkDestroyShaderModule(device, shader, nullptr);
    }
}

VulkanGraphicsPipeline VulkanLogicalDevice::createGraphicsPipeline(const VulkanGraphicsPipelineArgs& args) const
{
    VkShaderModule vert = this->createShaderModule(args.vert);
    VkShaderModule frag = this->createShaderModule(args.frag);

    VkPipelineShaderStageCreateInfo vsci = {};
    vsci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vsci.pNext = nullptr;
    vsci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vsci.module = vert;
    vsci.pName = "main";

    VkPipelineShaderStageCreateInfo fsci = {};
    fsci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fsci.pNext = nullptr;
    fsci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fsci.module = frag;
    fsci.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        vsci, fsci
    };

    VkPipelineVertexInputStateCreateInfo visci = {};
    visci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    visci.pNext = nullptr;
    visci.vertexBindingDescriptionCount = 0;
    visci.pVertexBindingDescriptions = nullptr;
    visci.vertexAttributeDescriptionCount = 0;
    visci.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo iasci = {};
    iasci.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    iasci.pNext = nullptr;
    iasci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    iasci.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo vpsci = {};
    vpsci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vpsci.pNext = nullptr;
    vpsci.viewportCount = 1;
    vpsci.pViewports = &args.viewport;
    vpsci.scissorCount = 1;
    vpsci.pScissors = &args.scissor;

    VkPipelineRasterizationStateCreateInfo rssci = {};
    rssci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rssci.pNext = nullptr;
    rssci.depthClampEnable = VK_FALSE;
    rssci.rasterizerDiscardEnable = VK_FALSE;
    rssci.polygonMode = VK_POLYGON_MODE_FILL;
    rssci.lineWidth = 1.0f;
    rssci.cullMode = VK_CULL_MODE_BACK_BIT;
    rssci.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rssci.depthBiasEnable = VK_FALSE;
    rssci.depthBiasConstantFactor = 0.0f;
    rssci.depthBiasClamp = 0.0f;
    rssci.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo mssci = {};
    mssci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    mssci.pNext = nullptr;
    mssci.sampleShadingEnable = VK_FALSE;
    mssci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    mssci.minSampleShading = 1.0f;
    mssci.pSampleMask = nullptr;
    mssci.alphaToCoverageEnable = VK_FALSE;
    mssci.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState cbas = {};
    cbas.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    cbas.blendEnable = VK_FALSE;
    cbas.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    cbas.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    cbas.colorBlendOp = VK_BLEND_OP_ADD;
    cbas.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    cbas.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    cbas.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo cbsci = {};
    cbsci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cbsci.pNext = nullptr;
    cbsci.logicOpEnable = VK_FALSE;
    cbsci.logicOp = VK_LOGIC_OP_COPY;
    cbsci.attachmentCount = 1;
    cbsci.pAttachments = &cbas;
    cbsci.blendConstants[0] = 0.0f; // Optional
    cbsci.blendConstants[1] = 0.0f; // Optional
    cbsci.blendConstants[2] = 0.0f; // Optional
    cbsci.blendConstants[3] = 0.0f; // Optional

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };
    VkPipelineDynamicStateCreateInfo dsci = {};
    dsci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dsci.pNext = nullptr;
    dsci.dynamicStateCount = 2;
    dsci.pDynamicStates = dynamicStates;

    VulkanGraphicsPipeline pipeline = {};
    pipeline.vert = vert;
    pipeline.frag = frag;

    VkPipelineLayoutCreateInfo lci = {};
    lci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    lci.pNext = nullptr;
    lci.setLayoutCount = 0;
    lci.pSetLayouts = nullptr;
    lci.pushConstantRangeCount = 0;
    lci.pPushConstantRanges = 0;
    if (VK_SUCCESS != vkCreatePipelineLayout(device, &lci, nullptr, &pipeline.layout))
    {
        this->destroyGraphicsPipeline(pipeline);
        throw std::runtime_error("Create pipeline layout failed.");
    }

    VkAttachmentDescription attachment = {};
    attachment.format = args.colorFormat;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo rpci = {};
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.pNext = nullptr;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attachment;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    if (VK_SUCCESS != vkCreateRenderPass(device, &rpci, nullptr, &pipeline.renderPass))
    {
        this->destroyGraphicsPipeline(pipeline);
        throw std::runtime_error("Create render-pass failed.");
    }

    VkGraphicsPipelineCreateInfo gpci = {};
    gpci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    gpci.pNext = nullptr;
    gpci.stageCount = 2;
    gpci.pStages = shaderStages;
    gpci.pVertexInputState = &visci;
    gpci.pInputAssemblyState = &iasci;
    gpci.pViewportState = &vpsci;
    gpci.pRasterizationState = &rssci;
    gpci.pMultisampleState = &mssci;
    gpci.pDepthStencilState = nullptr;
    gpci.pColorBlendState = &cbsci;
    gpci.pDynamicState = &dsci;
    gpci.layout = pipeline.layout;
    gpci.renderPass = pipeline.renderPass;
    gpci.subpass = 0;
    gpci.basePipelineHandle = VK_NULL_HANDLE;
    gpci.basePipelineIndex = -1;
    if (VK_SUCCESS != vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &gpci, nullptr, &pipeline.handle))
    {
        this->destroyGraphicsPipeline(pipeline);
        throw std::runtime_error("Create graphics pipeline failed.");
    }

    return pipeline;
}

VulkanFrameBufferObject VulkanLogicalDevice::createFrameBufferObject(const VulkanFrameBufferArgs& args) const
{
    VulkanFrameBufferObject fbo;
    fbo.handles.resize(args.imageViews.size());

    for (size_t i = 0; i < args.imageViews.size(); i++)
    {
        auto& imageView = args.imageViews.at(i);
        VkImageView attachments[] = { imageView };

        VkFramebufferCreateInfo fbci = {};
        fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbci.pNext = nullptr;
        fbci.renderPass = args.renderPass;
        fbci.attachmentCount = 1;
        fbci.pAttachments = attachments;
        fbci.width = args.width;
        fbci.height = args.height;
        fbci.layers = 1;

        VkFramebuffer handle;
        if(VK_SUCCESS != vkCreateFramebuffer(device, &fbci, nullptr, &handle))
        {
            this->destroyFrameBufferObject(fbo);
            throw std::runtime_error("Create framebuffer failed.");
        }

        fbo.handles[i] = handle;
    }

    return fbo;
}

void VulkanLogicalDevice::destroyFrameBufferObject(VulkanFrameBufferObject& fbo) const
{
    for(size_t i = 0; i < fbo.handles.size(); i++)
    {
        VkFramebuffer handle = fbo.handles.at(i);
        if(handle == VK_NULL_HANDLE)
            continue;
        vkDestroyFramebuffer(device, handle, nullptr);
        fbo.handles[i] = VK_NULL_HANDLE;
    }
}

void VulkanLogicalDevice::destroyGraphicsPipeline(VulkanGraphicsPipeline& pipeline) const
{
    if (pipeline.layout != nullptr)
    {
        vkDestroyPipelineLayout(device, pipeline.layout, nullptr);
        pipeline.layout = nullptr;
    }
    if (pipeline.vert != nullptr)
    {
        this->destroyShaderModule(pipeline.vert);
        pipeline.vert = nullptr;
    }
    if (pipeline.frag != nullptr)
    {
        this->destroyShaderModule(pipeline.frag);
        pipeline.frag = nullptr;
    }
    if (pipeline.handle != nullptr)
    {
        vkDestroyPipeline(device, pipeline.handle, nullptr);
        pipeline.handle = nullptr;
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

void VulkanPhysicalDevice::destroyLogicalDevice(VulkanLogicalDevice& logicalDevice) const
{
    if (logicalDevice.device != nullptr)
    {
        vkDestroyDevice(logicalDevice.device, nullptr);
        logicalDevice.device = nullptr;
    }
}


bool VulkanPhysicalDevice::checkSurfaceSupport(VkSurfaceKHR surface, uint32_t queueFamilyIndex) const
{
    VkBool32 support = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, queueFamilyIndex, surface, &support);
    return support == VK_TRUE;
}

VulkanSwapchainSupport VulkanPhysicalDevice::checkSwapchainSupport(VkSurfaceKHR surface) const
{
    VulkanSwapchainSupport support;
    support.surface = surface;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &support.capabilities);

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

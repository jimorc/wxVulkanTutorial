#include "VulkanCanvas.h"
#include "VulkanException.h"
#include "wxVulkanTutorialApp.h"
#include <vulkan/vulkan.h>
#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#pragma comment(lib, "vulkan-1.lib")

const std::vector<const char*> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef _DEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

VulkanCanvas::VulkanCanvas(wxWindow *pParent,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name)
    : wxWindow(pParent, id, pos, size, style, name),
    m_vulkanInitialized(false), m_instance(VK_NULL_HANDLE),
    m_surface(VK_NULL_HANDLE), m_physicalDevice(VK_NULL_HANDLE),
    m_logicalDevice(VK_NULL_HANDLE), m_swapchain(VK_NULL_HANDLE),
    m_renderPass(VK_NULL_HANDLE), m_pipelineLayout(VK_NULL_HANDLE),
    m_descriptorSetLayout(VK_NULL_HANDLE),
    m_graphicsPipeline(VK_NULL_HANDLE), m_commandPool(VK_NULL_HANDLE),
    m_textureImage(VK_NULL_HANDLE), m_textureImageMemory(VK_NULL_HANDLE),
    m_textureImageView(VK_NULL_HANDLE), m_textureSampler(VK_NULL_HANDLE),
    m_depthImage(VK_NULL_HANDLE), m_depthImageMemory(VK_NULL_HANDLE),
    m_depthImageView(VK_NULL_HANDLE),
    m_stagingImage(VK_NULL_HANDLE), m_stagingImageMemory(VK_NULL_HANDLE), 
    m_vertexBuffer(VK_NULL_HANDLE), m_vertexBufferMemory(VK_NULL_HANDLE),
    m_indexBuffer(VK_NULL_HANDLE), m_indexBufferMemory(VK_NULL_HANDLE),
    m_uniformStagingBuffer(VK_NULL_HANDLE), m_uniformStagingBufferMemory(VK_NULL_HANDLE),
    m_uniformBuffer(VK_NULL_HANDLE), m_uniformBufferMemory(VK_NULL_HANDLE),
    m_descriptorPool(VK_NULL_HANDLE),
    m_imageAvailableSemaphore(VK_NULL_HANDLE), m_renderFinishedSemaphore(VK_NULL_HANDLE)
{
    Bind(wxEVT_PAINT, &VulkanCanvas::OnPaint, this);
    Bind(wxEVT_SIZE, &VulkanCanvas::OnResize, this);
    Bind(wxEVT_TIMER, &VulkanCanvas::OnTimer, this);
    std::vector<const char*> requiredExtensions = { "VK_KHR_surface", "VK_KHR_win32_surface" };
    InitializeVulkan(requiredExtensions);
    VkApplicationInfo appInfo = CreateApplicationInfo("VulkanApp1");
    std::vector<const char*> layerNames;
	if (enableValidationLayers) {
		layerNames = validationLayers;
	}
    VkInstanceCreateInfo createInfo = CreateInstanceCreateInfo(appInfo, requiredExtensions, layerNames);
    CreateInstance(createInfo);
    CreateWindowSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain(size);
    CreateImageViews();
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline("modelsvert.spv", "modelsfrag.spv");
    CreateCommandPool();
    CreateDepthResources();
    CreateFrameBuffers();
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffer();
    CreateDescriptorPool();
    CreateDescriptorSet();
    CreateCommandBuffers();
    CreateSemaphores();

    m_startTime = std::chrono::high_resolution_clock::now();
    m_timer = std::make_unique<wxTimer>(this, TIMERNUMBER);
    m_timer->Start(INTERVAL);
}


VulkanCanvas::~VulkanCanvas() noexcept
{
    m_timer->Stop();

    if (m_instance != VK_NULL_HANDLE) {
        if (m_logicalDevice != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(m_logicalDevice);
            if (m_graphicsPipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
            }
            if (m_descriptorSetLayout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(m_logicalDevice, m_descriptorSetLayout, nullptr);
            }
            if (m_pipelineLayout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
            }
            if (m_renderPass != VK_NULL_HANDLE) {
                vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);
            }
            if (m_swapchain != VK_NULL_HANDLE) {
                vkDestroySwapchainKHR(m_logicalDevice, m_swapchain, nullptr);
            }
            for (auto& imageView : m_swapchainImageViews) {
                vkDestroyImageView(m_logicalDevice, imageView, nullptr);
            }
            for (auto& framebuffer : m_swapchainFramebuffers) {
                vkDestroyFramebuffer(m_logicalDevice, framebuffer, nullptr);
            }
            if (m_stagingImageMemory != VK_NULL_HANDLE) {
                vkFreeMemory(m_logicalDevice, m_stagingImageMemory, nullptr);
            }
            if (m_stagingImage != VK_NULL_HANDLE) {
                vkDestroyImage(m_logicalDevice, m_stagingImage, nullptr);
            }
            if (m_vertexBufferMemory != VK_NULL_HANDLE) {
                vkFreeMemory(m_logicalDevice, m_vertexBufferMemory, nullptr);
            }
            if (m_vertexBuffer != VK_NULL_HANDLE) {
                vkDestroyBuffer(m_logicalDevice, m_vertexBuffer, nullptr);
            }
            if (m_indexBufferMemory != VK_NULL_HANDLE) {
                vkFreeMemory(m_logicalDevice, m_indexBufferMemory, nullptr);
            }
            if (m_indexBuffer != VK_NULL_HANDLE) {
                vkDestroyBuffer(m_logicalDevice, m_indexBuffer, nullptr);
            }
            if (m_uniformStagingBuffer != VK_NULL_HANDLE) {
                vkDestroyBuffer(m_logicalDevice, m_uniformStagingBuffer, nullptr);
            }
            if (m_uniformStagingBufferMemory != VK_NULL_HANDLE) {
                vkFreeMemory(m_logicalDevice, m_uniformStagingBufferMemory, nullptr);
            }
            if (m_uniformBuffer != VK_NULL_HANDLE) {
                vkDestroyBuffer(m_logicalDevice, m_uniformBuffer, nullptr);
            }
            if (m_uniformBufferMemory != VK_NULL_HANDLE) {
                vkFreeMemory(m_logicalDevice, m_uniformBufferMemory, nullptr);
            }
            if (m_descriptorPool != VK_NULL_HANDLE) {
                vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
            }
            if (m_textureImageMemory != VK_NULL_HANDLE) {
                vkFreeMemory(m_logicalDevice, m_textureImageMemory, nullptr);
            }
            if (m_textureImage != VK_NULL_HANDLE) {
                vkDestroyImage(m_logicalDevice, m_textureImage, nullptr);
            }
            if (m_textureImageView != VK_NULL_HANDLE) {
                vkDestroyImageView(m_logicalDevice, m_textureImageView, nullptr);
            }
            if (m_textureSampler != VK_NULL_HANDLE) {
                vkDestroySampler(m_logicalDevice, m_textureSampler, nullptr);
            }
            if (m_depthImageView != VK_NULL_HANDLE) {
                vkDestroyImageView(m_logicalDevice, m_depthImageView, nullptr);
            }
            if (m_depthImageMemory != VK_NULL_HANDLE) {
                vkFreeMemory(m_logicalDevice, m_depthImageMemory, nullptr);
            }
            if (m_depthImage != VK_NULL_HANDLE) {
                vkDestroyImage(m_logicalDevice, m_depthImage, nullptr);
            }
            if (m_commandPool != VK_NULL_HANDLE) {
                vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
            }
            if (m_imageAvailableSemaphore != VK_NULL_HANDLE) {
                vkDestroySemaphore(m_logicalDevice, m_imageAvailableSemaphore, nullptr);
            }
            if (m_renderFinishedSemaphore != VK_NULL_HANDLE) {
                vkDestroySemaphore(m_logicalDevice, m_renderFinishedSemaphore, nullptr);
            }
            vkDestroyDevice(m_logicalDevice, nullptr);
        }
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr);
    }
}

void VulkanCanvas::InitializeVulkan(std::vector<const char*> requiredExtensions)
{
    // make sure that the Vulkan library is available on this system
#ifdef _WIN32
    HMODULE vulkanModule = ::LoadLibraryA("vulkan-1.dll");
    if (vulkanModule == NULL) {
        throw std::runtime_error("Vulkan library is not available on this system, so program cannot run.\n"
            "You must install the appropriate Vulkan library and also have a graphics card that supports Vulkan.");
    }
#else
#error Only Win32 is currently supported. To see how to support other windowing systems, \
 see the definition of _glfw_dlopen in XXX_platform.h and its use in vulkan.c in the glfw\
 source code. XXX specifies the windowing system (e.g. x11 for X11, and wl for Wayland).
#endif
    // make sure that the correct extensions are available
    uint32_t count;
    VkResult err = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    if (err != VK_SUCCESS) {
        throw VulkanException(err, "Failed to retrieve the instance extension properties:");
    }
    std::vector<VkExtensionProperties> extensions(count);
    err = vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
    if (err != VK_SUCCESS) {
        throw VulkanException(err, "Failed to retrieve the instance extension properties:");
    }
    for(int extNum = 0; extNum < extensions.size(); ++extNum) {
        for (auto& iter = requiredExtensions.begin(); iter < requiredExtensions.end(); ++iter) {
            if (std::string(*iter) == extensions[extNum].extensionName) {
                requiredExtensions.erase(iter);
                break;
            }
        }
    };
    if (!requiredExtensions.empty()) {
        std::stringstream ss;
        ss << "The following required Vulkan extensions could not be found:\n";
        for (int extNum = 0; extNum < requiredExtensions.size(); ++extNum) {
            ss << requiredExtensions[extNum] << "\n";
        }
        ss << "Program cannot continue.";
        throw std::runtime_error(ss.str());
    }

    m_vulkanInitialized = true;
}

VkApplicationInfo VulkanCanvas::CreateApplicationInfo(const std::string& appName,
    const int32_t appVersion,
    const std::string& engineName,
    const int32_t engineVersion,
    const int32_t apiVersion) const noexcept
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName.c_str();
    appInfo.applicationVersion = appVersion;
    appInfo.pEngineName = engineName.c_str();
    appInfo.engineVersion = engineVersion;
    appInfo.apiVersion = apiVersion;
    return appInfo;
}

VkInstanceCreateInfo VulkanCanvas::CreateInstanceCreateInfo(const VkApplicationInfo& appInfo,
    const std::vector<const char*>& extensionNames,
    const std::vector<const char*>& layerNames) const noexcept
{
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensionNames.size();
    createInfo.ppEnabledExtensionNames = extensionNames.data();
    createInfo.enabledLayerCount = layerNames.size();
    createInfo.ppEnabledLayerNames = layerNames.data();
    return createInfo;
}

void VulkanCanvas::CreateInstance(const VkInstanceCreateInfo& createInfo)
{
    if (!m_vulkanInitialized) {
        throw std::runtime_error("Programming Error:\nAttempted to create a Vulkan instance before Vulkan was initialized.");
    }
    VkResult err = vkCreateInstance(&createInfo, nullptr, &m_instance);
    if (err != VK_SUCCESS) {
        throw VulkanException(err, "Unable to create a Vulkan instance:");
    }
}

#ifdef _WIN32
VkWin32SurfaceCreateInfoKHR VulkanCanvas::CreateWin32SurfaceCreateInfo() const noexcept
{
    VkWin32SurfaceCreateInfoKHR sci = {};
    sci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    sci.hwnd = GetHwnd();
    sci.hinstance = GetModuleHandle(NULL);
    return sci;
}
#endif

void VulkanCanvas::CreateWindowSurface()
{
    if (!m_instance) {
        throw std::runtime_error("Programming Error:\n"
            "Attempted to create a window surface before the Vulkan instance was created.");
    }
#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR sci = CreateWin32SurfaceCreateInfo();
    VkResult err = vkCreateWin32SurfaceKHR(m_instance, &sci, nullptr, &m_surface);
    if (err != VK_SUCCESS) {
        throw VulkanException(err, "Cannot create a Win32 Vulkan surface:");
    }
#else
#error The code in VulkanCanvas::CreateWindowSurface only supports Win32. Changes are \
required to support other windowing systems.
#endif
}

void VulkanCanvas::PickPhysicalDevice()
{
    if (!m_instance) {
        throw std::runtime_error("Programming Error:\n"
            "Attempted to get a Vulkan physical device before the Vulkan instance was created.");
    }
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find a GPU with Vulkan support.");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
    for (const auto& device : devices) {
        if (IsDeviceSuitable(device)) {
            m_physicalDevice = device;
            break;
        }
    }
    if (m_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("No physical GPU could be found with the required extensions and swap chain support.");
    }
}

bool VulkanCanvas::IsDeviceSuitable(const VkPhysicalDevice& device) const
{
    QueueFamilyIndices indices = FindQueueFamilies(device);
    bool extensionsSupported = CheckDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    return indices.IsComplete() & extensionsSupported && swapChainAdequate;
}

QueueFamilyIndices VulkanCanvas::FindQueueFamilies(const VkPhysicalDevice& device) const
{
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
        if (result != VK_SUCCESS) {
            throw VulkanException(result, "Error while attempting to check if a surface supports presentation:");
        }
        if (queueFamily.queueCount > 0 && presentSupport) {
            indices.presentFamily = i;
        }
        if (indices.IsComplete()) {
            break;
        }
        ++i;
    }
    return indices;
}

bool VulkanCanvas::CheckDeviceExtensionSupport(const VkPhysicalDevice& device) const
{
    uint32_t extensionCount;
    VkResult result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Cannot retrieve count of properties for a physical device:");
    }
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Cannot retrieve properties for a physical device:");
    }
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails VulkanCanvas::QuerySwapChainSupport(const VkPhysicalDevice& device) const
{
    SwapChainSupportDetails details;

    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Unable to retrieve physical device surface capabilities:");
    }
    uint32_t formatCount = 0;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Unable to retrieve the number of formats for a surface on a physical device:");
    }
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
        if (result != VK_SUCCESS) {
            throw VulkanException(result, "Unable to retrieve the formats for a surface on a physical device:");
        }
    }

    uint32_t presentModeCount = 0;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Unable to retrieve the count of present modes for a surface on a physical device:");
    }
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        result =vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
        if (result != VK_SUCCESS) {
            throw VulkanException(result, "Unable to retrieve the present modes for a surface on a physical device:");
        }
    }
    return details;
}

VkDeviceQueueCreateInfo VulkanCanvas::CreateDeviceQueueCreateInfo(int queueFamily) const noexcept
{
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    return queueCreateInfo;
}

std::vector<VkDeviceQueueCreateInfo> VulkanCanvas::CreateQueueCreateInfos(
    const std::set<int>& uniqueQueueFamilies) const noexcept
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    for (int queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = CreateDeviceQueueCreateInfo(queueFamily);
        queueCreateInfos.push_back(queueCreateInfo);
    }
    return queueCreateInfos;
}

VkDeviceCreateInfo VulkanCanvas::CreateDeviceCreateInfo(
    const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos,
    const VkPhysicalDeviceFeatures& deviceFeatures) const noexcept
{
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }
    return createInfo;
}

void VulkanCanvas::CreateLogicalDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
    std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = CreateQueueCreateInfos(uniqueQueueFamilies);
    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkDeviceCreateInfo createInfo = CreateDeviceCreateInfo(queueCreateInfos, deviceFeatures);

    VkResult result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_logicalDevice);
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Unable to create a logical device");
    }
    vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily, 0, &m_presentQueue);
}

VkSwapchainCreateInfoKHR VulkanCanvas::CreateSwapchainCreateInfo(
    const SwapChainSupportDetails& swapChainSupport,
    const VkSurfaceFormatKHR& surfaceFormat,
    uint32_t imageCount, const VkExtent2D& extent)
{
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
    uint32_t queueFamilyIndices[] = { static_cast<uint32_t>(indices.graphicsFamily),
        static_cast<uint32_t>(indices.presentFamily) };
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    return createInfo;
}

void VulkanCanvas::CreateSwapChain(const wxSize& size)
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, size);
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR createInfo = CreateSwapchainCreateInfo(swapChainSupport,
        surfaceFormat, imageCount, extent);
    VkSwapchainKHR oldSwapchain = m_swapchain;
    createInfo.oldSwapchain = oldSwapchain;
    VkSwapchainKHR newSwapchain;
    VkResult result = vkCreateSwapchainKHR(m_logicalDevice, &createInfo, nullptr, &newSwapchain);
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Error attempting to create a swapchain:");
    }
    *&m_swapchain = newSwapchain;

    result = vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain, &imageCount, nullptr);
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Error attempting to retrieve the count of swapchain images:");
    }
    m_swapchainImages.resize(imageCount);
    result = vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain, &imageCount, m_swapchainImages.data());
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Error attempting to retrieve the swapchain images:");
    }
    m_swapchainImageFormat = surfaceFormat.format;
    m_swapchainExtent = extent;
}

VkSurfaceFormatKHR VulkanCanvas::ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats) const noexcept
{
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
        return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR VulkanCanvas::ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes) const noexcept
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanCanvas::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
    const wxSize& size) const noexcept
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
        VkExtent2D actualExtent = { static_cast<uint32_t>(size.GetWidth()), 
            static_cast<uint32_t>(size.GetHeight()) };
        actualExtent.width = std::max(capabilities.minImageExtent.width, 
            std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height,
            std::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }
}

void VulkanCanvas::CreateImageViews()
{
    m_swapchainImageViews.resize(m_swapchainImages.size());
    for (uint32_t i = 0; i < m_swapchainImages.size(); i++) {
        m_swapchainImageViews[i] = VK_NULL_HANDLE;
        CreateImageView(m_swapchainImages[i], m_swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 
            m_swapchainImageViews[i]);
    }
}

VkAttachmentDescription VulkanCanvas::CreateAttachmentDescription() const noexcept
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    return colorAttachment;
}

VkAttachmentReference VulkanCanvas::CreateAttachmentReference() const noexcept
{
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    return colorAttachmentRef;
}

VkSubpassDescription VulkanCanvas::CreateSubpassDescription(
    const VkAttachmentReference& colorAttachmentRef,
    const VkAttachmentReference& depthAttachmentRef) const noexcept
{
    VkSubpassDescription subPass = {};
    subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPass.colorAttachmentCount = 1;
    subPass.pColorAttachments = &colorAttachmentRef;
    subPass.pDepthStencilAttachment = &depthAttachmentRef; 
    return subPass;
}

VkSubpassDependency VulkanCanvas::CreateSubpassDependency() const noexcept
{
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    return dependency;
}

VkRenderPassCreateInfo VulkanCanvas::CreateRenderPassCreateInfo(
    const std::vector<VkAttachmentDescription>& attachments,
    const VkSubpassDescription& subPass,
    const VkSubpassDependency& dependency) const noexcept
{
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subPass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    return renderPassInfo;
}

void VulkanCanvas::CreateRenderPass() 
{
    VkAttachmentDescription colorAttachment = CreateAttachmentDescription();
    VkAttachmentReference colorAttachmentRef = CreateAttachmentReference();
    VkAttachmentDescription depthAttachment = CreateDepthAttachmentDescription();
    VkAttachmentReference depthAttachmentRef = CreateDepthAttachmentReference();
    VkSubpassDescription subPass = CreateSubpassDescription(colorAttachmentRef,
        depthAttachmentRef);
    VkSubpassDependency dependency = CreateSubpassDependency();
    std::vector<VkAttachmentDescription> attachments{ colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo = CreateRenderPassCreateInfo(
    attachments, subPass, dependency);

    VkResult result = vkCreateRenderPass(m_logicalDevice, &renderPassInfo, nullptr, &m_renderPass);
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to create a render pass:");
    }
}

VkPipelineShaderStageCreateInfo VulkanCanvas::CreatePipelineShaderStageCreateInfo(
    VkShaderStageFlagBits stage, VkShaderModule& module, const char* entryName) const noexcept
{
    VkPipelineShaderStageCreateInfo shaderStageInfo = {};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = stage;
    shaderStageInfo.module = module;
    shaderStageInfo.pName = entryName;
    return shaderStageInfo;
}

VkPipelineVertexInputStateCreateInfo VulkanCanvas::CreatePipelineVertexInputStateCreateInfo() noexcept
{
    m_bindingDescription = Vertex::getBindingDescription();
    m_attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = m_attributeDescriptions.size();
    vertexInputInfo.pVertexBindingDescriptions = &m_bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = m_attributeDescriptions.data();
    return vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo VulkanCanvas::CreatePipelineInputAssemblyStateCreateInfo(
    const VkPrimitiveTopology& topology, uint32_t restartEnable) const noexcept
{
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = topology;
    inputAssembly.primitiveRestartEnable = restartEnable;
    return inputAssembly;
}

VkViewport VulkanCanvas::CreateViewport() const noexcept
{
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_swapchainExtent.width;
    viewport.height = (float)m_swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    return viewport;
}

VkRect2D VulkanCanvas::CreateScissor() const noexcept
{
    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = m_swapchainExtent;
    return scissor;
}

VkPipelineViewportStateCreateInfo VulkanCanvas::CreatePipelineViewportStateCreateInfo(
    const VkViewport& viewport, const VkRect2D& scissor) const noexcept
{
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    return viewportState;
}

VkPipelineRasterizationStateCreateInfo VulkanCanvas::CreatePipelineRasterizationStateCreateInfo() const noexcept
{
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo VulkanCanvas::CreatePipelineMultisampleStateCreateInfo() const noexcept
{
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    return multisampling;
}

VkPipelineColorBlendAttachmentState VulkanCanvas::CreatePipelineColorBlendAttachmentState() const noexcept
{
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    return colorBlendAttachment;
}

VkPipelineColorBlendStateCreateInfo VulkanCanvas::CreatePipelineColorBlendStateCreateInfo(
    const VkPipelineColorBlendAttachmentState& colorBlendAttachment) const noexcept
{
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;
    return colorBlending;
}

void VulkanCanvas::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();

    VkResult result = vkCreateDescriptorSetLayout(m_logicalDevice, &layoutInfo, nullptr,
        &m_descriptorSetLayout);
    if(result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to create descriptor set layout!");
    }
}

VkPipelineLayoutCreateInfo VulkanCanvas::CreatePipelineLayoutCreateInfo() noexcept
{
    m_descriptorSetLayouts = { m_descriptorSetLayout };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = m_descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = m_descriptorSetLayouts.data();
    return pipelineLayoutInfo;
}

VkGraphicsPipelineCreateInfo VulkanCanvas::CreateGraphicsPipelineCreateInfo(
    const VkPipelineShaderStageCreateInfo shaderStages[],
    const VkPipelineVertexInputStateCreateInfo& vertexInputInfo,
    const VkPipelineInputAssemblyStateCreateInfo& inputAssembly,
    const VkPipelineViewportStateCreateInfo& viewportState,
    const VkPipelineRasterizationStateCreateInfo& rasterizer,
    const VkPipelineMultisampleStateCreateInfo& multisampling,
    const VkPipelineColorBlendStateCreateInfo& colorBlending,
    const VkPipelineDepthStencilStateCreateInfo& depthStencil) const noexcept
{
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencil;
    return pipelineInfo;
}

VkPipelineDepthStencilStateCreateInfo VulkanCanvas::CreatePipelineDepthStencilStateCreateInfo()
{
    VkPipelineDepthStencilStateCreateInfo depthStencil {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    return depthStencil;
}

void VulkanCanvas::CreateGraphicsPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile)
{
    auto vertShaderCode = ReadFile(vertexShaderFile);
    auto fragShaderCode = ReadFile(fragmentShaderFile);

    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
   
    CreateShaderModule(vertShaderCode, vertShaderModule);
    CreateShaderModule(fragShaderCode, fragShaderModule);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = CreatePipelineShaderStageCreateInfo(
        VK_SHADER_STAGE_VERTEX_BIT, vertShaderModule, "main");
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = CreatePipelineShaderStageCreateInfo(
        VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderModule, "main");
    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = CreatePipelineVertexInputStateCreateInfo();
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = CreatePipelineInputAssemblyStateCreateInfo(
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
    VkViewport viewport = CreateViewport();
    VkRect2D scissor = CreateScissor();
    VkPipelineViewportStateCreateInfo viewportState = CreatePipelineViewportStateCreateInfo(
        viewport, scissor);
    VkPipelineRasterizationStateCreateInfo rasterizer = CreatePipelineRasterizationStateCreateInfo();
    VkPipelineMultisampleStateCreateInfo multisampling = CreatePipelineMultisampleStateCreateInfo();
    VkPipelineDepthStencilStateCreateInfo depthStencil = CreatePipelineDepthStencilStateCreateInfo();
    VkPipelineColorBlendAttachmentState colorBlendAttachment = CreatePipelineColorBlendAttachmentState();
    VkPipelineColorBlendStateCreateInfo colorBlending = CreatePipelineColorBlendStateCreateInfo(
        colorBlendAttachment);
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = CreatePipelineLayoutCreateInfo();

    VkResult result = vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
    if(result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to create pipeline layout:");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = CreateGraphicsPipelineCreateInfo(shaderStages,
        vertexInputInfo, inputAssembly, viewportState, rasterizer, multisampling, colorBlending,
        depthStencil);


    result = vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline);
    // vkDestroyShaderModule calls below must be placed before possible throw of exception
    vkDestroyShaderModule(m_logicalDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_logicalDevice, vertShaderModule, nullptr);
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to create graphics pipeline:");
    }
}

std::vector<char> VulkanCanvas::ReadFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        std::stringstream ss;
        ss << "Failed to open file: " << filename;
        throw std::runtime_error(ss.str().c_str());
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModuleCreateInfo VulkanCanvas::CreateShaderModuleCreateInfo(
    const std::vector<char>& code) const noexcept
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = (uint32_t*)code.data();
    return createInfo;
}

void VulkanCanvas::CreateShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule) const
{
    VkShaderModuleCreateInfo createInfo = CreateShaderModuleCreateInfo(code);

    VkResult result = vkCreateShaderModule(m_logicalDevice, &createInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to create shader module:");
    }
}

VkFramebufferCreateInfo VulkanCanvas::CreateFramebufferCreateInfo(
    const std::vector<VkImageView>& attachments) const noexcept
{
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_swapchainExtent.width;
    framebufferInfo.height = m_swapchainExtent.height;
    framebufferInfo.layers = 1;
    return framebufferInfo;
}

void VulkanCanvas::CreateFrameBuffers()
{
    VkFramebuffer framebuffer;
    m_swapchainFramebuffers.resize(m_swapchainImageViews.size(), framebuffer);

    std::vector < VkImageView> attachments;
    for (size_t i = 0; i < m_swapchainImageViews.size(); i++) {
        attachments = { m_swapchainImageViews[i], m_depthImageView };
        VkFramebufferCreateInfo framebufferInfo = CreateFramebufferCreateInfo(attachments);

        VkResult result = vkCreateFramebuffer(m_logicalDevice, &framebufferInfo, nullptr, &m_swapchainFramebuffers[i]);
        if (result != VK_SUCCESS) {
            throw VulkanException(result, "Failed to create framebuffer:");
        }
    }
}

uint32_t VulkanCanvas::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

void VulkanCanvas::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
    VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(m_logicalDevice, &bufferInfo, nullptr, &buffer);
    if(result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_logicalDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(m_logicalDevice, &allocInfo, nullptr, &bufferMemory);
    if(result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_logicalDevice, buffer, bufferMemory, 0);
}

void VulkanCanvas::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    EndSingleTimeCommands(commandBuffer);
}

void VulkanCanvas::CopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkImageSubresourceLayers subResource = {};
    subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subResource.baseArrayLayer = 0;
    subResource.mipLevel = 0;
    subResource.layerCount = 1;

    VkImageCopy region = {};
    region.srcSubresource = subResource;
    region.dstSubresource = subResource;
    region.srcOffset = { 0, 0, 0 };
    region.dstOffset = { 0, 0, 0 };
    region.extent.width = width;
    region.extent.height = height;
    region.extent.depth = 1;

    vkCmdCopyImage(
        commandBuffer,
        srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &region
    );

    EndSingleTimeCommands(commandBuffer);
}

void VulkanCanvas::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (HasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED 
        && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED 
        && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL 
        && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED 
        && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT 
            | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    else {
        throw std::invalid_argument("Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    EndSingleTimeCommands(commandBuffer);
}

void VulkanCanvas::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, 
    VkImageView& imageView)
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkResult result = vkCreateImageView(m_logicalDevice, &viewInfo, nullptr, &imageView);
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to create texture image view!");
    }
}

void VulkanCanvas::CreateTextureSampler()
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkResult result = vkCreateSampler(m_logicalDevice, &samplerInfo, nullptr, &m_textureSampler);
    if(result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to create texture sampler!");
    }
}

void VulkanCanvas::CreateTextureImageView() 
{
    CreateImageView(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, m_textureImageView);
}

void VulkanCanvas::CreateVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

    VkBuffer stagingBuffer{ VK_NULL_HANDLE };
    VkDeviceMemory stagingBufferMemory{ VK_NULL_HANDLE };
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_logicalDevice, stagingBufferMemory);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

    CopyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

    vkFreeMemory(m_logicalDevice, stagingBufferMemory, nullptr);
    vkDestroyBuffer(m_logicalDevice, stagingBuffer, nullptr);
}

void VulkanCanvas::CreateIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

    VkBuffer stagingBuffer { VK_NULL_HANDLE };
    VkDeviceMemory stagingBufferMemory { VK_NULL_HANDLE };
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_indices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_logicalDevice, stagingBufferMemory);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

    CopyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

    vkFreeMemory(m_logicalDevice, stagingBufferMemory, nullptr);
    vkDestroyBuffer(m_logicalDevice, stagingBuffer, nullptr);
}

VkCommandPoolCreateInfo VulkanCanvas::CreateCommandPoolCreateInfo(
    QueueFamilyIndices& queueFamilyIndices) const noexcept
{
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
    return poolInfo;
}

void VulkanCanvas::CreateUniformBuffer()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformStagingBuffer, m_uniformStagingBufferMemory);
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_uniformBuffer, m_uniformBufferMemory);
}

VkFormat VulkanCanvas::FindDepthFormat() const
{
    return FindSupportedFormat(
    { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool VulkanCanvas::HasStencilComponent(VkFormat format) const noexcept
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

    VkFormat VulkanCanvas::FindSupportedFormat(const std::vector<VkFormat>& candidates,
    VkImageTiling tiling, VkFormatFeatureFlags features) const
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("Failed to find supported format.");
}


void VulkanCanvas::CreateDepthResources()
{
    VkFormat depthFormat = FindDepthFormat();
    CreateImage(m_swapchainExtent.width, m_swapchainExtent.height, depthFormat, 
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
    CreateImageView(m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, m_depthImageView);

    TransitionImageLayout(m_depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, 
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void VulkanCanvas::CreateDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;
    VkResult result = vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, &m_descriptorPool);
    if(result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to create descriptor pool!");
    }
}

void VulkanCanvas::CreateDescriptorSet()
{
    VkDescriptorSetLayout layouts[] = { m_descriptorSetLayout };
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;

    VkResult result = vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, &m_descriptorSet);
    if(result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to allocate descriptor set!");
    }
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = m_uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_textureImageView;
    imageInfo.sampler = m_textureSampler;

    std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = m_descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = m_descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_logicalDevice, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void VulkanCanvas::CreateCommandPool() {
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice);
    VkCommandPoolCreateInfo poolInfo = CreateCommandPoolCreateInfo(queueFamilyIndices);
    VkResult result = vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_commandPool);
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to create command pool:");
    }
}

void VulkanCanvas::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateImage(m_logicalDevice, &imageInfo, nullptr, &image);
    if(result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_logicalDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(m_logicalDevice, &allocInfo, nullptr, &imageMemory);
    if(result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to allocate image memory!");
    }

    vkBindImageMemory(m_logicalDevice, image, imageMemory, 0);
}

VkCommandBuffer VulkanCanvas::BeginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanCanvas::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, &commandBuffer);
}

void VulkanCanvas::CreateTextureImage()
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("Failed to load texture image!");
    }

    CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, 
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_stagingImage, m_stagingImageMemory);

    VkImageSubresource subresource = {};
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.mipLevel = 0;
    subresource.arrayLayer = 0;

    VkSubresourceLayout stagingImageLayout;
    vkGetImageSubresourceLayout(m_logicalDevice, m_stagingImage, &subresource, &stagingImageLayout);

    void* data;
    vkMapMemory(m_logicalDevice, m_stagingImageMemory, 0, imageSize, 0, &data);

    if (stagingImageLayout.rowPitch == texWidth * 4) {
        memcpy(data, pixels, (size_t)imageSize);
    }
    else {
        uint8_t* dataBytes = reinterpret_cast<uint8_t*>(data);

        for (int y = 0; y < texHeight; y++) {
            memcpy(&dataBytes[y * stagingImageLayout.rowPitch], &pixels[y * texWidth * 4], texWidth * 4);
        }
    }

    vkUnmapMemory(m_logicalDevice, m_stagingImageMemory);
    stbi_image_free(pixels);

    CreateImage(
        texWidth, texHeight,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_textureImage,
        m_textureImageMemory
    );

    TransitionImageLayout(m_stagingImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyImage(m_stagingImage, m_textureImage, texWidth, texHeight);
    TransitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

VkCommandBufferAllocateInfo VulkanCanvas::CreateCommandBufferAllocateInfo() const noexcept
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();
    return allocInfo;
}

VkCommandBufferBeginInfo VulkanCanvas::CreateCommandBufferBeginInfo() const noexcept
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    return beginInfo;
}

VkRenderPassBeginInfo VulkanCanvas::CreateRenderPassBeginInfo(size_t swapchainBufferNumber,
    std::vector<VkClearValue>& clearValues) const noexcept
{
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapchainFramebuffers[swapchainBufferNumber];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swapchainExtent;

    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();
    return renderPassInfo;
}

void VulkanCanvas::CreateCommandBuffers()
{
    if (m_commandBuffers.size() > 0) {
        vkFreeCommandBuffers(m_logicalDevice, m_commandPool, m_commandBuffers.size(), m_commandBuffers.data());
    }
    m_commandBuffers.resize(m_swapchainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = CreateCommandBufferAllocateInfo();
    VkResult result = vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, m_commandBuffers.data());
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to allocate command buffers:");
    }

    std::vector<VkClearValue> clearValues = { {},{} };
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    for (size_t i = 0; i < m_commandBuffers.size(); i++) {

        VkCommandBufferBeginInfo beginInfo = CreateCommandBufferBeginInfo();
        vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo);

        VkRenderPassBeginInfo renderPassInfo = CreateRenderPassBeginInfo(i, clearValues);
        vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
        VkBuffer vertexBuffers[] = { m_vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(m_commandBuffers[i], m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, 
            m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
        vkCmdDrawIndexed(m_commandBuffers[i], m_indices.size(), 1, 0, 0, 0);

        vkCmdEndRenderPass(m_commandBuffers[i]);

        result = vkEndCommandBuffer(m_commandBuffers[i]);
        if (result != VK_SUCCESS) {
            throw VulkanException(result, "Failed to record command buffer:");
        }
    }
}

VkSemaphoreCreateInfo VulkanCanvas::CreateSemaphoreCreateInfo() const noexcept
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    return semaphoreInfo;
}

void VulkanCanvas::CreateSemaphores()
{
    VkSemaphoreCreateInfo semaphoreInfo = CreateSemaphoreCreateInfo();

    VkResult result = vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore);
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to create image available semaphore:");
    }
    result = vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore);
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to create render finished semaphore:");
    }
}

VkAttachmentDescription VulkanCanvas::CreateDepthAttachmentDescription() const noexcept
{
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    return depthAttachment;
}

VkAttachmentReference VulkanCanvas::CreateDepthAttachmentReference() const noexcept
{
    VkAttachmentReference depthAttachmentRef {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    return depthAttachmentRef;
}

void VulkanCanvas::RecreateSwapchain()
{
    vkDeviceWaitIdle(m_logicalDevice);

    wxSize size = GetSize();
    CreateSwapChain(size);
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline("modelsvert.spv", "modelsfrag.spv");
    CreateDepthResources();
    CreateFrameBuffers();
    CreateCommandBuffers();
}

VkSubmitInfo VulkanCanvas::CreateSubmitInfo(uint32_t imageIndex,
	VkPipelineStageFlags* waitStageFlags) const noexcept
{
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = waitStageFlags;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_renderFinishedSemaphore;
    return submitInfo;
}

VkPresentInfoKHR VulkanCanvas::CreatePresentInfoKHR(uint32_t& imageIndex) const noexcept
{
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_renderFinishedSemaphore;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;

    presentInfo.pImageIndices = &imageIndex;
    return presentInfo;
}

void VulkanCanvas::UpdateUniformBuffer()
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_startTime).count() / 1000.0f;
    UniformBufferObject ubo = {};
    ubo.model = glm::rotate(glm::mat4(), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), m_swapchainExtent.width / (float)m_swapchainExtent.height, 0.1f, 10.0f);

    void* data;
    VkResult result = vkMapMemory(m_logicalDevice, m_uniformStagingBufferMemory, 0, sizeof(ubo), 0, &data);
    if (result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to map uniformStagingBufferMemory");
    }
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(m_logicalDevice, m_uniformStagingBufferMemory);

    CopyBuffer(m_uniformStagingBuffer, m_uniformBuffer, sizeof(ubo));
}

void VulkanCanvas::OnPaint(wxPaintEvent& event)
{
    try {
        UpdateUniformBuffer();

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_logicalDevice, m_swapchain,
            std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapchain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw VulkanException(result, "Failed to acquire swap chain image");
        }
		VkPipelineStageFlags waitFlags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo submitInfo = CreateSubmitInfo(imageIndex, waitFlags);
        result = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        if (result != VK_SUCCESS) {
            throw VulkanException(result, "Failed to submit draw command buffer:");
        }

        VkPresentInfoKHR presentInfo = CreatePresentInfoKHR(imageIndex);
        result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            RecreateSwapchain();
        }
        else if (result != VK_SUCCESS) {
            throw VulkanException(result, "Failed to present swap chain image:");
        }
    }
    catch (const VulkanException& ve) {
        std::string status = ve.GetStatus();
        std::stringstream ss;
        ss << ve.what() << "\n" << status;
        CallAfter(&VulkanCanvas::OnPaintException, ss.str());
    }
    catch (const std::exception& err) {
        std::stringstream ss;
        ss << "Error encountered trying to create the Vulkan canvas:\n";
        ss << err.what();
        CallAfter(&VulkanCanvas::OnPaintException, ss.str());
    }
}

void VulkanCanvas::OnResize(wxSizeEvent& event)
{
    wxSize size = GetSize();
    if (size.GetWidth() == 0 || size.GetHeight() == 0) {
        return;
    }
    RecreateSwapchain();
    wxRect refreshRect(size);
    RefreshRect(refreshRect, false);
}

void VulkanCanvas::OnTimer(wxTimerEvent&)
{
    ProcessEvent(wxPaintEvent());
}

void VulkanCanvas::OnPaintException(const std::string& msg)
{
    wxMessageBox(msg, "Vulkan Error");
    wxTheApp->ExitMainLoop();
}

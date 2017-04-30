#pragma once
#include "wx/wxprec.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <string>
#include <set>
#include <array>
#include <memory>
#include <chrono>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct QueueFamilyIndices {
    int graphicsFamily = -1;
    int presentFamily = -1;

    bool IsComplete() {
        return graphicsFamily >= 0 && presentFamily >= 0;
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class VulkanCanvas :
    public wxWindow
{
public:
    VulkanCanvas(wxWindow *pParent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxString& name = "VulkanCanvasName");

    virtual ~VulkanCanvas() noexcept;

private:
    void InitializeVulkan(std::vector<const char*> extensions);
    void CreateInstance(const VkInstanceCreateInfo& createInfo);
    void CreateWindowSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapChain(const wxSize& size);
    void CreateImageViews();
    void CreateRenderPass();
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
    void CreateFrameBuffers();
    void CreateCommandPool();
    void CreateTextureImage();
    void CreateTextureImageView();
    void CreateTextureSampler();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateUniformBuffer();
    void CreateDescriptorPool();
    void CreateDescriptorSet();
    void CreateCommandBuffers();
    void CreateSemaphores();
    void RecreateSwapchain();
    void UpdateUniformBuffer();
    VkWin32SurfaceCreateInfoKHR VulkanCanvas::CreateWin32SurfaceCreateInfo() const noexcept;
    VkDeviceQueueCreateInfo CreateDeviceQueueCreateInfo(int queueFamily) const noexcept;
    VkApplicationInfo CreateApplicationInfo(const std::string& appName,
        const int32_t appVersion = VK_MAKE_VERSION(1, 0, 0),
        const std::string& engineName = "No Engine",
        const int32_t engineVersion = VK_MAKE_VERSION(1, 0, 0),
        const int32_t apiVersion = VK_API_VERSION_1_0) const noexcept;
    VkInstanceCreateInfo CreateInstanceCreateInfo(const VkApplicationInfo& appInfo,
        const std::vector<const char*>& extensionNames,
        const std::vector<const char*>& layerNames) const noexcept;
    std::vector<VkDeviceQueueCreateInfo> VulkanCanvas::CreateQueueCreateInfos(
        const std::set<int>& uniqueQueueFamilies) const noexcept;
    VkDeviceCreateInfo CreateDeviceCreateInfo(
        const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos,
        const VkPhysicalDeviceFeatures& deviceFeatures) const noexcept;
    VkSwapchainCreateInfoKHR CreateSwapchainCreateInfo(
        const SwapChainSupportDetails& swapChainSupport,
        const VkSurfaceFormatKHR& surfaceFormat,
        uint32_t imageCount,
        const VkExtent2D& extent);
    VkAttachmentDescription CreateAttachmentDescription() const noexcept;
    VkAttachmentReference CreateAttachmentReference() const noexcept;
    VkSubpassDescription CreateSubpassDescription(const VkAttachmentReference& attachmentRef) const noexcept;
    VkSubpassDependency CreateSubpassDependency() const noexcept;
    VkRenderPassCreateInfo CreateRenderPassCreateInfo(
        const VkAttachmentDescription& colorAttachment,
        const VkSubpassDescription& subPass,
        const VkSubpassDependency& dependency) const noexcept;
    VkPipelineShaderStageCreateInfo CreatePipelineShaderStageCreateInfo(
        VkShaderStageFlagBits stage, VkShaderModule& module, const char* entryName) const noexcept;
    VkPipelineVertexInputStateCreateInfo CreatePipelineVertexInputStateCreateInfo() noexcept;
    VkPipelineInputAssemblyStateCreateInfo CreatePipelineInputAssemblyStateCreateInfo(
        const VkPrimitiveTopology& topology, uint32_t restartEnable) const noexcept;
    VkViewport CreateViewport() const noexcept;
    VkRect2D CreateScissor() const noexcept;
    VkPipelineViewportStateCreateInfo CreatePipelineViewportStateCreateInfo(
        const VkViewport& viewport, const VkRect2D& scissor) const noexcept;
    VkPipelineRasterizationStateCreateInfo CreatePipelineRasterizationStateCreateInfo() const noexcept;
    VkPipelineMultisampleStateCreateInfo CreatePipelineMultisampleStateCreateInfo() const noexcept;
    VkPipelineColorBlendAttachmentState CreatePipelineColorBlendAttachmentState() const noexcept;
    VkPipelineColorBlendStateCreateInfo CreatePipelineColorBlendStateCreateInfo(
        const VkPipelineColorBlendAttachmentState& colorBlendAttachment) const noexcept;
    VkPipelineLayoutCreateInfo CreatePipelineLayoutCreateInfo() noexcept;
    VkGraphicsPipelineCreateInfo CreateGraphicsPipelineCreateInfo(
        const VkPipelineShaderStageCreateInfo shaderStages[],
        const VkPipelineVertexInputStateCreateInfo& vertexInputInfo,
        const VkPipelineInputAssemblyStateCreateInfo& inputAssembly,
        const VkPipelineViewportStateCreateInfo& viewportState,
        const VkPipelineRasterizationStateCreateInfo& rasterizer,
        const VkPipelineMultisampleStateCreateInfo& multisampling,
        const VkPipelineColorBlendStateCreateInfo& colorBlending) const noexcept;
    VkShaderModuleCreateInfo CreateShaderModuleCreateInfo(
        const std::vector<char>& code) const noexcept;
    VkFramebufferCreateInfo CreateFramebufferCreateInfo(
        const VkImageView& attachments) const noexcept;
    VkCommandPoolCreateInfo CreateCommandPoolCreateInfo(QueueFamilyIndices& queueFamilyIndices) const noexcept;
    VkCommandBufferAllocateInfo CreateCommandBufferAllocateInfo() const noexcept;
    VkCommandBufferBeginInfo CreateCommandBufferBeginInfo() const noexcept;
    VkRenderPassBeginInfo CreateRenderPassBeginInfo(size_t swapchainBufferNumber) const noexcept;
    VkSemaphoreCreateInfo CreateSemaphoreCreateInfo() const noexcept;
    VkSubmitInfo CreateSubmitInfo(uint32_t imageIndex,
		VkPipelineStageFlags* pipelineStageFlags) const noexcept;
    VkPresentInfoKHR CreatePresentInfoKHR(uint32_t& imageIndex) const noexcept;
    bool IsDeviceSuitable(const VkPhysicalDevice& device) const;
    QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice& device) const;
    bool CheckDeviceExtensionSupport(const VkPhysicalDevice& device) const;
    SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice& device) const;
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const noexcept;
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const noexcept;
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const wxSize& size) const noexcept;
    static std::vector<char> ReadFile(const std::string& filename);
    void CreateShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule) const;
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);
    void CreateImageView(VkImage image, VkFormat format, VkImageView& imageView);
    virtual void OnPaint(wxPaintEvent& event);
    virtual void OnResize(wxSizeEvent& event);
    virtual void OnTimer(wxTimerEvent& event);
    void OnPaintException(const std::string& msg);

    static const int INTERVAL = 1000 / 60;
    static const int TIMERNUMBER = 3;
    std::unique_ptr<wxTimer> m_timer;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;

    VkInstance m_instance;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_logicalDevice;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkSwapchainKHR m_swapchain;
    std::vector<VkImage> m_swapchainImages;
    VkFormat m_swapchainImageFormat;
    VkExtent2D m_swapchainExtent;
    std::vector<VkImageView> m_swapchainImageViews;
    VkRenderPass m_renderPass;
    VkDescriptorSetLayout m_descriptorSetLayout;
    std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;
    std::vector<VkFramebuffer> m_swapchainFramebuffers;
    VkImage m_stagingImage;
    VkDeviceMemory m_stagingImageMemory;
    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;
    VkBuffer m_uniformStagingBuffer;
    VkDeviceMemory m_uniformStagingBufferMemory;
    VkBuffer m_uniformBuffer;
    VkDeviceMemory m_uniformBufferMemory;
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSet m_descriptorSet;
    VkCommandPool m_commandPool;
    VkImage m_textureImage;
    VkDeviceMemory m_textureImageMemory;
    VkImageView m_textureImageView;
    VkSampler m_textureSampler;
    std::vector<VkCommandBuffer> m_commandBuffers;
    VkSemaphore m_imageAvailableSemaphore;
    VkSemaphore m_renderFinishedSemaphore;
    bool m_vulkanInitialized;
    const std::vector<Vertex> m_vertices {
        { { -0.5f, -0.5f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
        { { 0.5f, -0.5f, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
        { { 0.5f, 0.5f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
        { { -0.5f, 0.5f, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

        { { -0.5f, -0.5f, 0.5f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
        { { 0.5f, -0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
        { { 0.5f, 0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
        { { -0.5f, 0.5f, 0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } }
    };
    const std::vector<uint16_t> m_indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4
    };
    VkVertexInputBindingDescription m_bindingDescription;
    std::array<VkVertexInputAttributeDescription, 3> m_attributeDescriptions;
};

#include "VulkanException.h"
#include <sstream>

std::map<VkResult, std::string> VulkanException::m_statuses;

VulkanException::VulkanException(const VkResult status, const std::string& msg)
    : std::runtime_error(msg.c_str()), m_status(status)
{
    if (m_statuses.size() == 0) { 
        m_statuses = { { VkResult::VK_SUCCESS, "VK_SUCCESS: Command completed successfully" },
            { VkResult::VK_NOT_READY, "VK_NOT_READY: A fence or query has not yet completed" },
            { VkResult::VK_TIMEOUT, "VK_TIMEOUT: a wait operation has not completed in the specified time" },
            { VkResult::VK_EVENT_SET, "VK_EVENT_SET: An event is signaled" },
            { VkResult::VK_EVENT_RESET, "VK_EVENT_RESET: An even is unsignaled" },
            { VkResult::VK_INCOMPLETE, "VK_INCOMPLETE: A return array was too small for the result" },
            { VkResult::VK_ERROR_OUT_OF_HOST_MEMORY, "VK_ERROR_OUT_OF_HOST_MEMORY: A host memory allocation has failed" },
            { VkResult::VK_ERROR_OUT_OF_DEVICE_MEMORY, "VK_ERROR_OUT_OF_DEVICE_MEMORY: A device memory allocation has failed" },
            { VkResult::VK_ERROR_INITIALIZATION_FAILED, "VK_ERROR_INITIALIZATION_FAILED: Initialization of an object could not be completed for implementation-specific reasons" },
            { VkResult::VK_ERROR_DEVICE_LOST, "VK_ERROR_DEVICE_LOST: The logical or physical device has been lost" },
            { VkResult::VK_ERROR_MEMORY_MAP_FAILED, "VK_ERROR_MEMORY_MAP_FAILED: Mapping of a memory object has failed" },
            { VkResult::VK_ERROR_LAYER_NOT_PRESENT, "VK_ERROR_LAYER_NOT_PRESENT: A requested layer is not present or could not be loaded" },
            { VkResult::VK_ERROR_EXTENSION_NOT_PRESENT, "VK_ERROR_EXTENSION_NOT_PRESENT: A requested extension is not supported" },
            { VkResult::VK_ERROR_FEATURE_NOT_PRESENT, "VK_ERROR_FEATURE_NOT_PRESENT: A requested feature is not supported" },
            { VkResult::VK_ERROR_INCOMPATIBLE_DRIVER, "VK_ERROR_INCOMPATIBLE_DRIVER: The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation=specific reasons" },
            { VkResult::VK_ERROR_TOO_MANY_OBJECTS, "VK_ERROR_TOO_MANY_OBJECTS: Too many objects of the type has already been created" },
            { VkResult::VK_ERROR_FORMAT_NOT_SUPPORTED, "VK_ERROR_FORMAT_NOT_SUPPORTED: A requested format is not supported on this device" },
            { VkResult::VK_ERROR_SURFACE_LOST_KHR , "VK_ERROR_SURFACE_LOST_KHR: A surface is no longer available" },
            { VkResult::VK_ERROR_NATIVE_WINDOW_IN_USE_KHR, "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: The requested window is already connected to a VkSurfaceKHR, or some other non-Vulkan API" },
            { VkResult::VK_ERROR_OUT_OF_DATE_KHR, "VK_ERROR_OUT_OF_DATE_KHR: A surface has changed in such a way that it is no longer compatible with the swapchain, and further presentation requests using the swapchaing will fail" },
            { VkResult::VK_ERROR_INCOMPATIBLE_DISPLAY_KHR, "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: The display used by a swapchain does not use the same presentable image layout, or is incompatible in a way that prevents sharing an image" }
        };
    }
}


VulkanException::~VulkanException()
{
}

const std::string VulkanException::GetStatus() const
{
    std::string statusString;
    auto iter = m_statuses.find(m_status);
    if (iter != m_statuses.end()) {
        statusString = iter->second;
    }
    else {
        std::stringstream ss;
        ss << "Invalid status: " << m_status;
        statusString = ss.str();
    }
    return statusString;

}

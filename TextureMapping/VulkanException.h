#pragma once
#include <stdexcept>
#include <map>
#include <string>
#include <vulkan/vulkan.h>
class VulkanException :
    public std::runtime_error
{
public:
    VulkanException(const VkResult status, const std::string& msg);
    virtual ~VulkanException();
    const std::string GetStatus() const;

private:
    const VkResult m_status;
    static std::map<VkResult, std::string> m_statuses;
};


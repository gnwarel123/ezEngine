#pragma once

#include <vulkan/vulkan.h>

class ezGALDeviceVulkan;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEBUG)
#  define EZ_DISPATCH_CONTEXT_MEMBER_NAME(Name) m_p##Name
#else
#  define EZ_DISPATCH_CONTEXT_MEMBER_NAME(Name) Name
#endif

// A vulkan hpp compatible dispatch context.
class ezVulkanDispatchContext
{
public:
  void Init(ezGALDeviceVulkan& device);

  ezUInt32 getVkHeaderVersion() const { return VK_HEADER_VERSION; }

  PFN_vkGetMemoryFdKHR EZ_DISPATCH_CONTEXT_MEMBER_NAME(vkGetMemoryFdKHR) = nullptr;
  PFN_vkGetSemaphoreFdKHR EZ_DISPATCH_CONTEXT_MEMBER_NAME(vkGetSemaphoreFdKHR) = nullptr;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEBUG)
  VkResult vkGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) const;

  VkResult vkGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) const;
#endif

private:
  ezGALDeviceVulkan* m_pDevice = nullptr;
};
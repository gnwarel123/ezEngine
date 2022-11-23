#include <RendererVulkan/RendererVulkanPCH.h>

#include <RendererVulkan/Device/DeviceVulkan.h>
#include <RendererVulkan/Device/DispatchContext.h>

void ezVulkanDispatchContext::Init(ezGALDeviceVulkan& device)
{
  m_pDevice = &device;

  VkDevice nativeDevice = (VkDevice)device.GetVulkanDevice();
  const ezGALDeviceVulkan::Extensions& extensions = device.GetExtensions();

  if (extensions.m_bExternalMemoryFd)
  {
    EZ_DISPATCH_CONTEXT_MEMBER_NAME(vkGetMemoryFdKHR) = (PFN_vkGetMemoryFdKHR)vkGetDeviceProcAddr(nativeDevice, "vkGetMemoryFdKHR");
  }

  if (extensions.m_bExternalSemaphoreFd)
  {
    EZ_DISPATCH_CONTEXT_MEMBER_NAME(vkGetSemaphoreFdKHR) = (PFN_vkGetSemaphoreFdKHR)vkGetDeviceProcAddr(nativeDevice, "vkGetSemaphoreFdKHR");
  }
}

#if EZ_ENABLED(EZ_COMPILE_FOR_DEBUG)
VkResult ezVulkanDispatchContext::vkGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) const
{
  EZ_ASSERT_DEBUG(m_pvkGetMemoryFdKHR != nullptr, "vkGetMemoryFdKHR not supported");
  return m_pvkGetMemoryFdKHR(device, pGetFdInfo, pFd);
}

VkResult ezVulkanDispatchContext::vkGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) const
{
  EZ_ASSERT_DEBUG(m_pvkGetSemaphoreFdKHR != nullptr, "vkGetSemaphoreFdKHR not supported");
  return m_pvkGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
}
#endif
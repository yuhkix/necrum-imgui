#include "hooks.h"
#include "vulkan_renderer.h"
#include "menu/menu.h"

#include <MinHook.h>
#include <vulkan/vulkan.h>

namespace renderer
{

namespace
{
typedef VkResult(VKAPI_PTR* tQueuePresentKHR)(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);
tQueuePresentKHR oQueuePresentKHR = nullptr;

VulkanRenderer renderer;
menu::Menu menu;
bool hooks_enabled = true;

// Placeholder for objects that would be captured via other hooks (e.g. vkCreateDevice)
VkInstance g_instance = VK_NULL_HANDLE;
VkPhysicalDevice g_pd = VK_NULL_HANDLE;
VkDevice g_device = VK_NULL_HANDLE;
VkRenderPass g_rp = VK_NULL_HANDLE;
uint32_t g_qf = 0;

VkResult VKAPI_PTR h_QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* p_present_info)
{
	if (!hooks_enabled)
		return oQueuePresentKHR(queue, p_present_info);

	// In a real-world scenario, we would initialize the renderer here if we have all objects.
	// This usually involves hooking vkCreateDevice, vkCreateSwapchainKHR, etc.

	/*
	if (!renderer.is_initialized() && g_instance && g_device)
	{
		renderer.init(g_instance, g_pd, g_device, g_qf, queue, g_rp);
	}

	if (renderer.is_initialized())
	{
		renderer.begin_frame();
		menu.render();
		// In Vulkan we need a command buffer to render; this is usually managed
		// by hooking vkAcquireNextImageKHR or having our own command pool.
		// renderer.end_frame(command_buffer);
	}
	*/

	return oQueuePresentKHR(queue, p_present_info);
}
} // namespace

bool Hooks::init()
{
	if (MH_Initialize() != MH_OK)
		return false;

	HMODULE h_vulkan = GetModuleHandleW(L"vulkan-1.dll");
	if (!h_vulkan)
		return false;

	auto p_present = (void*)GetProcAddress(h_vulkan, "vkQueuePresentKHR");
	if (!p_present)
		return false;

	if (MH_CreateHook(p_present, &h_QueuePresentKHR, (LPVOID*)&oQueuePresentKHR) != MH_OK)
		return false;

	if (MH_EnableHook(p_present) != MH_OK)
		return false;

	return true;
}

void Hooks::shutdown()
{
	hooks_enabled = false;
	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();
	renderer.shutdown();
}

} // namespace renderer

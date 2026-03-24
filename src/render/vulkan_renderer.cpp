#include "vulkan_renderer.h"
#include "menu/theme.h"
#include "pch.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_win32.h>

#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
#include <volk.h>
#endif

namespace renderer
{

bool VulkanRenderer::init(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device, uint32_t queue_family,
													VkQueue queue, VkRenderPass render_pass)
{
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
	if (volkInitialize() != VK_SUCCESS)
		return false;
	volkLoadInstance(instance);
	volkLoadDevice(device);
#endif

	if (initialized)
		return true;

	p_instance = instance;
	p_physical_device = physical_device;
	p_device = device;
	p_queue = queue;
	p_render_pass = render_pass;

	if (!create_descriptor_pool())
		return false;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.IniFilename = nullptr;

	theme::LoadFonts(io);

	// Note: In a real hook, HWND should be obtained from the window associated with Vulkan
	ImGui_ImplWin32_Init(GetForegroundWindow());

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance;
	init_info.PhysicalDevice = physical_device;
	init_info.Device = device;
	init_info.QueueFamily = queue_family;
	init_info.Queue = queue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = p_descriptor_pool;
	init_info.ApiVersion = VK_API_VERSION_1_0;
	init_info.PipelineInfoMain.Subpass = 0;
	init_info.MinImageCount = 2;
	init_info.ImageCount = 2;
	init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = nullptr;
	init_info.PipelineInfoMain.RenderPass = render_pass;

	if (!ImGui_ImplVulkan_Init(&init_info))
		return false;

	theme::Apply();

	initialized = true;
	return true;
}

bool VulkanRenderer::create_descriptor_pool()
{
	VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
																			 {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
																			 {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
																			 {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
																			 {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
																			 {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
																			 {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
																			 {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
																			 {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
																			 {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
																			 {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * 11;
	pool_info.poolSizeCount = 11;
	pool_info.pPoolSizes = pool_sizes;

	return vkCreateDescriptorPool(p_device, &pool_info, nullptr, &p_descriptor_pool) == VK_SUCCESS;
}

void VulkanRenderer::begin_frame()
{
	if (!initialized)
		return;

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void VulkanRenderer::end_frame(VkCommandBuffer command_buffer)
{
	if (!initialized)
		return;

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
}

void VulkanRenderer::shutdown()
{
	if (!initialized)
		return;

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	if (p_descriptor_pool)
	{
		vkDestroyDescriptorPool(p_device, p_descriptor_pool, nullptr);
		p_descriptor_pool = VK_NULL_HANDLE;
	}

	initialized = false;
}

} // namespace renderer

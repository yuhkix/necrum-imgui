#pragma once
#include "pch.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace renderer
{

class VulkanRenderer
{
public:
	bool init(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device, uint32_t queue_family,
						VkQueue queue, VkRenderPass render_pass);
	void begin_frame();
	void end_frame(VkCommandBuffer command_buffer);
	void shutdown();

	bool is_initialized() const { return initialized; }

private:
	VkInstance p_instance = VK_NULL_HANDLE;
	VkPhysicalDevice p_physical_device = VK_NULL_HANDLE;
	VkDevice p_device = VK_NULL_HANDLE;
	VkQueue p_queue = VK_NULL_HANDLE;
	VkDescriptorPool p_descriptor_pool = VK_NULL_HANDLE;
	VkRenderPass p_render_pass = VK_NULL_HANDLE;

	bool initialized = false;

	bool create_descriptor_pool();
};

} // namespace renderer

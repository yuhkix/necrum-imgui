#include "vulkan_renderer.h"
#include "../menu/theme.h"
#include "../core/web_image.h"

#include "../ext/imgui/imgui.h"
#include "../ext/imgui/backends/imgui_impl_vulkan.h"
#include "../ext/imgui/backends/imgui_impl_win32.h"

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
	m_queue_family = queue_family;
	p_queue = queue;
	p_render_pass = render_pass;

	// Create command pool for one-shot commands
	VkCommandPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	poolInfo.queueFamilyIndex = queue_family;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (vkCreateCommandPool(p_device, &poolInfo, nullptr, &p_command_pool) != VK_SUCCESS)
		return false;

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

	web_image::set_texture_create_callback(
			[this](unsigned char* pixels, int width, int height) -> ImTextureID
			{
				if (!this->p_device)
					return 0;

				VkDeviceSize imageSize = width * height * 4;

				// Create staging buffer
				VkBuffer stagingBuffer;
				VkDeviceMemory stagingBufferMemory;

				VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
				bufferInfo.size = imageSize;
				bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				if (vkCreateBuffer(p_device, &bufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS)
					return 0;

				VkMemoryRequirements memReqs;
				vkGetBufferMemoryRequirements(p_device, stagingBuffer, &memReqs);

				VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
				allocInfo.allocationSize = memReqs.size;
				allocInfo.memoryTypeIndex = find_memory_type(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
																																								 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

				if (vkAllocateMemory(p_device, &allocInfo, nullptr, &stagingBufferMemory) != VK_SUCCESS)
					return 0;
				vkBindBufferMemory(p_device, stagingBuffer, stagingBufferMemory, 0);

				void* data;
				vkMapMemory(p_device, stagingBufferMemory, 0, imageSize, 0, &data);
				memcpy(data, pixels, static_cast<size_t>(imageSize));
				vkUnmapMemory(p_device, stagingBufferMemory);

				// Create Image
				VkImage textureImage;
				VkDeviceMemory textureImageMemory;

				VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
				imageInfo.imageType = VK_IMAGE_TYPE_2D;
				imageInfo.extent.width = width;
				imageInfo.extent.height = height;
				imageInfo.extent.depth = 1;
				imageInfo.mipLevels = 1;
				imageInfo.arrayLayers = 1;
				imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
				imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
				imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

				if (vkCreateImage(p_device, &imageInfo, nullptr, &textureImage) != VK_SUCCESS)
					return 0;

				vkGetImageMemoryRequirements(p_device, textureImage, &memReqs);
				allocInfo.allocationSize = memReqs.size;
				allocInfo.memoryTypeIndex = find_memory_type(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

				if (vkAllocateMemory(p_device, &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS)
					return 0;
				vkBindImageMemory(p_device, textureImage, textureImageMemory, 0);

				// Copy Buffer to Image
				execute_one_shot(
						[&](VkCommandBuffer cmd)
						{
							VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
							barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
							barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
							barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
							barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
							barrier.image = textureImage;
							barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
							barrier.subresourceRange.levelCount = 1;
							barrier.subresourceRange.layerCount = 1;
							barrier.srcAccessMask = 0;
							barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

							vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
																	 nullptr, 0, nullptr, 1, &barrier);

							VkBufferImageCopy region = {};
							region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
							region.imageSubresource.layerCount = 1;
							region.imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};

							vkCmdCopyBufferToImage(cmd, stagingBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
																		 &region);

							barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
							barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
							barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
							barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

							vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
																	 nullptr, 0, nullptr, 1, &barrier);
						});

				vkDestroyBuffer(p_device, stagingBuffer, nullptr);
				vkFreeMemory(p_device, stagingBufferMemory, nullptr);

				// Create View
				VkImageView textureImageView;
				VkImageViewCreateInfo viewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
				viewInfo.image = textureImage;
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
				viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				viewInfo.subresourceRange.levelCount = 1;
				viewInfo.subresourceRange.layerCount = 1;

				if (vkCreateImageView(p_device, &viewInfo, nullptr, &textureImageView) != VK_SUCCESS)
					return 0;

				// Create Sampler
				VkSampler textureSampler;
				VkSamplerCreateInfo samplerInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
				samplerInfo.magFilter = VK_FILTER_LINEAR;
				samplerInfo.minFilter = VK_FILTER_LINEAR;
				samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				samplerInfo.anisotropyEnable = VK_FALSE;
				samplerInfo.maxAnisotropy = 1.0f;
				samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
				samplerInfo.unnormalizedCoordinates = VK_FALSE;
				samplerInfo.compareEnable = VK_FALSE;
				samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
				samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

				if (vkCreateSampler(p_device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
					return 0;

				return (ImTextureID)ImGui_ImplVulkan_AddTexture(textureSampler, textureImageView,
																												VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			});

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

	if (p_command_pool)
	{
		vkDestroyCommandPool(p_device, p_command_pool, nullptr);
		p_command_pool = VK_NULL_HANDLE;
	}

	initialized = false;
}

uint32_t VulkanRenderer::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(p_physical_device, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((type_filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	return 0;
}

void VulkanRenderer::execute_one_shot(const std::function<void(VkCommandBuffer)>& callback)
{
	VkCommandBufferAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = p_command_pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(p_device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	callback(commandBuffer);
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(p_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(p_queue);

	vkFreeCommandBuffers(p_device, p_command_pool, 1, &commandBuffer);
}

} // namespace renderer

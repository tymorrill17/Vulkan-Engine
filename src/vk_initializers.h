// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>

namespace vkinit {
	VkRenderingAttachmentInfoKHR attachment_info(VkImageView view, VkClearValue* clear, VkImageLayout layout);
	VkPipelineColorBlendAttachmentState color_blend_attachment_state();
	VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags = 0);
	VkCommandBufferSubmitInfo command_buffer_submit_info(VkCommandBuffer cmd);
	VkCommandPoolCreateInfo command_pool_create_info(uint32_t queue_family_index, VkCommandPoolCreateFlags flags = 0);
	VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);
	VkDescriptorSetLayoutBinding descriptorset_layout_binding(VkDescriptorType type, VkShaderStageFlags flags, uint32_t binding);
	VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags = 0);
	VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent);
	VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspect_mask);
	VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags);
	VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info(VkPrimitiveTopology topology);
	VkPipelineMultisampleStateCreateInfo multisampling_state_create_info();
	VkPipelineLayoutCreateInfo pipeline_layout_create_info();
	VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shaderModule);
	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(VkPolygonMode polygon_mode);
	VkRenderingInfoKHR rendering_info(VkExtent2D extent, uint32_t color_attachment_count, VkRenderingAttachmentInfo* color_attachment, VkRenderingAttachmentInfo* depth_attachment);
	VkSamplerCreateInfo sampler_create_info(VkFilter filters, VkSamplerAddressMode sampler_address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
	VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags = 0);
	VkSemaphoreSubmitInfo semaphore_submit_info(VkPipelineStageFlags2 stage_mask, VkSemaphore semaphore);
	VkSubmitInfo2 submit_info(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signal_semaphore_info, VkSemaphoreSubmitInfo* wait_semaphore_info);
	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info();
	VkWriteDescriptorSet write_descriptor_image(VkDescriptorType type, VkDescriptorSet dst_set, VkDescriptorImageInfo* image_info, uint32_t binding);
	VkWriteDescriptorSet write_descriptorset_buffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding);
	// VkFramebufferCreateInfo frame_buffer_create_info(VkRenderPass renderpass, VkExtent2D extent, uint32_t attach_count = 1, uint32_t layer_count = 1);
	// VkRenderPassBeginInfo renderpass_begin_info(VkRenderPass renderpass, VkExtent2D extent, VkFramebuffer framebuffer);
}


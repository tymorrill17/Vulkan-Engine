﻿// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>

namespace vkinit {
	VkCommandPoolCreateInfo command_pool_create_info(uint32_t queue_family_index, VkCommandPoolCreateFlags flags = 0);
	VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	VkFramebufferCreateInfo frame_buffer_create_info(VkRenderPass renderpass, VkExtent2D extent, uint32_t count = 1, uint32_t layer_count = 1);
	VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shaderModule);
	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info();
	VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info(VkPrimitiveTopology topology);
	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(VkPolygonMode polygon_mode);
	VkPipelineMultisampleStateCreateInfo multisampling_state_create_info();
	VkPipelineColorBlendAttachmentState color_blend_attachment_state();
	VkPipelineLayoutCreateInfo pipeline_layout_create_info();
	VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);
	VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags = 0);
	VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags = 0);
	VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent);
	VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags);
	VkRenderPassBeginInfo renderpass_begin_info(VkRenderPass renderpass, VkExtent2D extent, VkFramebuffer framebuffer);
	VkDescriptorSetLayoutBinding descriptorset_layout_binding(VkDescriptorType type, VkShaderStageFlags flags, uint32_t binding);
	VkWriteDescriptorSet write_descriptorset_buffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding);
}


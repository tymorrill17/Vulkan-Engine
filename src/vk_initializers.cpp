﻿#include <vk_initializers.h>

VkCommandPoolCreateInfo vkinit::command_pool_create_info(uint32_t queue_family_index, VkCommandPoolCreateFlags flags) {
    VkCommandPoolCreateInfo info={};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = nullptr;
    info.queueFamilyIndex = queue_family_index;
    info.flags = flags;
    return info;
}

VkCommandBufferAllocateInfo vkinit::command_buffer_allocate_info(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level) {
    VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = nullptr;
	info.commandPool = pool;
	info.commandBufferCount = count;
	info.level = level;
    return info;
}

// VkFramebufferCreateInfo vkinit::frame_buffer_create_info(VkRenderPass renderpass, VkExtent2D extent, uint32_t attach_count, uint32_t layer_count) {
//     VkFramebufferCreateInfo info={};
//     info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//     info.pNext = nullptr;
//     info.renderPass = renderpass;
//     info.attachmentCount = attach_count;
//     info.width = extent.width;
//     info.height = extent.height;
//     info.layers = layer_count;
//     return info;
// }
// Contains info for a single shader stage
VkPipelineShaderStageCreateInfo vkinit::pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shaderModule) {
    VkPipelineShaderStageCreateInfo info={};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.pNext = nullptr;
    info.stage = stage;
    info.module = shaderModule;
    info.pName = "main"; // entry point of the shader
    return info;
}
// Contains info for vertex buffers and vertex formats
VkPipelineVertexInputStateCreateInfo vkinit::vertex_input_state_create_info() {
    VkPipelineVertexInputStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	info.pNext = nullptr;
	// None yet
	info.vertexBindingDescriptionCount = 0;
	info.vertexAttributeDescriptionCount = 0;
	return info;
}
// Contains info about which kind of topology will be drawn: triangles, points, lines, triangle-lists...
VkPipelineInputAssemblyStateCreateInfo vkinit::input_assembly_state_create_info(VkPrimitiveTopology topology) {
    VkPipelineInputAssemblyStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	info.pNext = nullptr;
	info.topology = topology;
	info.primitiveRestartEnable = VK_FALSE; // not going to use for now
	return info;
}
// Config for the fixed-function rasterization
VkPipelineRasterizationStateCreateInfo vkinit::rasterization_state_create_info(VkPolygonMode polygon_mode) {
    VkPipelineRasterizationStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.depthClampEnable = VK_FALSE;
    info.rasterizerDiscardEnable = VK_FALSE; // Discards all primitives before the rasterization stage if enabled
    info.polygonMode = polygon_mode;
    info.lineWidth = 1.0f;
    info.cullMode = VK_CULL_MODE_NONE; // Backface cull -> remove unseen primitives
    info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    info.depthBiasEnable = VK_FALSE; // No depth bias
    info.depthBiasConstantFactor = 0.0f;
    info.depthBiasClamp = 0.0f;
    info.depthBiasSlopeFactor = 0.0f;
    return info;
}
// If you want to use multisampling, the renderpass must also support it
VkPipelineMultisampleStateCreateInfo vkinit::multisampling_state_create_info() {
    VkPipelineMultisampleStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.sampleShadingEnable = VK_FALSE; // enable if using MSAA
    info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // Default to no multisampling
    info.minSampleShading = 1.0f;
    info.pSampleMask = nullptr;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable = VK_FALSE;
    return info;
}
// Controls how pipeline blends into a given attachment
VkPipelineColorBlendAttachmentState vkinit::color_blend_attachment_state() {
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE; // We are not blending here, just overriding
    return colorBlendAttachment;
}

VkPipelineLayoutCreateInfo vkinit::pipeline_layout_create_info() {
    VkPipelineLayoutCreateInfo info={};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.setLayoutCount = 0;
    info.pSetLayouts = nullptr;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges = nullptr;
    return info;
}

VkFenceCreateInfo vkinit::fence_create_info(VkFenceCreateFlags flags) {
    VkFenceCreateInfo fence_info={};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.pNext = nullptr;
	fence_info.flags = flags; // Create the fence signaled
    return fence_info;
}

VkSemaphoreCreateInfo vkinit::semaphore_create_info(VkSemaphoreCreateFlags flags) {
    VkSemaphoreCreateInfo semaphore_info;
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_info.pNext = nullptr;
	semaphore_info.flags = flags;
    return semaphore_info;
}

VkImageCreateInfo vkinit::image_create_info(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent) {
    VkImageCreateInfo info={};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;
    info.imageType = VK_IMAGE_TYPE_2D; // How many dimensions the image has
    info.format = format; // Format of the image data
    info.extent = extent; // Size of image in pixels
    info.mipLevels = 1; // How many mipmap levels the image has
    info.arrayLayers = 1; // For layered images
    info.samples = VK_SAMPLE_COUNT_1_BIT; // Only applicable for target images
    info.tiling = VK_IMAGE_TILING_OPTIMAL; // Tiling tells GPU how to store the image
    info.usage = usage_flags;
    return info;
}

VkImageViewCreateInfo vkinit::imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags) {
    VkImageViewCreateInfo info={};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.image = image;
    info.format = format;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.aspectMask = aspect_flags;
    return info;
}

VkPipelineDepthStencilStateCreateInfo vkinit::depth_stencil_create_info(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp) {
    VkPipelineDepthStencilStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.depthTestEnable = bDepthTest ? VK_TRUE : VK_FALSE; // Should we do any z-culling at all?
    info.depthWriteEnable = bDepthWrite ? VK_TRUE : VK_FALSE; // Allows depth to be written
    info.depthCompareOp = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS; // Holds depth-testing function
    info.depthBoundsTestEnable = VK_FALSE;
    info.minDepthBounds = 0.0f; // Optional: allows capping the depth test
    info.maxDepthBounds = 1.0f; // Optional
    info.stencilTestEnable = VK_FALSE;
    return info;
}

// VkRenderPassBeginInfo vkinit::renderpass_begin_info(VkRenderPass renderpass, VkExtent2D extent, VkFramebuffer framebuffer) {
//     VkRenderPassBeginInfo info;
//     info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
// 	info.pNext = nullptr;
// 	info.renderPass = renderpass;
// 	info.renderArea.extent = extent;
// 	info.renderArea.offset.x = 0;
// 	info.renderArea.offset.y = 0;
// 	info.framebuffer = framebuffer;
//     return info;
// }

VkDescriptorSetLayoutBinding vkinit::descriptorset_layout_binding(VkDescriptorType type, VkShaderStageFlags stage_flags, uint32_t binding) {
    VkDescriptorSetLayoutBinding layout_binding={};
	layout_binding.binding = binding;
	layout_binding.descriptorCount = 1;
	layout_binding.descriptorType = type; // What type of binding?
    layout_binding.pImmutableSamplers = nullptr;
	layout_binding.stageFlags = stage_flags; // Which stage is the buffer being accesed?
    return layout_binding;
}

VkWriteDescriptorSet vkinit::write_descriptorset_buffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding) {
    VkWriteDescriptorSet write_buf={};
    write_buf.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_buf.pNext = nullptr;
    write_buf.dstBinding = binding; // Writing to binding 0
    write_buf.dstSet = dstSet; // of the global descriptor
    write_buf.descriptorCount = 1;
    write_buf.descriptorType = type;
    write_buf.pBufferInfo = bufferInfo;
    return write_buf;
}

VkCommandBufferBeginInfo vkinit::command_buffer_begin_info(VkCommandBufferUsageFlags flags) {
    VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = nullptr;
	info.pInheritanceInfo = nullptr;
	info.flags = flags;
	return info;
}

VkSubmitInfo2 vkinit::submit_info(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signal_semaphore_info, VkSemaphoreSubmitInfo* wait_semaphore_info) {
    VkSubmitInfo2 info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	info.pNext = nullptr;
	info.waitSemaphoreInfoCount = wait_semaphore_info == nullptr ? 0 : 1;
	info.pWaitSemaphoreInfos = wait_semaphore_info;
	info.signalSemaphoreInfoCount = signal_semaphore_info == nullptr ? 0 : 1;
	info.pSignalSemaphoreInfos = signal_semaphore_info;
	info.commandBufferInfoCount = 1;
	info.pCommandBufferInfos = cmd;
	return info;
}

VkSemaphoreSubmitInfo vkinit::semaphore_submit_info(VkPipelineStageFlags2 stage_mask, VkSemaphore semaphore) {
    VkSemaphoreSubmitInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    info.pNext = nullptr;
    info.semaphore = semaphore;
    info.stageMask = stage_mask;
    info.deviceIndex = 0;
    info.value = 1;
    return info;
}

VkCommandBufferSubmitInfo vkinit::command_buffer_submit_info(VkCommandBuffer cmd) {
    VkCommandBufferSubmitInfo info={
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .pNext = nullptr,
        .commandBuffer = cmd,
        .deviceMask = 0,
    };
    return info;
}

VkSamplerCreateInfo vkinit::sampler_create_info(VkFilter filters, VkSamplerAddressMode sampler_address_mode) {
    VkSamplerCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.pNext = nullptr;
    info.magFilter = filters;
    info.minFilter = filters;
    info.addressModeU = sampler_address_mode;
    info.addressModeV = sampler_address_mode;
    info.addressModeW = sampler_address_mode;
    return info;
}

VkWriteDescriptorSet vkinit::write_descriptor_image(VkDescriptorType type, VkDescriptorSet dst_set, VkDescriptorImageInfo* image_info, uint32_t binding) {
    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstBinding = binding;
    write.dstSet = dst_set;
    write.descriptorCount = 1;
    write.descriptorType = type;
    write.pImageInfo = image_info;
    return write;
}

VkImageSubresourceRange vkinit::image_subresource_range(VkImageAspectFlags aspect_mask) {
    VkImageSubresourceRange subrange={};
    subrange.aspectMask = aspect_mask;
    subrange.baseMipLevel = 0;
    subrange.levelCount = VK_REMAINING_MIP_LEVELS;
    subrange.baseArrayLayer = 0;
    subrange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    return subrange;
}

VkRenderingInfoKHR vkinit::rendering_info(VkExtent2D extent, uint32_t color_attach_count, uint32_t layer_count) {
    VkRenderingInfoKHR info{
        .sType=VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
        .pNext=nullptr,
        .layerCount=layer_count,
        .colorAttachmentCount=color_attach_count,
    };
    info.renderArea.extent=extent;
    info.renderArea.offset.x=0;
    info.renderArea.offset.y=0;
    return info;
}
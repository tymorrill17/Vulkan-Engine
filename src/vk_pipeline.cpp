#include "vk_engine.h"
#include "vk_pipeline.h"

#include <fstream>

VkPipeline PipelineBuilder::build_pipeline(VkDevice device, VulkanEngine* engine) { //VkRenderPass pass) {
    // Make viewport state from stored viewport and scissor.
    VkPipelineViewportStateCreateInfo viewport_state={};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.pNext = nullptr;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    // Setup dummy color blending. Not using transparent objects yet. 
    VkPipelineColorBlendStateCreateInfo color_blending={};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.pNext = nullptr;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;

    // Build the actual pipeline.
    // All of the initializers of pipeline objects come into play
    VkGraphicsPipelineCreateInfo pipeline_info={};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.pNext = &rendering_info;
    pipeline_info.stageCount = shader_stages.size();
    pipeline_info.pStages = shader_stages.data(); // Shader stages contain shader programs
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.layout = pipeline_layout;
    pipeline_info.renderPass = nullptr;
    
    VkDynamicState state[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamic_info.dynamicStateCount = 2;
    dynamic_info.pDynamicStates = &state[0];

    pipeline_info.pDynamicState = &dynamic_info;

    VkPipeline new_pipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &new_pipeline) != VK_SUCCESS) {
        std::cout << "Failed to create pipeline" << std::endl;
        return VK_NULL_HANDLE;
    } else {
        return new_pipeline;
    }

    VkPipeline pipeline;
    return pipeline;
}

// Loads a shader module from a SPIR-V file. Returns false if there are errors.
bool vkutil::load_shader_module(const char* filepath, VkDevice device, VkShaderModule* out_shader_module) {
	// std::ios::ate -> puts stream curser at end
	// std::ios::binary -> opens file in binary mode
	std::ifstream file(filepath, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "ERROR: Shader file does not exist!" << std::endl;
		return false;
	}
	// Since cursor is at the end, use it to find the size of the file, then copy the entire shader into a vector of uint32_t
	size_t filesize = (size_t)file.tellg(); // tellg() returns the position of the cursor
	std::vector<uint32_t> buffer(filesize / sizeof(uint32_t));
	file.seekg(0); // move cursor to beginning
	file.read((char*)buffer.data(), filesize); // load entire file into the buffer
	file.close();
	// Now we have the entire shader in the buffer and can load it to Vulkan
	VkShaderModuleCreateInfo createinfo;
	createinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createinfo.pNext = nullptr;
	createinfo.codeSize = buffer.size() * sizeof(uint32_t); // codeSize has to be in bytes
	createinfo.pCode = buffer.data();
	createinfo.flags=0;

	VkShaderModule shader_module;
	if (vkCreateShaderModule(device, &createinfo, nullptr, &shader_module) != VK_SUCCESS) {
		std::cerr << "ERROR: Something went wrong within vkCreateShaderModule()!" << std::endl;
		return false;
	}
	*out_shader_module = shader_module;
	return true;
}
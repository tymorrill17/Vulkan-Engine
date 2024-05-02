#include "vk_engine.h"
#include "vk_pipeline.h"
#include "vk_initializers.h"

#include <fstream>

VkPipeline PipelineBuilder::build_pipeline(VkDevice device) { //VkRenderPass pass) {
    // Make viewport state from stored viewport and scissor. Could add support for multiple viewports in the future
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

    // Inititalize to zero since it won't be used
    // TODO: CHANGE THIS BACK LATER WHEN WE CHANGE THE VERTEX BUFFER SYSTEM
    // VkPipelineVertexInputStateCreateInfo vertex_input_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    // Build the actual pipeline.
    // All of the initializers of pipeline objects come into play
    VkGraphicsPipelineCreateInfo pipeline_info={.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
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
// Clears all values from the PipelineBuilder to be able to safely reuse the object.
void PipelineBuilder::clear() {
    input_assembly = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    rasterizer = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    color_blend_attachment = {};
    multisampling = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    pipeline_layout = {};
    depth_stencil = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    rendering_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    shader_stages.clear();
}
// Loads a shader module from a SPIR-V file. Returns false if there are errors.
bool vkutil::load_shader_module(const char* filepath, VkDevice device, VkShaderModule* out_shader_module) {
	// std::ios::ate -> puts stream curser at end
	// std::ios::binary -> opens file in binary mode
	std::ifstream file(filepath, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "ERROR: Shader file does not exist: " << filepath << std::endl;
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
		std::cerr << "ERROR: vkCreateShaderModule() failed while creating " << filepath << std::endl;
		return false;
	}
	*out_shader_module = shader_module;
    std::cout << "Shader successfully loaded: " << filepath << std::endl;
	return true;
}
void PipelineBuilder::set_shaders(VkShaderModule vertex_shader, VkShaderModule fragment_shader) {
    shader_stages.clear();
    shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader));
    shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader));
}
void PipelineBuilder::set_input_topology(VkPrimitiveTopology topology) {
    input_assembly.topology = topology;
    input_assembly.primitiveRestartEnable = VK_FALSE; // Not using for now
}
// These next few pertain to the rasterizer since there are several features we might want to change
void PipelineBuilder::set_polygon_mode(VkPolygonMode mode) {
    rasterizer.polygonMode = mode;
    rasterizer.lineWidth = 1.0f; // Setting this to a default of 1.0
}
void PipelineBuilder::set_cull_mode(VkCullModeFlags cull_mode, VkFrontFace front_face) {
    rasterizer.cullMode = cull_mode;
    rasterizer.frontFace = front_face;
}
void PipelineBuilder::set_multisampling_none() {
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // no multisampling: 1 sample per pixel
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;
}
void PipelineBuilder::disable_blending() {
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
}
void PipelineBuilder::set_color_attachment_format(VkFormat format) {
    color_attachment_format = format;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachmentFormats = &color_attachment_format;
}
void PipelineBuilder::set_depth_format(VkFormat format) {
    rendering_info.depthAttachmentFormat = format;
}
void PipelineBuilder::disable_depth_test() {
    depth_stencil.depthTestEnable = VK_FALSE;
    depth_stencil.depthWriteEnable = VK_FALSE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_NEVER;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.front = {};
    depth_stencil.back = {};
    depth_stencil.minDepthBounds = 0.0f;
    depth_stencil.maxDepthBounds = 1.0f;
}
void PipelineBuilder::enable_depth_test(VkCompareOp compareOp) {
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = compareOp;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.front = {};
    depth_stencil.back = {};
    depth_stencil.minDepthBounds = 0.0f;
    depth_stencil.maxDepthBounds = 1.0f;
}
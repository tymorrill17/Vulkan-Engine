
#include "vk_engine.h"


class PipelineBuilder {
public:
	std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
	
	// TODO: DELETE THIS LATER
	VkPipelineVertexInputStateCreateInfo vertex_input_info; // Removing this because of a better vertex indexing system

	VkPipelineInputAssemblyStateCreateInfo input_assembly;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineColorBlendAttachmentState color_blend_attachment;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineDepthStencilStateCreateInfo depth_stencil;
	VkPipelineLayout pipeline_layout;
	VkPipelineRenderingCreateInfo rendering_info;
	VkFormat color_attachment_format;

	PipelineBuilder() {
		clear();
	}

	void clear();
	VkPipeline build_pipeline(VkDevice device);
	void set_shaders(VkShaderModule vertex_shader, VkShaderModule fragment_shader);
	void set_input_topology(VkPrimitiveTopology topology);
	void set_polygon_mode(VkPolygonMode mode);
	void set_cull_mode(VkCullModeFlags cull_mode, VkFrontFace front_face);
	void set_multisampling_none();
	void disable_blending();
	void set_color_attachment_format(VkFormat format);
	void set_depth_format(VkFormat format);
	void disable_depth_test();
	void enable_depth_test(VkCompareOp compareOp);
};

namespace vkutil {
	bool load_shader_module(const char* filepath, VkDevice device, VkShaderModule* out_shader_module);
}

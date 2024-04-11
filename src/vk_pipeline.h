
#include "vk_engine.h"


class PipelineBuilder {
public:
	std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
	VkPipelineVertexInputStateCreateInfo vertex_input_info;
	VkPipelineInputAssemblyStateCreateInfo input_assembly;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineColorBlendAttachmentState color_blend_attachment;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineDepthStencilStateCreateInfo depth_stencil;
	VkPipelineLayout pipeline_layout;
	VkPipelineRenderingCreateInfo rendering_info;

	VkPipeline build_pipeline(VkDevice device, VulkanEngine* engine);
};

namespace vkutil {
	bool load_shader_module(const char* filepath, VkDevice device, VkShaderModule* out_shader_module);
}

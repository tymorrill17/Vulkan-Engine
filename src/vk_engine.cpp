
#include "vk_engine.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "vk_types.h"
#include "vk_initializers.h"
#include "vk_pipeline.h"
#include "VkBootstrap.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <fstream>
#define VK_CHECK(x)														\
	do {                                            			        \
		VkResult err = x;												\
		if (err) {														\
			std::cout << "Detected Vulkan Error: " << err << std::endl;	\
			abort();													\
		}																\
	} while (0)															\

/* This is the main class that will handle all of the engine functionality. Most of the tutorial code will go here */
void VulkanEngine::init() {
	// Initializes SDL. SDL_INIT_VIDEO enables the SDL video subsystems (windows, events, etc)
	SDL_Init(SDL_INIT_VIDEO);
	// Window flags for use in SDL_CreateWindow
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
	window = SDL_CreateWindow(
		"Vulkan Engine",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		windowExtent.width,
		windowExtent.height,
		window_flags
	);

	init_vulkan();
	// Initialize core vulkan structures
	VmaAllocatorCreateInfo allocator_info={};
	allocator_info.physicalDevice = chosenGPU;
	allocator_info.device = device;
	allocator_info.instance = instance;
	vmaCreateAllocator(&allocator_info, &allocator);

	init_swapchain();
	init_commands();
	init_default_renderpass();
	init_framebuffers();
	init_sync();
	init_descriptors();
	init_pipelines();

	load_meshes();
	init_scene();
	//everything went fine
	isInitialized = true;
}
// Initialize vulkan structures
void VulkanEngine::init_vulkan() {
	// From vkboostrap: builds the vulkan instace
	vkb::InstanceBuilder instance_builder;
	// To configure the instance, call desired member functions before .build()
	auto instance_builder_return = instance_builder.set_app_name("Vulkan App")
		.request_validation_layers(true) // 
		.use_default_debug_messenger()
		.require_api_version(1,3,0)
		.build();
	if (!instance_builder_return) { // Verify instance was built correctly
		std::cerr << "Failed to create Vulkan instance. Error: " << instance_builder_return.error().message() << std::endl;
		abort();
	}
	// Returns the instance from the builder to an instance variable. vkb::Instance contains info needed at the instance level.
	// All instance info within vkb::Instance can be extracted.
	vkb::Instance vkb_instance = instance_builder_return.value();
	instance = vkb_instance.instance; // Store the instance
	debug_messenger = vkb_instance.debug_messenger; // Store debug messenger
	// Get the surface of the window created with SDL
	SDL_bool err = SDL_Vulkan_CreateSurface(window, instance, &surface);
	if (!err) {std::cout << "Error obtaining the surface from the SDL window!" << std::endl;}
	// Obtain and select physical devices
	vkb::PhysicalDeviceSelector phys_device_selector(vkb_instance); // constructs phys. device selector with a vkb instance
	auto phys_device_selector_return = phys_device_selector
		.set_surface(surface)
		.select();
	if (!phys_device_selector_return) { // Verify physical device was selected
		std::cerr << "Failed to select a physical device. Error: " << phys_device_selector_return.error().message() << std::endl;
		abort();
	}
	vkb::PhysicalDevice vkb_physical_device = phys_device_selector_return.value();
	// Build logical device
	vkb::DeviceBuilder device_builder(vkb_physical_device);
	VkPhysicalDeviceShaderDrawParameterFeatures shader_draw_parameter_features = {};
	shader_draw_parameter_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETER_FEATURES;
	shader_draw_parameter_features.pNext = nullptr;
	shader_draw_parameter_features.shaderDrawParameters = VK_TRUE;
	auto device_builder_return = device_builder.add_pNext(&shader_draw_parameter_features).build();
	if (!device_builder_return) { // Verify logical device was built
		std::cerr << "Failed to build logical device. Error: " << device_builder_return.error().message() << std::endl;
		abort();
	}
	vkb::Device vkb_device = device_builder_return.value();
	device = vkb_device.device; // Store the VkDevice
	chosenGPU = vkb_physical_device.physical_device; // Store the physical device
	// Get the graphics queue using vkbootstrap
	graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
	graphics_queue_family = vkb_device.get_queue_index(vkb::QueueType::graphics).value();
	// // Get physical device properties
	gpu_properties = vkb_device.physical_device.properties;
	// Print out the minimum buffer alignment offset value: RTX 3080 offset is 64 bytes
	std::cout << "The GPU has a minimum buffer alignment of " << gpu_properties.limits.minUniformBufferOffsetAlignment << std::endl;
}
// Initialize swapchain structures
void VulkanEngine::init_swapchain() {
	vkb::SwapchainBuilder swapchain_builder(chosenGPU,device,surface);
	auto swapchain_builder_return = swapchain_builder
		.use_default_format_selection()
		.set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
		.set_desired_extent(windowExtent.width,windowExtent.height)
		.build();
	if (!swapchain_builder_return) { // Verify swap chain was built
		std::cerr << "Failed to build swapchain. Error: " << swapchain_builder_return.error().message() << std::endl;
		abort();
	}
	vkb::Swapchain vkb_swapchain = swapchain_builder_return.value();
	// Store swapchain and related values
	swapchain = vkb_swapchain.swapchain;
	swapchain_images = vkb_swapchain.get_images().value();
	swapchain_image_views = vkb_swapchain.get_image_views().value();
	swapchain_image_format = vkb_swapchain.image_format;
	// std::cout << "Number of swapchain images: " << swapchain_images.size() << std::endl;
	// Depth image will match window extent
	VkExtent3D depthimage_extent = {
		windowExtent.width,
		windowExtent.height,
		1
	};
	depth_format = VK_FORMAT_D32_SFLOAT; // Hardcoding depth format to 32 bit float
	VkImageCreateInfo dimage_info = vkinit::image_create_info(depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthimage_extent);
	// We want to allocate the depth image from local GPU memory
	VmaAllocationCreateInfo dimage_allocationinfo = {};
	dimage_allocationinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	dimage_allocationinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	// Allocate and create the image
	vmaCreateImage(allocator, &dimage_info, &dimage_allocationinfo, &depth_image.image, &depth_image.allocation, nullptr);
	// Build the depth image view
	VkImageViewCreateInfo dimageview_info=vkinit::imageview_create_info(depth_format, depth_image.image, VK_IMAGE_ASPECT_DEPTH_BIT);
	VK_CHECK(vkCreateImageView(device, &dimageview_info, nullptr, &depth_image_view));

	main_deletion_queue.push_function([=](){
		vkDestroyImageView(device, depth_image_view, nullptr);
		vmaDestroyImage(allocator, depth_image.image, depth_image.allocation);
		vkDestroySwapchainKHR(device, swapchain, nullptr);
	});
}
// Initialize command pool and command buffers
void VulkanEngine::init_commands() {
	// No longer using vkbootstrap here
	// Fill command buffer allocateInfo struct
	VkCommandPoolCreateInfo cmd_pool_createinfo = vkinit::command_pool_create_info(graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	for (int i = 0; i < FRAME_OVERLAP; i++) {
		VK_CHECK(vkCreateCommandPool(device, &cmd_pool_createinfo, nullptr, &frames[i].command_pool));
		VkCommandBufferAllocateInfo cmd_alloc_info = vkinit::command_buffer_allocate_info(frames[i].command_pool, 1);
		VK_CHECK(vkAllocateCommandBuffers(device, &cmd_alloc_info, &frames[i].command_buffer));
		// Push deletion function to the deletion queue
		main_deletion_queue.push_function([=](){vkDestroyCommandPool(device, frames[i].command_pool, nullptr);});
	}
	// GPU memory uplead command structures
	VkCommandPoolCreateInfo upload_command_pool_info = vkinit::command_pool_create_info(graphics_queue_family);
	VK_CHECK(vkCreateCommandPool(device, &upload_command_pool_info, nullptr, &upload_context.command_pool));
	main_deletion_queue.push_function([=](){vkDestroyCommandPool(device, upload_context.command_pool, nullptr);});
	// Allocate the default command buffer for the instant commands
	VkCommandBufferAllocateInfo cmd_allocinfo = vkinit::command_buffer_allocate_info(upload_context.command_pool, 1);
	VK_CHECK(vkAllocateCommandBuffers(device, &cmd_allocinfo, &upload_context.command_buffer));
}
// Returns the FrameData of the current frame that is ready to be prepared by the CPU
FrameData& VulkanEngine::get_current_frame() {
	return frames[frameNumber % FRAME_OVERLAP];
}
// Initialize a default renderpass
void VulkanEngine::init_default_renderpass() {
	// Color attachment is the description of the image we will be writing with rendering commands
	VkAttachmentDescription color_attachment={};
	color_attachment.format = swapchain_image_format; // renderpass will have image format needed by the swapchain
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT; // MSAA sample count
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear when this attachment is loaded
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Store attachment when renderpass ends
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // Don't care about stencils
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Don't know or care about the starting layout of the attachment
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // After the renderpass ends, the image has to be in a layout ready to display

	// Now the main image target is defined, so create a subpass that will render to it.
	VkAttachmentReference color_attachment_ref={};
	color_attachment_ref.attachment = 0; // Attachment number will index into the pAttachments array in the renderpass
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Depth attachment creation
	VkAttachmentDescription depth_attachment={};
	depth_attachment.flags = 0;
	depth_attachment.format = depth_format;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = {};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Minimum 1 subpass required
	VkSubpassDescription subpass={};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	VkAttachmentDescription attachments[2] = {color_attachment, depth_attachment};

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	// TODO: Understand these dependencies more
	VkSubpassDependency depth_dependency = {};
	depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	depth_dependency.dstSubpass = 0;
	depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depth_dependency.srcAccessMask = 0;
	depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency dependencies[2] = {dependency, depth_dependency};

	// Now the attachments and subpass are done, we can create the main pass
	VkRenderPassCreateInfo render_pass_info={};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 2; // Connect color and depth attachments to render pass
	render_pass_info.pAttachments = &attachments[0];
	render_pass_info.dependencyCount = 2;
	render_pass_info.pDependencies = &dependencies[0];
	render_pass_info.subpassCount = 1; // Connect subpass
	render_pass_info.pSubpasses = &subpass;
	VK_CHECK(vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass));
	main_deletion_queue.push_function([=](){vkDestroyRenderPass(device, render_pass, nullptr);});
	std::cout << "Finished initializing renderpass" << std::endl;
}
// Initialize the framebuffers
void VulkanEngine::init_framebuffers() {
	VkFramebufferCreateInfo framebuffer_info = vkinit::frame_buffer_create_info(render_pass,windowExtent,1,1);
	const uint32_t swapchain_imagecount = swapchain_images.size(); // We need a framebuffer for each swapchain image
	framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);
	for (int i=0; i < swapchain_imagecount; i++) {
		VkImageView attachments[2];
		attachments[0] = swapchain_image_views[i];
		attachments[1] = depth_image_view;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.attachmentCount = 2;
		VK_CHECK(vkCreateFramebuffer(device, &framebuffer_info, nullptr, &framebuffers[i]));
		main_deletion_queue.push_function([=]() {
			vkDestroyFramebuffer(device, framebuffers[i], nullptr);
			vkDestroyImageView(device, swapchain_image_views[i], nullptr);
    	});
	}
}
void VulkanEngine::init_sync() {
	VkFenceCreateInfo fence_info = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semaphore_info = vkinit::semaphore_create_info();

	VkFenceCreateInfo upload_fence_info = vkinit::fence_create_info();
	VK_CHECK(vkCreateFence(device, &upload_fence_info, nullptr, &upload_context.upload_fence));
	main_deletion_queue.push_function([=](){
			vkDestroyFence(device, upload_context.upload_fence, nullptr);
		});

	for (int i = 0; i < FRAME_OVERLAP; i++) {
		VK_CHECK(vkCreateFence(device, &fence_info, nullptr, &frames[i].render_fence));
		// Both semaphores use the same create info
		VK_CHECK(vkCreateSemaphore(device, &semaphore_info, nullptr, &frames[i].render_semaphore));
		VK_CHECK(vkCreateSemaphore(device, &semaphore_info, nullptr, &frames[i].present_semaphore));
		main_deletion_queue.push_function([=](){
			vkDestroyFence(device, frames[i].render_fence, nullptr);
			vkDestroySemaphore(device, frames[i].render_semaphore, nullptr);
			vkDestroySemaphore(device, frames[i].present_semaphore, nullptr);
		});
	}
}
// Initialize rendering pipeline structures
void VulkanEngine::init_pipelines() {
	VkShaderModule triangleFragShader;
	if (!load_shader_module("../shaders/default_lit.frag.spv", &triangleFragShader)) {
		std::cout << "Error building the colored mesh shader module!" << std::endl;
	} else {
		std::cout << "Fragment shader module loaded!" << std::endl;
	}
	VkShaderModule meshVertShader;
	if (!load_shader_module("../shaders/tri_mesh.vert.spv", &meshVertShader))
	{
		std::cout << "Error when building the triangle vertex shader module" << std::endl;
	} else {
		std::cout << "Triangle mesh vertex shader successfully loaded" << std::endl;
	}

	// Build pipeline layout which controls inputs and outputs of the shader
	VkPipelineLayoutCreateInfo mesh_pipeline_layout_info = vkinit::pipeline_layout_create_info();

	// Set up push constants
	VkPushConstantRange push_constant;
	push_constant.offset = 0; // Starts at beginning
	push_constant.size = sizeof(MeshPushConstants); // How much memory does the push constant occupy?
	push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Will be used in the vertex shader
	// Tell the pipeline layout about the push constants
	mesh_pipeline_layout_info.pushConstantRangeCount = 1;
	mesh_pipeline_layout_info.pPushConstantRanges = &push_constant;
	// Tell the pipeline layout about the descriptor set layouts
	VkDescriptorSetLayout set_layouts[] = {global_set_layout, object_set_layout};
	mesh_pipeline_layout_info.setLayoutCount = 2;
	mesh_pipeline_layout_info.pSetLayouts = set_layouts;

	VK_CHECK(vkCreatePipelineLayout(device, &mesh_pipeline_layout_info, nullptr, &mesh_pipeline_layout));

	PipelineBuilder pipeline_builder;

	// Vertex input controls how to read vertices from vertex buffers
	pipeline_builder.vertex_input_info = vkinit::vertex_input_state_create_info();
	// Input assembly is the configuration for drawing triangle lists, strips, or points
	pipeline_builder.input_assembly = vkinit::input_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	// Build viewport and scissor from the swapchain extents
	pipeline_builder.viewport.x = 0.0f;
	pipeline_builder.viewport.y = 0.0f;
	pipeline_builder.viewport.height = windowExtent.height;
	pipeline_builder.viewport.width = windowExtent.width;
	pipeline_builder.viewport.minDepth = 0.0f;
	pipeline_builder.viewport.maxDepth = 1.0f;
	pipeline_builder.scissor.offset = {0,0};
	pipeline_builder.scissor.extent = windowExtent;
	// Configure the rasterizer to draw filled triangles
	pipeline_builder.rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);
	// Not using multisampling righ now
	pipeline_builder.multisampling = vkinit::multisampling_state_create_info();
	// A single blend attachment with no blending writing to RGBA
	pipeline_builder.color_blend_attachment = vkinit::color_blend_attachment_state();
	// Depth stencil
	pipeline_builder.depth_stencil = vkinit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
	// Pipeline layout
	pipeline_builder.pipeline_layout = mesh_pipeline_layout;
	
	// Vertex buffer description
	VertexInputDescription vertex_description = Vertex::get_vertex_description();
	pipeline_builder.vertex_input_info.vertexAttributeDescriptionCount = vertex_description.attributes.size();
	pipeline_builder.vertex_input_info.pVertexAttributeDescriptions = vertex_description.attributes.data();
	pipeline_builder.vertex_input_info.vertexBindingDescriptionCount = vertex_description.bindings.size();
	pipeline_builder.vertex_input_info.pVertexBindingDescriptions = vertex_description.bindings.data();

	pipeline_builder.shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));
	pipeline_builder.shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, triangleFragShader));


	// Build the mesh pipeline
	mesh_pipeline = pipeline_builder.build_pipeline(device, render_pass);
	create_material(mesh_pipeline, mesh_pipeline_layout, "default_mesh");

	// Destroy all shader modules outside of the deletion queue
	vkDestroyShaderModule(device, triangleFragShader, nullptr);
	vkDestroyShaderModule(device, meshVertShader, nullptr);
	main_deletion_queue.push_function([=](){
		vkDestroyPipeline(device, mesh_pipeline, nullptr);
		vkDestroyPipelineLayout(device, mesh_pipeline_layout, nullptr);
	});
}
// Loads a shader module from a SPIR-V file. Returns false if there are errors.
bool VulkanEngine::load_shader_module(const char* filepath, VkShaderModule* out_shader_module) {
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
// Loads the mesh object with vertex information
void VulkanEngine::load_meshes() {
	Mesh triangle_mesh;
	triangle_mesh.vertices.resize(3);
	// vertex positions
	triangle_mesh.vertices[0].position = {1.0f,1.0f,0.0f};
	triangle_mesh.vertices[1].position = {-1.0f,1.0f,0.0f};
	triangle_mesh.vertices[2].position = {0.0f,-1.0f,0.0f};
	// vertex colors
	triangle_mesh.vertices[0].color = {48.0f,83.0f,112.0f};
	triangle_mesh.vertices[1].color = {48.0f,83.0f,112.0f};
	triangle_mesh.vertices[2].color = {48.0f,83.0f,112.0f};
	upload_mesh(triangle_mesh);
	
	Mesh monkey_mesh;
	monkey_mesh.load_from_obj("../assets/monkey_smooth.obj"); // Load monkey
	upload_mesh(monkey_mesh);

	std::string obj_dir = "../../../Test/OBJ_Files/"; // Directory for downloaded OBJ files

	std::string filename = obj_dir + "Koenigsegg.obj";
	Mesh koenigsegg_mesh;
	koenigsegg_mesh.load_from_obj(filename.c_str()); // Load monkey
	upload_mesh(koenigsegg_mesh);
	// Mesh ironman;
	// ironman.load_from_obj("../../../Test/OBJ_Files/IronMan.obj");
	// upload_mesh(ironman);
	// Copy the meshes to the unordered list and delete them (later)
	meshes["monkey"] = monkey_mesh;
	meshes["koenigsegg"] = koenigsegg_mesh;
	meshes["triangle"] = triangle_mesh;
}
// Allocates CPU side buffer, fills it, then sends it to GPU memory
void VulkanEngine::upload_mesh(Mesh& mesh) {
	const size_t buffer_size = mesh.vertices.size() * sizeof(Vertex);
	// Allocate the CPU-side staging buffer
	AllocatedBuffer staging_buffer = create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	
	// Staging buffer is allocated, so now put vertex data inside of it.
	void* data;
	vmaMapMemory(allocator, staging_buffer.allocation, &data); // Map the data to a point in the allocation.
	memcpy(data, mesh.vertices.data(), buffer_size);
	vmaUnmapMemory(allocator, staging_buffer.allocation); // This doesn't have to be unmapped, but unmapping tells the driver we are done sending data

	// Now create the GPU-side buffer
	mesh.vertex_buffer = create_buffer(buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	// Buffers created, time to copy
	immediate_submit([=](VkCommandBuffer cmd) {
		VkBufferCopy copy;
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = buffer_size;
		vkCmdCopyBuffer(cmd, staging_buffer.buffer, mesh.vertex_buffer.buffer, 1, &copy);
	});
	// Buffer copied, clean up
	main_deletion_queue.push_function([=](){
		vmaDestroyBuffer(allocator, mesh.vertex_buffer.buffer, mesh.vertex_buffer.allocation);
	});
	vmaDestroyBuffer(allocator, staging_buffer.buffer, staging_buffer.allocation); // We can instantly destroy this to free up the CPU memory
}
// Adds material to the unordered_map of materials
Material* VulkanEngine::create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name) {
	Material mat;
	mat.pipeline = pipeline;
	mat.pipeline_layout = layout;
	materials[name] = mat;
	return &materials[name];
}
Material* VulkanEngine::get_material(const std::string& name) {
	auto it = materials.find(name);
	if (it == materials.end()) {
		return nullptr;
	} else {
		return &(*it).second;
	}
}
Mesh* VulkanEngine::get_mesh(const std::string& name) {
	auto it = meshes.find(name);
	if (it == meshes.end()) {
		return nullptr;
	} else {
		return &(*it).second;
	}
}
AllocatedBuffer VulkanEngine::create_buffer(size_t alloc_size, VkBufferUsageFlags usage_flags, VmaMemoryUsage memory_usage) {
	VkBufferCreateInfo bufinfo={}; // Buffer info
	bufinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufinfo.pNext = nullptr;
	bufinfo.size = alloc_size;
	bufinfo.usage = usage_flags;
	VmaAllocationCreateInfo allocinfo={}; // Vma allocation info
	allocinfo.usage = memory_usage;
	AllocatedBuffer new_buffer; // Buffer struct
	VK_CHECK(vmaCreateBuffer(allocator, &bufinfo, &allocinfo, &new_buffer.buffer, &new_buffer.allocation, nullptr));
	return new_buffer;
}
// Pad uniform buffer data to fit the alignment requirements
size_t VulkanEngine::pad_uniform_buffer_size(size_t original_size) {
	// Calculate required alignment based on minimum device offset alignment
	size_t minUBOalignment = gpu_properties.limits.minUniformBufferOffsetAlignment;
	size_t aligned_size = original_size;
	if (minUBOalignment > 0) {
		aligned_size = (aligned_size + minUBOalignment - 1) & ~(minUBOalignment - 1);
	}
	return aligned_size;
}
// Immediately submit a command to a command buffer
void VulkanEngine::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function) {
	VkCommandBuffer cmd = upload_context.command_buffer;
	// Using this flag because we are only submitting a command once before resetting it
	VkCommandBufferBeginInfo begininfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK_CHECK(vkBeginCommandBuffer(cmd, &begininfo));
	function(cmd); // Execute the function
	VK_CHECK(vkEndCommandBuffer(cmd));
	VkSubmitInfo submit = vkinit::submit_info(&cmd);
	// Submit command buffer to the queue and execute
	// Upload fence will block until graphics commands finish execution
	VK_CHECK(vkQueueSubmit(graphics_queue, 1, &submit, upload_context.upload_fence));
	vkWaitForFences(device, 1, &upload_context.upload_fence, true, 9999999999);
	vkResetFences(device, 1, &upload_context.upload_fence);
	vkResetCommandPool(device, upload_context.command_pool, 0); // Reset the command buffers inside the cmd pool
}
// Initialize descriptor sets and related objects
void VulkanEngine::init_descriptors() {
	// Create a descriptor pool that will hold 10 uniform buffers (10 is arbitrary for now)
	std::vector<VkDescriptorPoolSize> sizes = {
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10}
	};
	VkDescriptorPoolCreateInfo pool_info={};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.pNext = nullptr;
	pool_info.flags = 0; // No flags
	pool_info.maxSets = 10;
	pool_info.poolSizeCount = (uint32_t)sizes.size();
	pool_info.pPoolSizes = sizes.data();
	vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool);

	// Initialize Uniform Buffer layouts
	// Create binding for camera uniform buffer
	VkDescriptorSetLayoutBinding camera_bind=vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
	// Create binding for scene parameter uniform buffer
	VkDescriptorSetLayoutBinding scene_bind=vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	VkDescriptorSetLayoutBinding bindings[] = {camera_bind, scene_bind};
	// Create the global descriptor set layout
	// TODO: abstract this with vkinit?
	VkDescriptorSetLayoutCreateInfo setinfo={};
	setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setinfo.pNext = nullptr;
	setinfo.bindingCount = 2; // Length of bindings[]
	setinfo.flags = 0; // No flags
	setinfo.pBindings = bindings;
	VK_CHECK(vkCreateDescriptorSetLayout(device, &setinfo, nullptr, &global_set_layout));

	// Initialize Storage Buffer layouts
	VkDescriptorSetLayoutBinding object_bind=vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
	VkDescriptorSetLayoutCreateInfo objsetinfo={};
	objsetinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	objsetinfo.pNext = nullptr;
	objsetinfo.bindingCount = 1;
	objsetinfo.flags = 0;
	objsetinfo.pBindings = &object_bind;
	VK_CHECK(vkCreateDescriptorSetLayout(device, &objsetinfo, nullptr, &object_set_layout));

	main_deletion_queue.push_function([&]() {
		vkDestroyDescriptorSetLayout(device, object_set_layout, nullptr);
		vkDestroyDescriptorSetLayout(device, global_set_layout, nullptr);
		vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
	});
	// Because of alignment, we need to increase the size of the buffer so that it fits 2 padded GPUSceneData structs
	const size_t scene_param_buffer_size = FRAME_OVERLAP * pad_uniform_buffer_size(sizeof(GPUSceneData));
	scene_parameter_buffer = create_buffer(scene_param_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	// Create and allocate the uniform buffers for the camera matricies
	for (int i = 0; i < FRAME_OVERLAP; i++) {
		frames[i].camera_buffer = create_buffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		
		const int MAX_OBJECTS = 10000;
		frames[i].object_buffer = create_buffer(sizeof(GPUObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		// Allocate descriptor set via the descriptor pool and descriptor layout
		VkDescriptorSetAllocateInfo allocinfo={};
		allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocinfo.pNext = nullptr;
		allocinfo.descriptorPool = descriptor_pool;
		allocinfo.descriptorSetCount = 1;
		allocinfo.pSetLayouts = &global_set_layout;
		VK_CHECK(vkAllocateDescriptorSets(device, &allocinfo, &frames[i].global_descriptor));
		// Object Storage Buffer Writing
		VkDescriptorSetAllocateInfo objallocinfo={};
		objallocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		objallocinfo.pNext = nullptr;
		objallocinfo.descriptorPool = descriptor_pool;
		objallocinfo.descriptorSetCount = 1;
		objallocinfo.pSetLayouts = &object_set_layout;
		VK_CHECK(vkAllocateDescriptorSets(device, &objallocinfo, &frames[i].object_descriptor));

		// Make the descriptor point to the camera buffer
		VkDescriptorBufferInfo camera_info={};
		camera_info.buffer = frames[i].camera_buffer.buffer;
		camera_info.offset = 0; // Starts at 0 offset
		camera_info.range = sizeof(GPUCameraData); // Size of the struct we are putting in the buffer
		VkWriteDescriptorSet camera_write = vkinit::write_descriptorset_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, frames[i].global_descriptor, &camera_info, 0);
		// Make the descriptor point to the scene param buffer at an offset
		VkDescriptorBufferInfo scene_info={};
		scene_info.buffer = scene_parameter_buffer.buffer;
		scene_info.offset = 0;
		// scene_info.offset = pad_uniform_buffer_size(sizeof(GPUSceneData)) * i;
		scene_info.range = sizeof(GPUSceneData);
		VkWriteDescriptorSet scene_write = vkinit::write_descriptorset_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, frames[i].global_descriptor, &scene_info, 1);
		// Make the object descriptor point to the object buffer
		VkDescriptorBufferInfo object_info={};
		object_info.buffer = frames[i].object_buffer.buffer;
		object_info.offset = 0;
		object_info.range = sizeof(GPUObjectData) * MAX_OBJECTS;
		VkWriteDescriptorSet object_write = vkinit::write_descriptorset_buffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, frames[i].object_descriptor, &object_info, 0);

		VkWriteDescriptorSet set_writes[] = {camera_write, scene_write, object_write};
		vkUpdateDescriptorSets(device, 3, set_writes, 0, nullptr);

		
		main_deletion_queue.push_function([&, i]() {
			vmaDestroyBuffer(allocator, frames[i].camera_buffer.buffer, frames[i].camera_buffer.allocation);
			vmaDestroyBuffer(allocator, frames[i].object_buffer.buffer, frames[i].object_buffer.allocation);
		});
	}
	main_deletion_queue.push_function([&]() {vmaDestroyBuffer(allocator, scene_parameter_buffer.buffer, scene_parameter_buffer.allocation);});
}
// Destroyer function
void VulkanEngine::cleanup() {
	if (isInitialized) {

		// Make sure GPU has finished using the objects we are about to delete
		for (int i = 0; i < FRAME_OVERLAP; i++) {
			vkWaitForFences(device, 1, &frames[i].render_fence, true, 1000000000);
		}

		main_deletion_queue.flush();

		// These are special, so we don't add them to the deletion queue
		vmaDestroyAllocator(allocator);
		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkb::destroy_debug_utils_messenger(instance, debug_messenger);
		vkDestroyInstance(instance, nullptr);
		SDL_DestroyWindow(window);
	}
}
// Add objects to the scene
void VulkanEngine::init_scene() {
	RenderObject monkey;
	monkey.mesh = get_mesh("monkey");
	monkey.material = get_material("default_mesh");
	monkey.transform_matrix = glm::translate(glm::mat4{1.0f}, glm::vec3(-5.0f, 0.0f, 0.0f));
	// Add monkey to renderables list
	renderables.push_back(monkey);

	RenderObject koenigsegg;
	koenigsegg.mesh = get_mesh("koenigsegg");
	koenigsegg.material = get_material("default_mesh");
	koenigsegg.transform_matrix = glm::translate(glm::mat4{1.0f}, glm::vec3(5.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3{0.3});
	renderables.push_back(koenigsegg);
	// Add several triangles using a loop
	for (int x = -20; x <= 20; x++) {
		for (int y = -20; y <= 20; y++) {
			RenderObject tri;
			tri.mesh = get_mesh("triangle");
			tri.material = get_material("default_mesh");
			glm::mat4 translation = glm::translate(glm::mat4{ 1.0 }, glm::vec3(x, -10.0f, y));
			glm::mat4 rotation = glm::rotate(glm::mat4{1.0f}, glm::radians(90.0f), glm::vec3(1.0f,0.0f,0.0f));
			glm::mat4 scale = glm::scale(glm::mat4{ 1.0 }, glm::vec3(0.4, 0.4, 0.4));
			tri.transform_matrix = translation * rotation *  scale;
			renderables.push_back(tri);
		}
	}
	// Will sort here when the scene gets complex and it will actually make a difference
}
// Renders each object in the RenderObject list. This is actually a pretty decent system. The renderables list can be sorted
// to minimize how many pipeline bindings are needed. Rendering the same object many times with different push constants is pretty fast,
// since push constants are accessable by both CPU and GPU memory and don't have to be sent or re-bound.
void VulkanEngine::draw_objects(VkCommandBuffer cmd, RenderObject* first, int count) {
	
	glm::vec3 camPos = {0.0f, 6.0f, 10.0f}; // camera position
	glm::mat4 view = glm::lookAt(camPos / zoom, glm::vec3{0,0,0}, up);
	glm::vec3 lookAt = glm::vec3(0,0,0) - camPos;
	glm::vec3 right = glm::cross(lookAt,up);
	glm::vec3 up = glm::cross(right, lookAt);
	glm::mat4 rotation_hor = glm::rotate(glm::radians(rotate_hor += rotate_vel_hor), up);
	glm::mat4 rotation_ver = glm::rotate(glm::radians(rotate_ver += rotate_vel_ver), right);
	// glm::mat4 view = glm::translate(glm::mat4(1.0f), camPos);
	// Camera projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(70.0f), (float)windowExtent.width/(float)windowExtent.height, 0.1f, 200.0f);
	projection[1][1] *= -1;
	// Fill camera data struct
	GPUCameraData cam_data;
	cam_data.proj = projection;
	cam_data.view = view;
	cam_data.viewproj = projection * view;
	// Then copy it to the buffer that is pointed to by the descriptor set
	void* data;
	vmaMapMemory(allocator, get_current_frame().camera_buffer.allocation, &data);
	memcpy(data, &cam_data, sizeof(GPUCameraData));
	vmaUnmapMemory(allocator, get_current_frame().camera_buffer.allocation);

	// Map the scene parameter data
	float framed = (frameNumber / 120.f);
	scene_parameters.ambient_color = {sin(framed), 0, cos(framed), 1};
	char* scene_data;
	vmaMapMemory(allocator, scene_parameter_buffer.allocation, (void**)&scene_data);
	int frameIndex = frameNumber % FRAME_OVERLAP;
	scene_data += pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameIndex;
	memcpy(scene_data, &scene_parameters, sizeof(GPUSceneData));
	vmaUnmapMemory(allocator,scene_parameter_buffer.allocation);

	void* object_data;
	vmaMapMemory(allocator, get_current_frame().object_buffer.allocation, &object_data);
	GPUObjectData* objectSSBO = (GPUObjectData*)object_data; // Case the void* pointer to a complex type pointer and we can insert into it normally
	for (int i = 0; i < count; i++) {
		RenderObject& object = first[i];
		objectSSBO[i].modelMatrix = object.transform_matrix;
	}
	vmaUnmapMemory(allocator, get_current_frame().object_buffer.allocation);

	Mesh* lastmesh = nullptr;
	Material* lastmat = nullptr;
	for (int i = 0; i < count; i++) {
		RenderObject& object = first[i];
		// Only bind a new pipeline if the new material is different from the last one
		if (object.material != lastmat) {
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
			lastmat = object.material;
			uint32_t uniform_offset = pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameIndex;
			// Bind descriptor sets when changing the pipeline
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline_layout, 0, 1, &get_current_frame().global_descriptor, 1, &uniform_offset);
			// Bind object data descriptor
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline_layout, 1, 1, &get_current_frame().object_descriptor, 0, nullptr);
		}
		glm::mat4 model = rotation_hor * rotation_ver * object.transform_matrix;

		MeshPushConstants constants;
		constants.render_matrix = model;
		// Upload push constants to the GPU
		vkCmdPushConstants(cmd, object.material->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);
		// Only bind the mesh if it's different from the previous one
		if (object.mesh != lastmesh) {
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->vertex_buffer.buffer, &offset);
			lastmesh = object.mesh;
		}
		vkCmdDraw(cmd, object.mesh->vertices.size(), 1, 0, i); // This index i allows us to use the gl_BaseInstance in the shader
	}
	// Model rotation
	// glm::mat4 model = glm::rotate(glm::mat4{1.0f}, glm::radians(frameNumber * 1.2f), glm::vec3(0,1,0));
	// glm::mat4 mesh_matrix = projection * view * model; // Final mesh matrix
}
// Draw to framebuffer each window
void VulkanEngine::draw() {
	// First, wait for the last frame to render
	VK_CHECK(vkWaitForFences(device, 1, &get_current_frame().render_fence, true, 1000000000));
	VK_CHECK(vkResetFences(device, 1, &get_current_frame().render_fence));
	// Request image from the swapchain
	uint32_t swapchain_image_index;
	VK_CHECK(vkAcquireNextImageKHR(device, swapchain, 1000000000, get_current_frame().present_semaphore, nullptr, &swapchain_image_index));
	// If we get here, we can safely reset the command buffer
	VK_CHECK(vkResetCommandBuffer(get_current_frame().command_buffer, 0));
	VkCommandBuffer cmd = get_current_frame().command_buffer; // renaming for brevity
	// Begin command buffer recording
	VkCommandBufferBeginInfo cmd_begininfo={};
	cmd_begininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_begininfo.pNext = nullptr;
	cmd_begininfo.pInheritanceInfo = nullptr;
	cmd_begininfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begininfo));

	VkClearValue clear; // Color clear
	// float flash = abs(sin(frameNumber/120.0f));
	clear.color = {{0.0f, 0.0f, 0.0f, 0.0f}};
	VkClearValue depth_clear; // Depth clear
	depth_clear.depthStencil.depth = 1.0f;
	VkClearValue clearvalues[2] = {clear, depth_clear};

	// Start the main renderpass
	VkRenderPassBeginInfo rpinfo=vkinit::renderpass_begin_info(render_pass, windowExtent, framebuffers[swapchain_image_index]);
	rpinfo.clearValueCount = 2;
	rpinfo.pClearValues = &clearvalues[0];
	vkCmdBeginRenderPass(cmd, &rpinfo, VK_SUBPASS_CONTENTS_INLINE);

	
	// RENDER HERE
	draw_objects(cmd, renderables.data(), renderables.size());

	// End render pass
	vkCmdEndRenderPass(cmd); // Finishes rendering and transitions image to what we specified
	VK_CHECK(vkEndCommandBuffer(cmd)); // Can't add any more commands, but can now submit to the queue
	// Submit command buffer
	VkSubmitInfo subinf={};
	subinf.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	subinf.pNext = nullptr;
	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subinf.pWaitDstStageMask = &wait_stage;
	subinf.waitSemaphoreCount = 1; // Waiting on the present semaphore to be signaled
	subinf.pWaitSemaphores = &get_current_frame().present_semaphore;
	subinf.signalSemaphoreCount = 1; // Going to signal the render semaphore since we can begin rendering commands on this cmd buffer again
	subinf.pSignalSemaphores = &get_current_frame().render_semaphore;
	subinf.commandBufferCount = 1; // Command buffer to be submitted
	subinf.pCommandBuffers = &cmd;
	VK_CHECK(vkQueueSubmit(graphics_queue, 1, &subinf, get_current_frame().render_fence));
	// Present rendered image to the screen
	VkPresentInfoKHR present_info={};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pNext = nullptr;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swapchain;
	present_info.waitSemaphoreCount = 1; // Waiting on the render command to complete to present to screen
	present_info.pWaitSemaphores = &get_current_frame().render_semaphore;
	present_info.pImageIndices = &swapchain_image_index;
	VK_CHECK(vkQueuePresentKHR(graphics_queue, &present_info));

	frameNumber++;
}
// Encloses the main loop which polls events and draws to framebuffer each iteration.
void VulkanEngine::run() {
	SDL_Event e;
	bool bQuit = false;
	//main loop
	while (!bQuit) {
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0) {
			//close the window when user alt-f4s or clicks the X button			
			switch (e.type) {
			case SDL_QUIT:
				 bQuit = true;
				 std::cout << "Window Closed" << std::endl;
				 break;
			 case SDL_KEYDOWN:
				switch (e.key.keysym.sym){
				case SDLK_SPACE:
					selectedShader += 1;
					if (selectedShader > 1) selectedShader = 0;
					break;
				case SDLK_w:
					rotate_vel_ver = rotate_speed;
					break;
				case SDLK_s:
					rotate_vel_ver = -rotate_speed;
					break;
				case SDLK_a:
					rotate_vel_hor = -rotate_speed;
					break;
				case SDLK_d:
					rotate_vel_hor = rotate_speed;
					break;
				default:
					break;
				}
				break;
			case SDL_KEYUP:
				switch (e.key.keysym.sym){
				case SDLK_SPACE:
					selectedShader += 1;
					if (selectedShader > 1) selectedShader = 0;
					break;
				case SDLK_w:
					if (rotate_vel_ver > 0)
						rotate_vel_ver = 0.0f;
					break;
				case SDLK_s:
					if (rotate_vel_ver < 0)
						rotate_vel_ver = 0.0f;
					break;
				case SDLK_a:
					if (rotate_vel_hor < 0)
						rotate_vel_hor = 0.0f;
					break;
				case SDLK_d:
					if (rotate_vel_hor > 0)
						rotate_vel_hor = 0.0f;
					break;
				default:
					break;
				}
			break;
			default:
				break;
			}
		}

		draw();
	}
}


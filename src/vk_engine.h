﻿// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include "vk_types.h"
#include <vk_mesh.h>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <functional>
#include <deque>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

struct DeletionQueue {
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& function) {
		deletors.push_back(function);
	}

	void flush() {
		// Run through the queue in the reverse order it was filled in and execute all of the functions
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)(); // call the function
		}

		deletors.clear();
	}
};

struct MeshPushConstants {
	glm::vec4 data;
	glm::mat4 render_matrix;
};

struct Material {
	VkDescriptorSet texture_set{VK_NULL_HANDLE};
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;
};

struct RenderObject {
	Mesh* mesh;
	Material* material;
	glm::mat4 transform_matrix;
};

struct GPUCameraData {
	glm::mat4 view; // Camera location/transform
	glm::mat4 proj; // For perspective
	glm::mat4 viewproj; // view * proj (to avoid doing so in the shader)
};

struct FrameData {
	VkSemaphore present_semaphore, render_semaphore;
	VkFence render_fence;
	VkCommandPool command_pool;
	VkCommandBuffer command_buffer;
	AllocatedBuffer camera_buffer; // Buffer that holds a single GPUCameraData to use when rendering
	VkDescriptorSet global_descriptor;
	AllocatedBuffer object_buffer; // Storage buffer
	VkDescriptorSet object_descriptor;
};

struct GPUSceneData {
	glm::vec4 fog_color;
	glm::vec4 fog_distances;
	glm::vec4 ambient_color;
	glm::vec4 sunlight_color;
	glm::vec4 sunlight_direction;
};

struct GPUObjectData {
	glm::mat4 modelMatrix;
};

struct UploadContext {
	VkFence upload_fence;
	VkCommandPool command_pool;
	VkCommandBuffer command_buffer;
};

struct Texture {
	AllocatedImage image;
	VkImageView image_view;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanEngine {
public:
	bool isInitialized{ false };
	int frameNumber {0};
	// Dimensions of the window extent
	VkExtent2D windowExtent{ 1920 , 1080 };
	struct SDL_Window* window{ nullptr };
	// VkInitialization structures
	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
	VkPhysicalDevice chosenGPU;
	VkPhysicalDeviceProperties gpu_properties;
	VkDevice device;
	VkSurfaceKHR surface;
	// Swapchain structures
	VkSwapchainKHR swapchain;
	VkFormat swapchain_image_format; // image format expected by the window system
	std::vector<VkImage> swapchain_images; // array of images in the swapchain
	std::vector<VkImageView> swapchain_image_views; // array of image views for swapchain images
	// Commands structures
	VkQueue graphics_queue;
	uint32_t graphics_queue_family;
	FrameData frames[FRAME_OVERLAP];
	// Renderpass structures
	VkRenderPass render_pass;
	std::vector<VkFramebuffer> framebuffers;
	// Deletion queue to make sure we destroy objects in the reverse order in which we create them
	DeletionQueue main_deletion_queue;
	// Object vma uses to allocate memory
	VmaAllocator allocator;
	// Default pipeline and layout
	VkPipelineLayout mesh_pipeline_layout;
	VkPipeline mesh_pipeline;
	// Meshes
	// Depth Image objects
	VkImageView depth_image_view;
	AllocatedImage depth_image;
	VkFormat depth_format;
	// Render object management
	std::vector<RenderObject> renderables; // default array of renderable objects
	std::unordered_map<std::string,Material> materials;
	std::unordered_map<std::string,Mesh> meshes;
	std::unordered_map<std::string,Texture> loaded_textures;
	// Descriptor Sets
	VkDescriptorSetLayout global_set_layout;
	VkDescriptorSetLayout object_set_layout;
	VkDescriptorPool descriptor_pool;
	GPUSceneData scene_parameters;
	AllocatedBuffer scene_parameter_buffer;
	// To copy data to GPU memory
	UploadContext upload_context;
	// Texture descriptor set layout
	VkDescriptorSetLayout single_texture_set_layout;


	int selectedShader{0}; // 0 -> red triangle, 1 -> colored triangle

	float zoom = 1.0f;
	float move_speed = 1.0f;
	float rotate_hor = 0.0f;
	float rotate_ver = 0.0f;
	float rotate_vel_hor = 0.0f;
	float rotate_vel_ver = 0.0f;
	float rotate_speed = 0.5f;
	glm::vec3 up = {0.0f, 1.0f, 0.0f};
	// std::unordered_map<SDL_Keycode, bool> keyboard; // Will this work to keep track of the keyboard since SDL_Keycode is enumerated??

	// :::::::::::::::::::::::::: Main Functions ::::::::::::::::::::::::::
	void init(); // Initializes engine
	void cleanup(); // Closes and cleans the engine
	void draw(); // Draw loop
	void run(); // Start the main loop

	// :::::::::::::::::::::::::: Utility Functions ::::::::::::::::::::::::::
	FrameData& get_current_frame(); // Returns true if lhs < rhs

	// Function to compare renderable objects to be able to sort them (and thus reduce the number of bindings and load them in faster)
	// TODO: Later when it will actually make a difference
	// May be able to just sort by memory address. Should probably move this to the Material/Mesh structs
	bool renderable_sorter(RenderObject const& lhs, RenderObject const& rhs) {
		return false;
	}
	size_t pad_uniform_buffer_size(size_t original_size); // Pad the uniform buffer sizes to align them properly with the minimum alignment size
	void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function); // Immediately execute command

	// :::::::::::::::::::::::::: Create Functions ::::::::::::::::::::::::::
	AllocatedBuffer create_buffer(size_t alloc_size, VkBufferUsageFlags usage_flags, VmaMemoryUsage memory_usage); // Create and allocate a buffer

private:
// :::::::::::::::::::::::::: Initialization Functions ::::::::::::::::::::::::::
	void init_vulkan();
	void init_swapchain();
	void init_commands();
	void init_default_renderpass();
	void init_framebuffers();
	void init_sync();
	void init_pipelines();
	void init_descriptors();

	// :::::::::::::::::::::::::: Loading Functions ::::::::::::::::::::::::::
	bool load_shader_module(const char* filepath, VkShaderModule* out_shader_module);
	void load_meshes();
	void load_images();
	void upload_mesh(Mesh& mesh); // Loads a mesh to a CPU buffer then transfers to GPU memory

	// :::::::::::::::::::::::::: Scene-Related Functions ::::::::::::::::::::::::::
	void draw_objects(VkCommandBuffer cmd, RenderObject* first, int count);
	void init_scene();
	Material* create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name); // Create materials and add them to the materials unordered_map
	Material* get_material(const std::string& name); // Returns nullptr if not found
	Mesh* get_mesh(const std::string& name); // Returns nullptr if not found
};

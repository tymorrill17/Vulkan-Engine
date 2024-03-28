// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vector>
#include <memory>
#include <optional>
#include <string>
#include <span>
#include <array>
#include <functional>
#include <deque>
#include <unordered_map>
#include <iostream>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm.hpp>
#include <gtx/transform.hpp>
#include <vec3.hpp>
#include <vec2.hpp>

#define VK_CHECK(x)																			\
	do {                                            			    					    \
		VkResult err = x;																	\
		if (err) {																			\
			std::cout << "Detected Vulkan Error: " << string_VkResult(err) << std::endl;	\
			abort();																		\
		}																					\
	} while (0)																				\

// Will hold the buffer we allocate, along with its allocation data.
struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};

struct AllocatedImage {
    VkImage image;
	VkImageView imageview;
    VmaAllocation allocation;
	VkExtent3D extent;
	VkFormat format;
};
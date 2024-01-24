// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h>

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
    VmaAllocation allocation;
};
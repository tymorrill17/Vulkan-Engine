#pragma once

#include <vk_types.h>
#include <vk_engine.h>

namespace vkutil {
    bool load_image_from_file(VulkanEngine& engine, const char* file, AllocatedImage& out_image);
    void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout current_layout, VkImageLayout new_layout);
}
#include <vk_texture.h>
#include <iostream>

#include <vk_initializers.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

bool vkutil::load_image_from_file(VulkanEngine& engine, const char* file, AllocatedImage& out_image) {
    int texW, texH, texChannels;
    // Load the texture directly into an array of pixels
    stbi_uc* pixels = stbi_load(file, &texW, &texH, &texChannels, STBI_rgb_alpha); // STBI_rgb_alpha will load pixels as RGBA 4 channels, which will match Vulkan format
    if (!pixels) {
        std::cout << "Failed to load texture file " << file << std::endl;
        return false;
    }

    // Create a staging buffer to load the pixels into
    void* pixel_ptr = pixels;
    VkDeviceSize image_size = texH * texW * 4; // 4 bytes per pixel times the # of pixels
    // R8G8B8A8 format matches exactly with the pixels loaded from stb
    VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;
    AllocatedBuffer staging_buffer = engine.create_buffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    // copy pixel data to buffer
    void* data;
    vmaMapMemory(engine.allocator, staging_buffer.allocation, &data);
    memcpy(data, pixel_ptr, static_cast<size_t>(image_size));
    vmaUnmapMemory(engine.allocator, staging_buffer.allocation);
    // We no longer need the loaded data, since it is in the staging buffer
    stbi_image_free(pixels);

    // Now create the image
    VkExtent3D extent;
    extent.width = static_cast<uint32_t>(texW);
    extent.height = static_cast<uint32_t>(texH);
    extent.depth = 1;
    // VK_IMAGE_USAGE_SAMPLED_BIT specifies that the image can occupy a descriptor set slot and be sampled bu a shader
    VkImageCreateInfo ici = vkinit::image_create_info(image_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, extent);
    AllocatedImage new_image;
    VmaAllocationCreateInfo aci = {};
    aci.flags = VMA_MEMORY_USAGE_GPU_ONLY;
    // Allocate and create image
    vmaCreateImage(engine.allocator, &ici, &aci, &new_image.image, &new_image.allocation, nullptr);

    // Now we can copy the texture from the buffer to the image
    // You can't just copy data from a buffer to an image, since the image requires a layout. 
    // Thus we will do a layout transition to Linear, which is the best for copying buffer -> image
    engine.immediate_submit([&](VkCommandBuffer cmd) {
        VkImageSubresourceRange range;
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;
        // If you do a pipeline barrier with an image barrier, you can transform the image to the correct format and layout
        VkImageMemoryBarrier image_barrier_totransfer = {};
        image_barrier_totransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        image_barrier_totransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_barrier_totransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        image_barrier_totransfer.image = new_image.image;
        image_barrier_totransfer.subresourceRange = range;
        image_barrier_totransfer.srcAccessMask = 0;
        image_barrier_totransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        // Barrier the image to the transfer-receive layout
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_barrier_totransfer);

        // Now the image is ready to receive buffer data, so lets copy
        VkBufferImageCopy bic = {};
        bic.bufferOffset = 0;
        bic.bufferRowLength = 0;
        bic.bufferImageHeight = 0;
        bic.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bic.imageSubresource.mipLevel = 0;
        bic.imageSubresource.baseArrayLayer = 0;
        bic.imageSubresource.layerCount = 1;
        bic.imageExtent = extent;
        vkCmdCopyBufferToImage(cmd, staging_buffer.buffer, new_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bic);

        // The image has the correct pixel data, now change its layout to a shader-readable one
        VkImageMemoryBarrier image_barrier_toreadable = image_barrier_totransfer;
        image_barrier_toreadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        image_barrier_toreadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_barrier_toreadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        image_barrier_toreadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_barrier_toreadable);
    });
    engine.main_deletion_queue.push_function([=]() {
        vmaDestroyImage(engine.allocator, new_image.image, new_image.allocation);
    });
    vmaDestroyBuffer(engine.allocator, staging_buffer.buffer, staging_buffer.allocation);
    std::cout << "Texture loaded successfully: " << file << std::endl;
    out_image = new_image;
    return true;
}

void vkutil::transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) {
    VkImageMemoryBarrier2 image_barrier = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
    image_barrier.pNext = nullptr;
    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
    image_barrier.oldLayout = current_layout;
    image_barrier.newLayout = new_layout;
    VkImageAspectFlags aspect_mask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    image_barrier.subresourceRange = vkinit::image_subresource_range(aspect_mask);
    image_barrier.image = image;
    VkDependencyInfo depinfo{};
    depinfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depinfo.pNext = nullptr;
    depinfo.imageMemoryBarrierCount = 1;
    depinfo.pImageMemoryBarriers = &image_barrier;
    vkCmdPipelineBarrier2(cmd,&depinfo);
}

void vkutil::copy_image_to_image(VkCommandBuffer cmd, VkImage src, VkExtent2D src_size, VkImage dst, VkExtent2D dst_size) {
    // Blit is a more powerful way to copy an image, as the layouts and subresource ranges may be different.
    VkImageBlit2 blit_region{.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr};
    blit_region.srcOffsets[1].x = src_size.width;
    blit_region.srcOffsets[1].y = src_size.height;
    blit_region.srcOffsets[1].z = 1;

    blit_region.dstOffsets[1].x = dst_size.width;
    blit_region.dstOffsets[1].y = dst_size.height;
    blit_region.dstOffsets[1].z = 1;

    blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.srcSubresource.baseArrayLayer = 0;
    blit_region.srcSubresource.layerCount = 1;
    blit_region.srcSubresource.mipLevel = 0;

    blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.dstSubresource.baseArrayLayer = 0;
    blit_region.dstSubresource.layerCount = 1;
    blit_region.dstSubresource.mipLevel = 0;

    VkBlitImageInfo2 blitinfo{.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr};
    blitinfo.dstImage = dst;
    blitinfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    blitinfo.srcImage = src;
    blitinfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    blitinfo.filter = VK_FILTER_LINEAR;
    blitinfo.regionCount = 1;
    blitinfo.pRegions = &blit_region;

    vkCmdBlitImage2(cmd, &blitinfo);
}
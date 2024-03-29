#pragma once

#include <vk_types.h>

struct VertexInputDescription {
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;
    VkPipelineVertexInputStateCreateFlags flags=0;
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 uv;
    static VertexInputDescription get_vertex_description();
};

struct Mesh {
    std::vector<Vertex> vertices;
    AllocatedBuffer vertex_buffer;

    bool load_from_obj(const char* filename);
};

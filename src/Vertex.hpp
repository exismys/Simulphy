#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace Simulphy {
struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
    glm::vec2 texCoord;
	glm::vec3 normal;

	static vk::VertexInputBindingDescription getBindingDescription() {
		return {.binding = 0, .stride = sizeof(Vertex), .inputRate = vk::VertexInputRate::eVertex};
	}

	static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions() {
		return {
            {
                {.location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, pos)},
                {.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, color)},
                {.location = 2, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, texCoord)},
				{.location = 3, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, normal)}
            }
        };
	}

    bool operator==(const Vertex &other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};
}

template <>
struct std::hash<Simulphy::Vertex> {
	size_t operator()(Simulphy::Vertex const &vertex) const noexcept {
		return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
	}
};

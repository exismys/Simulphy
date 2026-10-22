#pragma once

#include <glm/glm.hpp>

#include "../Mesh.hpp"

namespace Simulphy {

struct Transform {
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};
};

struct Object {
    Mesh* mesh = nullptr;
    const uint32_t materialIndex;
    Transform transform;
    bool hasTexture;
};

enum class LightSourceType {
    Point,
    Directional,
    Ambient
};

struct LightSource {
    LightSourceType type;
    float intensity;
    glm::vec3 position;
    glm::vec3 direction;
};

}

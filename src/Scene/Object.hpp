#pragma once

#include <glm/glm.hpp>

#include "../Mesh.hpp"
#include "../Texture.hpp"

namespace Simulphy {
    struct Transform {
        glm::vec3 position{0.0f};
        glm::vec3 rotation{0.0f};
        glm::vec3 scale{1.0f};
    };

    struct Object {
        Mesh* mesh = nullptr;
        const Texture* texture = nullptr;
        Transform transform;
    };
}

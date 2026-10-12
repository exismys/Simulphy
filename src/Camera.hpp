#pragma once

#include <cmath>

#include <glm/glm.hpp>

namespace Simulphy {
class Camera {
public:
    glm::vec3 position;
    glm::vec3 target;
    float pitch;
    float yaw;
    float roll;

    Camera(glm::vec3 position, glm::vec3 target)
        : position(position), target(target), pitch(0.0f), yaw(0.0f), roll(0.0f) {

        glm::vec3 normalizedDir = glm::normalize(target - position);
        yaw = std::atan2(normalizedDir.x, normalizedDir.z);
        pitch = std::asin(normalizedDir.y);
    }
};
}
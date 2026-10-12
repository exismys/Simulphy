#pragma once


#include <glm/glm.hpp>

namespace Simulphy {
class Camera {
public:
    glm::vec3 position;
    glm::vec3 target;
    float pitch;
    float yaw;
    float roll;

    Camera(glm::vec3 position, glm::vec3 target) {
        position = position;
        target = target;

        glm::vec3 normalizedDir = glm::normalize(target - position);
        yaw = std::atan2(normalizedDir.x, normalizedDir.z);
        pitch = std::asin(normalizedDir.y);
    }
};
}
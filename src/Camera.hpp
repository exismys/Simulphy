#pragma once

#include <cmath>
#include <iostream>

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

        glm::vec3 normalizedViewDir = glm::normalize(target - position);
        yaw = std::atan2(normalizedViewDir.x, normalizedViewDir.z);
        pitch = std::asin(normalizedViewDir.y);

        std::cout << "Initial camera yaw: " << yaw << std::endl;
        std::cout << "Initial camera pitch: " << pitch << std::endl;
    }
};
}

#pragma once

#include "../Physics/Particle.hpp"
#include "glm/ext/vector_double3.hpp"

namespace Simulphy {

class Simulation {
public:
    std::vector<Physics::Particle> particles;

    Simulation() {
        Physics::Particle particle{
            .mass = 1,
            .radius = 0.1,

            .position = glm::dvec3{0.0f, 1.0f, 0.0f},
            .velocity = glm::dvec3{0.0f},
            .acceleration = glm::dvec3{0.0f},

            .force = glm::dvec3{0.0f},
            .isStatic = false
        };

        particles.push_back(particle);
    }

    void update(float frameTime) {
        particles[0].acceleration = glm::dvec3{0.0, 1.0f, 0.0f};

        Physics::integrate(particles[0], frameTime);
    }
};

}

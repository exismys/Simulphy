#pragma once

#include "../Physics/Particle.hpp"
#include "glm/ext/vector_double3.hpp"

constexpr int PARTICLES = 10;

namespace Simulphy {

class Simulation {
public:
    std::vector<Physics::Particle> particles;

    Simulation() {
        for (int i = 0; i < PARTICLES; i++) {
            Physics::Particle particle{
                .mass = 1,
                .radius = 0.1,

                .position = glm::dvec3{0.0f, 1.0f + i, 0.0f},
                .velocity = glm::dvec3{0.0f},
                .acceleration = glm::dvec3{0.0f},

                .force = glm::dvec3{0.0f},
                .isStatic = false
            };

            particles.push_back(particle);

        }
    }

    void update(float frameTime) {
        for (int i = 0; i < PARTICLES; i++) {
            particles[i].acceleration = glm::dvec3{0.0, 1.0f, 0.0f};
            Physics::integrate(particles[i], frameTime);
        }
    }
};

}

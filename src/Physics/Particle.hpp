#pragma once

#include <glm/glm.hpp>

namespace Physics {

struct Particle {
    double mass = 1.0;
    double radius = 0.1;

    glm::dvec3 position{0.0};
    glm::dvec3 velocity{0.0};
    glm::dvec3 acceleration{0.0};
    glm::dvec3 force{0.0};

    bool isStatic = false;
};

void applyForce(Particle& particle, const glm::dvec3& force);
void integrate(Particle& particle, double dt);
void applyGravity(Particle& particle, const glm::dvec3& gravity);

}

#include "Particle.hpp"

namespace Physics {

void applyForce(Particle& particle, const glm::dvec3& force) {
    if (particle.isStatic) return;

    particle.force += force;
}

void integrate(Particle& particle, double dt) {
    if (particle.isStatic) return;

    particle.velocity += particle.acceleration * dt;
    particle.position += particle.velocity * dt;
}

void applyGravity(Particle& particle, const glm::dvec3& gravity) {

}

}

#include "Particle.hpp"

namespace Physics {

void applyForce(Particle& particle, const glm::dvec3& force) {
    if (particle.isStatic) return;
    
    particle.force += force;
}

void integrate(Particle& particle, double dt) {

}

void applyGravity(Particle& particle, const glm::dvec3& gravity) {

}

}

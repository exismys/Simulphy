#pragma once

#include "Object.hpp"
#include "../Device.hpp"
#include "../Material.hpp"
#include "../Simulation/Simulation.hpp"

namespace Simulphy {

class Scene {
private:
    std::vector<Object> objects;
    std::vector<Mesh*> meshes;
    std::vector<Material*> materials;
    std::vector<LightSource> lightSources;

    uint32_t nonSimObjects;

public:
    Scene(){}

    ~Scene() {
        for (Mesh* mesh: meshes) {
            delete mesh;
        }

        for (Material* material: materials) {
            delete material;
        }
    }

    const std::vector<Object>& getObjects() const {
        return objects;
    }

    const std::vector<Mesh*>& getMeshes() const {
        return meshes;
    }

    const std::vector<Material*>& getMaterials() const {
        return materials;
    }

    const std::vector<LightSource>& getLightSources() const {
        return lightSources;
    }

    static Scene createSceneFromSim(const Device& device, const Simulation& sim) {
        Scene scene;
        
        // Materials
        scene.materials.push_back(
            new Material(device, "textures/moss-floor-texture.jpg")
        );

        // Meshes
        scene.meshes.push_back(
            new Mesh(device, "plane")
        );

        scene.meshes.push_back(
            new Mesh(device, "sphere")
        );

        // Light Sources
        scene.lightSources.push_back(
            LightSource{
                .type = LightSourceType::Ambient,
                .intensity = 0.07,
            }
        );
        scene.lightSources.push_back(
            LightSource{
                .type = LightSourceType::Point,
                .intensity = 0.6,
                .position = {50, 1, -15},
            }
        );
        scene.lightSources.push_back(
            LightSource{
                .type = LightSourceType::Directional,
                .intensity = 0.33, 
                .direction = {-1, -1, -1},
            }
        );

        // Objects
        scene.objects.push_back({
            .mesh = scene.meshes[0],
            .materialIndex = 0,
            .transform = Transform{
                glm::vec3{0.0f, 0.0f, 0.0f},
                glm::vec3{0.0f},
                glm::vec3{200.0f},
            },
            .hasTexture = true
        });

        scene.nonSimObjects = scene.objects.size();

        for (const Physics::Particle& particle: sim.particles) {
            scene.objects.push_back({
                .mesh = scene.meshes[1],
                .materialIndex = 0,
                .transform = Transform{
                    particle.position,
                    glm::vec3{0.0f},
                    glm::vec3{1.0f},
                },
                .hasTexture = false
            });

        }

        return scene;
    }

    void updateSceneFromSim(const Simulation& sim) {
        for (int i = nonSimObjects; i < objects.size(); i++) {
            objects[i].transform.position = sim.particles[i-1].position;
        }
    }

    void createExpScene(const Device& device) {
        meshes.push_back(
            new Mesh(device, "cube")
        );
        meshes.push_back(
            new Mesh(device, "plane")
        );

        materials.push_back(
            new Material(device, "textures/texture.jpg")
        );
        materials.push_back(
            new Material(device, "textures/viking_room.png")
        );
        materials.push_back(
            new Material(device, "textures/moss-floor-texture.jpg")
        );

        objects.push_back({
            .mesh = meshes[0],
            .materialIndex = 0,
            .transform = Transform{
                glm::vec3{0.0f, 1.0f, 0.0f},
                glm::vec3{0.0f},
                glm::vec3{1.0f},
            },
            .hasTexture = true
        });

        objects.push_back({
            .mesh = meshes[0],
            .materialIndex = 1,
            .transform = Transform{
                glm::vec3{2.0f, 0.0f, 0.0f},
                glm::vec3{0.0f},
                glm::vec3{1.0f},
            },
            .hasTexture = true
        });

        objects.push_back({
            .mesh = meshes[1],
            .materialIndex = 2,
            .transform = Transform{
                glm::vec3{0.0f, 0.0f, 0.0f},
                glm::vec3{0.0f},
                glm::vec3{200.0f},
            },
            .hasTexture = true
        });

        objects.push_back({
            .mesh = meshes[0],
            .materialIndex = 4,
            .transform = Transform{
                glm::vec3{4.0f, 1.0f, 0.0f},
                glm::vec3{0.0f},
                glm::vec3{1.0f},
            },
            .hasTexture = false
        });
    }
};

}

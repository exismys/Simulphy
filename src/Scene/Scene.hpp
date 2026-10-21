#pragma once

#include "Object.hpp"
#include "../Device.hpp"
#include "../Material.hpp"

namespace Simulphy {
class Scene {
    private:
        std::vector<Object> objects;
        std::vector<Mesh*> meshes;
        std::vector<Material*> materials;

    public:
        Scene(const Device& device) {
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
};
}

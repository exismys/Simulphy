#pragma once

#include "Object.hpp"
#include "../Device.hpp"

namespace Simulphy {
class Scene {
    private:
        std::vector<Object> objects;
        Mesh* mesh = nullptr;

    public:
        Scene(const Device& device, const Texture* texture) {
            mesh = new Mesh(device);

            objects.push_back({
                .mesh = mesh,
                .texture = texture,
                .transform = Transform{
                    glm::vec3{0.0f},
                    glm::vec3{0.0f},
                    glm::vec3{1.0f},
                }
            });

            objects.push_back({
                .mesh = mesh,
                .texture = texture,
                .transform = Transform{
                    glm::vec3{2.0f, 0.0f, 0.0f},
                    glm::vec3{0.0f},
                    glm::vec3{1.0f},
                }
            });
        }

        ~Scene() {
            delete mesh;
        }

        const std::vector<Object>& getObjects() const {
            return objects;
        }

        const Mesh* getMesh() const {
            return mesh;
        }
};
}

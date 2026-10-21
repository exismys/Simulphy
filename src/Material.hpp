#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "Device.hpp"
#include "Texture.hpp"

namespace Simulphy {

class Material {
public:
    const Texture* texture = nullptr;

    Material(const Device& device, const std::string texturePath) {
        texture = new Texture(device, texturePath);   
    }

    ~Material() {
        delete texture;
    }
};

}
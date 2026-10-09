#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "Window.hpp"
#include "Instance.hpp"

namespace Simulphy {
class Surface {
    private:
        vk::raii::SurfaceKHR surface = nullptr;

    public:
        Surface(const Window& window, const Instance& instance) {
            VkSurfaceKHR _surface;
            if (!SDL_Vulkan_CreateSurface(window.get(), *instance.get(), &_surface)) {
                throw std::runtime_error("failed to create window surface!");
            }
            surface = vk::raii::SurfaceKHR(instance.get(), _surface);
        }

        const vk::raii::SurfaceKHR& get() const {
            return surface;
        }
};
}
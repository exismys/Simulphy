#pragma once

#include <SDL.h>
#include <SDL_vulkan.h>

#include <stdexcept>

namespace Simulphy {

    class Window {
        private:
            SDL_Window *window = nullptr;

        public:
            Window(uint32_t width, uint32_t height) {
                if (SDL_Init(SDL_INIT_VIDEO) != 0) {
                    throw std::runtime_error(std::string("failed to initialize SDL: ") + SDL_GetError());
                }

                window = SDL_CreateWindow(
                    "Simulphy",
                    SDL_WINDOWPOS_CENTERED,
                    SDL_WINDOWPOS_CENTERED,
                    static_cast<int>(width),
                    static_cast<int>(height),
                    SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
                );
                if (window == nullptr) {
                    throw std::runtime_error(std::string("failed to create SDL window: ") + SDL_GetError());
                }
            }

            ~Window() {
                SDL_DestroyWindow(window);
                SDL_Quit();
            }

            SDL_Window* get() const {
                return window;
            }
    };

}

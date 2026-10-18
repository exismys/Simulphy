#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <SDL.h>
#include <SDL_scancode.h>
#include <SDL_vulkan.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/geometric.hpp>

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <chrono>

#include "Window.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "Pipeline.hpp"
#include "Surface.hpp"
#include "Swapchain.hpp"
#include "Texture.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"
#include "Scene/Scene.hpp"

constexpr uint32_t WIDTH = 1200;
constexpr uint32_t HEIGHT = 720;

class Application {
    public:
        void run() {
            initImGui();
            mainLoop();
            cleanup();
        }

    private:
        Simulphy::Window window = Simulphy::Window(WIDTH, HEIGHT);
        Simulphy::Instance instance = Simulphy::Instance(window);
        Simulphy::Surface surface = Simulphy::Surface(window, instance);
        Simulphy::Device device = Simulphy::Device(window, instance, surface);
        Simulphy::Swapchain swapchain = Simulphy::Swapchain(surface, device, window);
        Simulphy::Pipeline pipeline = Simulphy::Pipeline(device, swapchain);

        Simulphy::Texture texture = Simulphy::Texture(device);
        Simulphy::Scene scene = Simulphy::Scene(device, &texture);


        Simulphy::Camera camera{
            {0, 2, 15},
            {0, 0, 0}
        };

        Simulphy::Renderer renderer = Simulphy::Renderer(device, swapchain, pipeline, texture);

        // ImGui vars
        VkFormat imguiColorFormat = VK_FORMAT_UNDEFINED;

        // Window and input control vars
        bool framebufferResized = false;
        bool windowShouldClose = false;
        bool mouseLookActive = false;
        bool relativeMouseJustEnabled = false;

        bool saveFrameRequested = false;

	    std::vector<const char *> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};

	    void processEvent(const SDL_Event &event) {
	        if (event.type == SDL_QUIT ||
	            (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) ||
	            (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
	            windowShouldClose = true;
            } else if (event.type == SDL_KEYDOWN &&
                       event.key.keysym.sym == SDLK_TAB &&
                       event.key.repeat == 0) {
                const bool enableMouseLook = !mouseLookActive;
                if (SDL_SetRelativeMouseMode(enableMouseLook ? SDL_TRUE : SDL_FALSE) != 0) {
                    throw std::runtime_error(std::string("failed to set relative mouse mode: ") + SDL_GetError());
                }
                mouseLookActive = enableMouseLook;
                if (mouseLookActive) {
                    int relX = 0;
                    int relY = 0;
                    SDL_GetRelativeMouseState(&relX, &relY);
                    relativeMouseJustEnabled = true;
                }
	        } else if (event.type == SDL_WINDOWEVENT &&
	                   (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
	                    event.window.event == SDL_WINDOWEVENT_RESIZED ||
	                    event.window.event == SDL_WINDOWEVENT_MINIMIZED ||
	                    event.window.event == SDL_WINDOWEVENT_RESTORED)) {
	            framebufferResized = true;
	        }
	    }

        void mainLoop() {
            auto previousFrameTime = std::chrono::steady_clock::now();
            while (!windowShouldClose) {
                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                    ImGui_ImplSDL2_ProcessEvent(&event);
                    processEvent(event);
                }

                const auto currentFrameTime = std::chrono::steady_clock::now();
                const float frameTime = std::chrono::duration<float>(currentFrameTime - previousFrameTime).count();
                previousFrameTime = currentFrameTime;

                const Uint8* keys = SDL_GetKeyboardState(nullptr);

                handleCameraMovement(keys, std::min(frameTime, 0.1f));
                handleRelativeMouseMovement();
                log(keys);
                if (windowShouldClose) {
                    break;
                }
                bool result = renderer.drawFrame(saveFrameRequested, framebufferResized, camera, scene);
                if (saveFrameRequested) saveFrameRequested = false;
                if (framebufferResized) framebufferResized = false;
                if (!result) {
                    recreateSwapChain();
                }
            }
            device.getLogicalDevice().waitIdle();
        }

        void recreateSwapChain() {
            int width = 0, height = 0;
            SDL_Vulkan_GetDrawableSize(window.get(), &width, &height);
            while ((width == 0 || height == 0) && !windowShouldClose) {
                SDL_Event event;
                if (!SDL_WaitEvent(&event)) {
                    throw std::runtime_error(std::string("failed to wait for SDL event: ") + SDL_GetError());
                }
                ImGui_ImplSDL2_ProcessEvent(&event);
                processEvent(event);
                while (SDL_PollEvent(&event)) {
                    ImGui_ImplSDL2_ProcessEvent(&event);
                    processEvent(event);
                }
                SDL_Vulkan_GetDrawableSize(window.get(), &width, &height);
            }
            if (windowShouldClose) {
                return;
            }

            device.getLogicalDevice().waitIdle();
            ImGui_ImplVulkan_Shutdown();

            swapchain.cleanupSwapchain();

            swapchain = Simulphy::Swapchain(surface, device, window); // reinitialize swapchain
            renderer.createColorResources();
            renderer.createDepthResources();
            initImGuiVulkan();
	    }

        void cleanup() {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext();
            swapchain.cleanupSwapchain();
        }

        void initImGui() {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = nullptr;

            ImGui::StyleColorsDark();

            if (!ImGui_ImplSDL2_InitForVulkan(window.get())) {
                throw std::runtime_error("failed to initialize ImGui SDL2 backend!");
            }
            initImGuiVulkan();
        }

        void initImGuiVulkan() {
            imguiColorFormat = static_cast<VkFormat>(swapchain.getSwapchainSurfaceFormat().format);
            ImGui_ImplVulkan_InitInfo initInfo{};
            initInfo.ApiVersion = VK_API_VERSION_1_3;
            initInfo.Instance = static_cast<VkInstance>(*instance.get());
            initInfo.PhysicalDevice = static_cast<VkPhysicalDevice>(*device.getPhysicalDevice());
            initInfo.Device = static_cast<VkDevice>(*device.getLogicalDevice());
            initInfo.QueueFamily = device.getQueueIndex();
            initInfo.Queue = static_cast<VkQueue>(*device.getGraphicsQueue());
            initInfo.DescriptorPoolSize = 1000;
            initInfo.MinImageCount = 2;
            initInfo.ImageCount = static_cast<uint32_t>(swapchain.getSwapchainImages().size());
            initInfo.MSAASamples = static_cast<VkSampleCountFlagBits>(device.getMsaaSampleCount());
            initInfo.UseDynamicRendering = true;
            initInfo.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
            initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &imguiColorFormat;
            initInfo.PipelineRenderingCreateInfo.depthAttachmentFormat = static_cast<VkFormat>(pipeline.getDepthFormat());

            if (!ImGui_ImplVulkan_Init(&initInfo)) {
                throw std::runtime_error("failed to initialize ImGui Vulkan backend!");
            }
        }

        void handleRelativeMouseMovement() {
            if (!mouseLookActive) {
                return;
            }

            int relX = 0;
            int relY = 0;
            SDL_GetRelativeMouseState(&relX, &relY);

            if (relativeMouseJustEnabled) {
                relX = 0;
                relY = 0;
                relativeMouseJustEnabled = false;
            }

            constexpr float sensitivity = 0.002f;
            camera.yaw -= static_cast<float>(relX) * sensitivity;
            // std::cout << "cameraYaw: " << cameraYaw << std::endl;
            camera.pitch -= static_cast<float>(relY) * sensitivity;
            camera.pitch = std::clamp(camera.pitch, -1.5f, 1.5f);
            // std::cout << "cameraPitch: " << cameraPitch << std::endl;
        }

        void handleCameraMovement(const Uint8* keys, float deltaTime) {
            const glm::vec3 forwardOnGround(std::sin(camera.yaw), 0.0f, std::cos(camera.yaw));
            const glm::vec3 rightOnGround(-std::cos(camera.yaw), 0.0f, std::sin(camera.yaw));
            glm::vec3 movement(0.0f);

            if (keys[SDL_SCANCODE_W]) movement += forwardOnGround;
            if (keys[SDL_SCANCODE_S]) movement -= forwardOnGround;
            if (keys[SDL_SCANCODE_A]) movement -= rightOnGround;
            if (keys[SDL_SCANCODE_D]) movement += rightOnGround;
 

            constexpr float movementSpeed = 5.0f;
            if (glm::dot(movement, movement) > 1.0f) {
                movement = glm::normalize(movement);
            }
            camera.position += movement * movementSpeed * deltaTime;
            if (keys[SDL_SCANCODE_SPACE]) {
                camera.position.y += movementSpeed * deltaTime;
            }
        }

        void log(const Uint8* keys) {
            if (keys[SDL_SCANCODE_O]) {
                std::cout << "Camera yaw: " << camera.yaw << " | Camera pitch: " << camera.pitch << std::endl; 
            }
        }
};

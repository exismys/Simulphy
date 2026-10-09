#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <iostream>

#include "Window.hpp"

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

const std::vector<char const*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

namespace Simulphy {

    class Instance {
        private:
            vk::raii::Context context;
            vk::raii::Instance instance = nullptr;
            vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

        public:
            Instance(const Window& window) {
                //----------------------------------------------------------------------
                // Check for supported layers.
                // Vulkan loader creates the following call chain:
            
                // application -> vulkan loader -> validation layer -> vulkan driver
                //----------------------------------------------------------------------
                std::vector<char const*> requiredLayers;
                if (enableValidationLayers) {
                    requiredLayers.assign(validationLayers.begin(), validationLayers.end());
                }

                auto layerProperties = context.enumerateInstanceLayerProperties();
                auto unsupportedLayerIt = std::ranges::find_if(requiredLayers, [&layerProperties](auto const &requiredLayer) {
                    return std::ranges::none_of(layerProperties, [requiredLayer](auto const &layerProperty) {
                        return strcmp(layerProperty.layerName, requiredLayer) == 0;
                    });
                });

                if (unsupportedLayerIt != requiredLayers.end()) {
                    throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
                }
                //----------------------------------------------------------------------

                //----------------------------------------------------------------------
                // Check for supported extensions

                // application -> vulkan loader -> vulkan driver -> gpu
                //                                      |
                //                                      +-- core vulkan api
                //                                      |
                //                                      +-- VK_EXT_debug_utils
                //                                      |
                //                                      `-- other extensions
                //----------------------------------------------------------------------
                auto requiredExtensions = getRequiredInstanceExtensions(window);

                auto extensionProperties = context.enumerateInstanceExtensionProperties();
                auto unsupportedPropertyIt = std::ranges::find_if(requiredExtensions, [&extensionProperties](auto const &requiredExtension) {
                    return std::ranges::none_of(extensionProperties, [requiredExtension](auto const &extensionProperty) {
                        return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
                    });
                });
                //----------------------------------------------------------------------

                if (unsupportedPropertyIt != requiredExtensions.end()) {
                    throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
                }

                constexpr vk::ApplicationInfo appInfo{
                    .pApplicationName   = "Simulphy",
                    .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
                    .pEngineName        = "No Engine",
                    .engineVersion      = VK_MAKE_VERSION( 1, 0, 0 ),
                    .apiVersion         = vk::ApiVersion14
                };

                vk::InstanceCreateInfo createInfo{
                    .pApplicationInfo        = &appInfo,
                    .enabledLayerCount       = static_cast<uint32_t>(requiredLayers.size()),
                    .ppEnabledLayerNames     = requiredLayers.data(),
                    .enabledExtensionCount   = static_cast<uint32_t>(requiredExtensions.size()),
                    .ppEnabledExtensionNames = requiredExtensions.data()
                };

                instance = vk::raii::Instance(context, createInfo);
            }

            const vk::raii::Instance& get() const {
                return instance;
            }
            
            static std::vector<const char*> getRequiredInstanceExtensions(const Window& window) {
                unsigned int extensionCount = 0;
                if (!SDL_Vulkan_GetInstanceExtensions(window.get(), &extensionCount, nullptr)) {
                    throw std::runtime_error(std::string("Failed to get SDL Vulkan extension count: ") + SDL_GetError());
                }
                std::vector<const char*> extensions(extensionCount);
                if (!SDL_Vulkan_GetInstanceExtensions(window.get(), &extensionCount, extensions.data())) {
                    throw std::runtime_error(std::string("Failed to get SDL Vulkan extensions: ") + SDL_GetError());
                }
                if (enableValidationLayers) {
                    extensions.push_back(vk::EXTDebugUtilsExtensionName);
                }

                return extensions;
            }

            static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
                                                                  vk::DebugUtilsMessageTypeFlagsEXT              type,
                                                                  const vk::DebugUtilsMessengerCallbackDataEXT  *pCallbackData,
                                                                  void                                          *pUserData) {

                if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning ||
                    severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError) {
                    std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
                }

                return vk::False;
            }

            void setupDebugMessenger() {
                if (!enableValidationLayers) return;

                vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | 
                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
                );
                
                vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
                    vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | 
                    vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | 
                    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                );

                vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
                    .messageSeverity = severityFlags,
                    .messageType     = messageTypeFlags,
                    .pfnUserCallback = &debugCallback
                };

                debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
            }
    };

}

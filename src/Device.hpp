#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <algorithm>

#include "Instance.hpp"

namespace Simulphy {
    class Device {
        private:
            vk::raii::PhysicalDevice physicalDevice = nullptr;
            vk::SampleCountFlagBits msaaSampleCount = vk::SampleCountFlagBits::e1;

            std::vector<const char *> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};

        public:
            Device(const Instance& instance) {
                pickPhysicalDevice(instance.get());
            }

            void pickPhysicalDevice(const vk::raii::Instance& instance) {
                std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

                auto const devIter = std::ranges::find_if(physicalDevices, [&](auto const & physicalDevice) {
                    return isDeviceSuitable(physicalDevice); 
                });

                if (devIter == physicalDevices.end()) {
                    throw std::runtime_error( "Failed to find a suitable GPU!" );
                }
                physicalDevice = *devIter;
                msaaSampleCount = getMaxUsableSampleCount();
                std::cout << "MSAA sample count: " << static_cast<uint32_t>(msaaSampleCount) << '\n';
            }

            // Check for supported GPU device
            bool isDeviceSuitable(const vk::raii::PhysicalDevice& physicalDevice) {
                bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;

                //----------------------------------------------------------------------
                // Check if these is a GPU queue family which supports 
                // graphics

                // A GPU can expose different types of queue families. It
                // might look like this:

                //  Physical Device
                //  |
                //  +-- Queue Family 0
                //  |   |
                //  |   +-- Queue 0
                //  |   +-- Queue 1
                //  |   `-- Graphics + Compute + Transfer
                //  |
                //  +-- Queue Family 1
                //  |   |
                //  |   +-- Queue 0
                //  |   +-- Queue 1
                //  |   +-- Queue 2
                //  |   `-- Compute + Transfer
                //  |
                //  `-- Queue Family 2
                //      |
                //      +-- Queue 0
                //      +-- Queue 1
                //      `-- Transfer
                //----------------------------------------------------------------------
                auto queueFamilies = physicalDevice.getQueueFamilyProperties();
                bool supportsGraphics = std::ranges::any_of(queueFamilies, [](const auto& qfp) {
                    return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); 
                });
                //----------------------------------------------------------------------

                // Check if the device supports required device extensions
                auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
                bool supportsAllRequiredExtensions = std::ranges::all_of( requiredDeviceExtension,[&availableDeviceExtensions](auto const & requiredDeviceExtension) {
                    return std::ranges::any_of(availableDeviceExtensions, [requiredDeviceExtension](auto const & availableDeviceExtension) { 
                        return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; 
                    });
                });

                // Check if the device supports required features
                auto features = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
                                                                    vk::PhysicalDeviceVulkan11Features,
                                                                    vk::PhysicalDeviceVulkan13Features,
                                                                    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

                bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy &&
                                                features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                                                features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                                                features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
                                                features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

                return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
            }

            vk::SampleCountFlagBits getMaxUsableSampleCount() {
                // vk::PhysicalDeviceProperties physicalDeviceProperties = physicalDevice.getProperties();

                // vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
                // if (counts & vk::SampleCountFlagBits::e64) {
                //     return vk::SampleCountFlagBits::e64;
                // }
                // if (counts & vk::SampleCountFlagBits::e32) {
                //     return vk::SampleCountFlagBits::e32;
                // }
                // if (counts & vk::SampleCountFlagBits::e16) {
                //     return vk::SampleCountFlagBits::e16;
                // }
                // if (counts & vk::SampleCountFlagBits::e8) {
                //     return vk::SampleCountFlagBits::e8;
                // }
                // if (counts & vk::SampleCountFlagBits::e4) {
                //     return vk::SampleCountFlagBits::e4;
                // }
                // if (counts & vk::SampleCountFlagBits::e2) {
                //     return vk::SampleCountFlagBits::e2;
                // }

                return vk::SampleCountFlagBits::e2;
	        }
    };
}

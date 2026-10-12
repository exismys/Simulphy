#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <algorithm>

#include "Instance.hpp"
#include "Window.hpp"
#include "Surface.hpp"

namespace Simulphy {
class Device {
    private:
        vk::raii::PhysicalDevice physicalDevice = nullptr;
        vk::SampleCountFlagBits msaaSampleCount = vk::SampleCountFlagBits::e1;

        uint32_t queueIndex = ~0;
        vk::raii::Device device = nullptr;
        vk::raii::Queue graphicsQueue = nullptr;

        vk::raii::CommandPool commandPool = nullptr;

        std::vector<const char *> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};

    public:
        Device(const Window& window, const Instance& instance, const Surface& surface) {
            pickPhysicalDevice(instance);
            createLogicalDevice(surface);
            createCommandPool();
        }

        const vk::raii::PhysicalDevice& getPhysicalDevice() const {
            return physicalDevice;
        }

        const vk::raii::Device& getLogicalDevice() const {
            return device;
        }

        const vk::SampleCountFlagBits getMsaaSampleCount() const {
            return msaaSampleCount;
        }

        const uint32_t getQueueIndex() const {
            return queueIndex;
        }

        const vk::raii::Queue& getGraphicsQueue() const {
            return graphicsQueue;
        }

        const vk::raii::CommandPool& getCommandPool() const {
            return commandPool;
        }

        void pickPhysicalDevice(const Instance& instance) {
            std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.get().enumeratePhysicalDevices();

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

        vk::SampleCountFlagBits getMaxUsableSampleCount() const {
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

        void createLogicalDevice(const Surface& surface) {
            // Get queue family index which supports graphics and surface
            std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

            for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++) {
                if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) && 
                    physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface.get())) {
                    queueIndex = qfpIndex;
                    break;
                }
            }
            if (queueIndex == ~0) {
                throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
            }

            // Create queue and logical device with required features
            vk::StructureChain<vk::PhysicalDeviceFeatures2,
                            vk::PhysicalDeviceVulkan11Features,
                            vk::PhysicalDeviceVulkan13Features,
                            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
                {.features = {.samplerAnisotropy = true}},
                {.shaderDrawParameters = true},
                {.synchronization2 = true, .dynamicRendering = true},
                {.extendedDynamicState = true}
            };

            float queuePriority = 0.5f;
            
            vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
                .queueFamilyIndex = queueIndex,
                .queueCount = 1, // only one queue in the family with priority "queuePriority"
                .pQueuePriorities = &queuePriority
            };

            vk::DeviceCreateInfo deviceCreateInfo{
                .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
                .queueCreateInfoCount = 1,
                .pQueueCreateInfos = &deviceQueueCreateInfo,
                .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
                .ppEnabledExtensionNames = requiredDeviceExtension.data()
            };

            device = vk::raii::Device(physicalDevice, deviceCreateInfo);
            graphicsQueue = vk::raii::Queue(device, queueIndex, 0);
        }

        vk::raii::ImageView createImageView(const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels) const {
            vk::ImageViewCreateInfo viewInfo{
                .image            = image,
                .viewType         = vk::ImageViewType::e2D,
                .format           = format,
                .subresourceRange = {.aspectMask = aspectFlags, .baseMipLevel = 0, .levelCount = mipLevels, .baseArrayLayer = 0, .layerCount = 1}
            };
            return vk::raii::ImageView(device, viewInfo);
        }

        void createCommandPool() {
            vk::CommandPoolCreateInfo poolInfo{
                .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                .queueFamilyIndex = queueIndex
            };
            commandPool = vk::raii::CommandPool(device, poolInfo);
	    }

        std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, 
                                                                         vk::BufferUsageFlags usage, 
                                                                         vk::MemoryPropertyFlags properties) const {
            vk::BufferCreateInfo bufferInfo{
                .size        = size, 
                .usage       = usage, 
                .sharingMode = vk::SharingMode::eExclusive
            };
            vk::raii::Buffer buffer = vk::raii::Buffer(device, bufferInfo);

            vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
            vk::MemoryAllocateInfo allocInfo{
                .allocationSize  = memRequirements.size, 
                .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
            };
            vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(device, allocInfo);

            buffer.bindMemory(*bufferMemory, 0);
            return {std::move(buffer), std::move(bufferMemory)};
	    }

        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const {
            vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if (
                    (typeFilter & (1 << i)) && 
                    (memProperties.memoryTypes[i].propertyFlags & properties) == properties
                ) {
                    return i;
                }
            }

            throw std::runtime_error("failed to find suitable memory type!");
	    }

        std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(uint32_t width, 
                                                                       uint32_t height, 
                                                                       uint32_t mipLevels,
                                                                       vk::SampleCountFlagBits numSamples,
                                                                       vk::Format format, 
                                                                       vk::ImageTiling tiling, 
                                                                       vk::ImageUsageFlags usage, 
                                                                       vk::MemoryPropertyFlags properties) const {
            vk::ImageCreateInfo imageInfo{
                .imageType     = vk::ImageType::e2D,
                .format        = format,
                .extent        = {width, height, 1},
                .mipLevels     = mipLevels,
                .arrayLayers   = 1,
                .samples       = numSamples,
                .tiling        = tiling,
                .usage         = usage,
                .sharingMode   = vk::SharingMode::eExclusive,
                .initialLayout = vk::ImageLayout::eUndefined
            };
            vk::raii::Image image = vk::raii::Image(device, imageInfo);

            vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
            vk::MemoryAllocateInfo allocInfo{
                .allocationSize  = memRequirements.size,
                .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
            };
            vk::raii::DeviceMemory imageMemory = vk::raii::DeviceMemory(device, allocInfo);
            
            image.bindMemory(imageMemory, 0);

            return {std::move(image), std::move(imageMemory)};
	    }
};
}

#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "Surface.hpp"
#include "Device.hpp"

namespace Simulphy {
    class Swapchain {
        private:
            vk::Extent2D swapChainExtent;
            vk::SurfaceFormatKHR swapChainSurfaceFormat;
            bool canCaptureFrame = false;
            vk::raii::SwapchainKHR swapChain = nullptr;
            std::vector<vk::Image> swapChainImages;
            std::vector<vk::raii::ImageView> swapChainImageViews;

        public:
            Swapchain(const Surface& surface, const Device& device, const Window& window) {
                createSwapChain(surface, device, window);
                createImageViews(device);
            }

            void createSwapChain(const Surface& surface, const Device& device, const Window& window) {
                vk::SurfaceCapabilitiesKHR surfaceCapabilities = device.getPhysicalDevice().getSurfaceCapabilitiesKHR(*surface.get());
                swapChainExtent = chooseSwapExtent(surfaceCapabilities, window);

                uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);

                std::vector<vk::SurfaceFormatKHR> availableFormats = device.getPhysicalDevice().getSurfaceFormatsKHR(*surface.get());
                swapChainSurfaceFormat = chooseSwapSurfaceFormat(availableFormats);

                const bool hasSupportedPixelFormat = swapChainSurfaceFormat.format == vk::Format::eB8G8R8A8Srgb ||
                                                     swapChainSurfaceFormat.format == vk::Format::eB8G8R8A8Unorm ||
                                                     swapChainSurfaceFormat.format == vk::Format::eR8G8B8A8Srgb ||
                                                     swapChainSurfaceFormat.format == vk::Format::eR8G8B8A8Unorm;

                const auto formatProperties = device.getPhysicalDevice().getFormatProperties(swapChainSurfaceFormat.format);
                canCaptureFrame = (surfaceCapabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferSrc) &&
                                (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eTransferSrc) &&
                                hasSupportedPixelFormat;

                std::vector<vk::PresentModeKHR> availablePresentModes = device.getPhysicalDevice().getSurfacePresentModesKHR(*surface.get());
                vk::PresentModeKHR presentMode = chooseSwapPresentMode(availablePresentModes);

                vk::SwapchainCreateInfoKHR swapChainCreateInfo{
                    .surface          = *surface.get(),
                    .minImageCount    = minImageCount,
                    .imageFormat      = swapChainSurfaceFormat.format,
                    .imageColorSpace  = swapChainSurfaceFormat.colorSpace,
                    .imageExtent      = swapChainExtent,
                    .imageArrayLayers = 1,
                    .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment |
                                        (canCaptureFrame ? vk::ImageUsageFlagBits::eTransferSrc : vk::ImageUsageFlags{}),
                    .imageSharingMode = vk::SharingMode::eExclusive,
                    .preTransform     = surfaceCapabilities.currentTransform,
                    .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
                    .presentMode      = presentMode,
                    .clipped          = true
                };

                swapChain = vk::raii::SwapchainKHR(device.getLogicalDevice(), swapChainCreateInfo);
                swapChainImages = swapChain.getImages();
            }

            vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const Window& window){
                if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                    return capabilities.currentExtent;
                }

                int width, height;
                SDL_Vulkan_GetDrawableSize(window.get(), &width, &height);

                return {
                    std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
                    std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
                };
            }

            uint32_t chooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities) {
                auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
                if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount)) {
                    minImageCount = surfaceCapabilities.maxImageCount;
                }
                return minImageCount;
            }

            vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
                assert(!availableFormats.empty());
                const auto formatIt = std::ranges::find_if(availableFormats, [](const auto &format) { 
                    return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; 
                });
                return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
            }

            vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
                assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { 
                    return presentMode == vk::PresentModeKHR::eFifo;
                }));
                return std::ranges::any_of(availablePresentModes, [](const vk::PresentModeKHR value) { 
                    return vk::PresentModeKHR::eMailbox == value;
                }) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
            }

            void createImageViews(const Device& device) {
                assert(swapChainImageViews.empty());

                swapChainImageViews.reserve(swapChainImages.size());
                for (auto &image : swapChainImages) {
                    swapChainImageViews.emplace_back(device.createImageView(image, swapChainSurfaceFormat.format, vk::ImageAspectFlagBits::eColor, 1));
                }
            }
    };
}

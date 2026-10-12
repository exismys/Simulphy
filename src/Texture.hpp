#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <stb_image.h>

#include <cmath>

#include "Device.hpp"
#include "Utilities.hpp"

const std::string TEXTURE_PATH = "textures/test-textures/Menlogate.jpg";

namespace Simulphy {
class Texture {
    private:
        uint32_t mipLevels = 0;
        vk::raii::Image textureImage = nullptr;
	    vk::raii::DeviceMemory textureImageMemory = nullptr;
        vk::raii::ImageView textureImageView = nullptr;
        vk::raii::Sampler textureSampler = nullptr;

    public:
        Texture(const Device& device) {
            createTextureImage(device);
            createTextureImageView(device);
            createTextureSampler(device);
        }

        const vk::raii::ImageView& getTextureImageView() const {
            return textureImageView;
        }

        const vk::raii::Sampler& getTextureSampler() const {
            return textureSampler;
        }

        void createTextureImage(const Device& device) {
            int texWidth, texHeight, texChannels;
            stbi_uc *pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            vk::DeviceSize imageSize = texWidth * texHeight * 4;
            mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

            if (!pixels) {
                throw std::runtime_error("failed to load texture image!");
            }

            auto [stagingBuffer, stagingBufferMemory] = device.createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

            void *data = stagingBufferMemory.mapMemory(0, imageSize);
            memcpy(data, pixels, imageSize);
            stagingBufferMemory.unmapMemory();

            stbi_image_free(pixels);

            std::tie(textureImage, textureImageMemory) = device.createImage(texWidth,
                                                                              texHeight,
                                                                              mipLevels,
                                                                              vk::SampleCountFlagBits::e1,
                                                                              vk::Format::eR8G8B8A8Srgb,
                                                                              vk::ImageTiling::eOptimal,
                                                                              vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                                                                              vk::MemoryPropertyFlagBits::eDeviceLocal);

            vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands(device);
            transitionImageLayout(commandBuffer, textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
            copyBufferToImage(commandBuffer, stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
            generateMipmaps(device, commandBuffer, textureImage, vk::Format::eR8G8B8A8Srgb, texWidth, texHeight, mipLevels);
            // transitionImageLayout(commandBuffer, textureImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, mipLevels);
            endSingleTimeCommands(std::move(commandBuffer), device);
	    }

        void createTextureImageView(const Device& device) {
		    textureImageView = device.createImageView(*textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, mipLevels);
	    }

        void createTextureSampler(const Device& device) {
            vk::PhysicalDeviceProperties properties = device.getPhysicalDevice().getProperties();
            vk::SamplerCreateInfo samplerInfo{
                .magFilter        = vk::Filter::eLinear,
                .minFilter        = vk::Filter::eLinear,
                .mipmapMode       = vk::SamplerMipmapMode::eLinear,
                .addressModeU     = vk::SamplerAddressMode::eRepeat,
                .addressModeV     = vk::SamplerAddressMode::eRepeat,
                .addressModeW     = vk::SamplerAddressMode::eRepeat,
                .mipLodBias       = 0.0f,
                .anisotropyEnable = vk::True,
                .maxAnisotropy    = properties.limits.maxSamplerAnisotropy,
                .compareEnable    = vk::False,
                .compareOp        = vk::CompareOp::eAlways,
                .minLod           = 0.0f,
		        .maxLod           = vk::LodClampNone
            };
            textureSampler = vk::raii::Sampler(device.getLogicalDevice(), samplerInfo);
	    }

        void generateMipmaps(const Device& device,
                             vk::raii::CommandBuffer &commandBuffer,
	                         vk::raii::Image         &image,
                             vk::Format               imageFormat,
                             int32_t                  texWidth,
                             int32_t                  texHeight,
                             uint32_t                 mipLevels) {

		    vk::FormatProperties formatProperties = device.getPhysicalDevice().getFormatProperties(imageFormat);
            if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
                throw std::runtime_error("texture image format does not support linear blitting!");
            }

            vk::ImageMemoryBarrier barrier = {
                .srcAccessMask       = vk::AccessFlagBits::eTransferWrite,
                .dstAccessMask       = vk::AccessFlagBits::eTransferRead,
                .oldLayout           = vk::ImageLayout::eTransferDstOptimal,
                .newLayout           = vk::ImageLayout::eTransferSrcOptimal,
                .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
                .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
                .image               = image,
                .subresourceRange    = {.aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1}
            };

            int32_t mipWidth = texWidth;
            int32_t mipHeight = texHeight;

            for (uint32_t i = 1; i < mipLevels; i++) {
                barrier.subresourceRange.baseMipLevel = i - 1;
                barrier.oldLayout                     = vk::ImageLayout::eTransferDstOptimal;
                barrier.newLayout                     = vk::ImageLayout::eTransferSrcOptimal;
                barrier.srcAccessMask                 = vk::AccessFlagBits::eTransferWrite;
                barrier.dstAccessMask                 = vk::AccessFlagBits::eTransferRead;

                commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);

                vk::ImageBlit blit = {
                    .srcSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = i - 1, .layerCount = 1},
                    .srcOffsets     = std::array<vk::Offset3D, 2>({{}, {mipWidth, mipHeight, 1}}),
                    .dstSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = i, .layerCount = 1},
                    .dstOffsets     = std::array<vk::Offset3D, 2>({{}, {1 < mipWidth ? mipWidth / 2 : 1, 1 < mipHeight ? mipHeight / 2 : 1, 1}})
                };

                commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

                barrier.oldLayout     = vk::ImageLayout::eTransferSrcOptimal;
                barrier.newLayout     = vk::ImageLayout::eShaderReadOnlyOptimal;
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
                barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

                commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);

                if (1 < mipWidth) {
                    mipWidth /= 2;
                }
                if (1 < mipHeight) {
                    mipHeight /= 2;
                }
            }

            barrier.subresourceRange.baseMipLevel = mipLevels - 1;
            barrier.oldLayout                     = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout                     = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask                 = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask                 = vk::AccessFlagBits::eShaderRead;

		    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);
	    }
};
}

#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <vector>

#include "Device.hpp"

namespace Simulphy {
std::vector<char> readFile(const std::string &filename);

vk::raii::CommandBuffer beginSingleTimeCommands(const Device& device);
void endSingleTimeCommands(vk::raii::CommandBuffer &&commandBuffer, const Device& device);

void copyBuffer(const Device& device, vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size);
void copyBufferToImage(vk::raii::CommandBuffer &commandBuffer, const vk::raii::Buffer &buffer, vk::raii::Image &image, uint32_t width, uint32_t height);

void transitionImageLayout(vk::raii::CommandBuffer &commandBuffer, const vk::raii::Image &image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);

std::string formatNumberLocale(int number);
}

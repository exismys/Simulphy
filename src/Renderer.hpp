#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <stb_image_write.h>

#include <chrono>

#include "Device.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "Texture.hpp"
#include "ImGuiLayer.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

namespace Simulphy {
struct UniformBufferObject{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class Renderer {
    private:
        const Device& device;
        const Swapchain& swapchain;
        const Pipeline& pipeline;

        vk::raii::Image colorImage = nullptr;
        vk::raii::DeviceMemory colorImageMemory = nullptr;
        vk::raii::ImageView colorImageView = nullptr;

        vk::raii::Image depthImage = nullptr;
        vk::raii::DeviceMemory depthImageMemory = nullptr;
        vk::raii::ImageView depthImageView = nullptr;

        std::vector<vk::raii::Buffer> uniformBuffers;
        std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
        std::vector<void *> uniformBuffersMapped;

        vk::raii::DescriptorPool descriptorPool = nullptr;
	    std::vector<vk::raii::DescriptorSet> descriptorSets;

        std::vector<vk::raii::CommandBuffer> commandBuffers;

        std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
        std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
        std::vector<vk::raii::Fence> inFlightFences;

        uint32_t frameIndex = 0;

        bool canCaptureFrame = true;

    public:
        Renderer(const Device& device, const Swapchain& swapchain, const Pipeline& pipeline, const Texture& texture): device(device), swapchain(swapchain), pipeline(pipeline) {
            createColorResources();
            createDepthResources();
            createUniformBuffers(device);
            createDescriptorPool();
            createDescriptorSets(texture);
            createCommandBuffer();
            createSyncObjects();
        }

        void createColorResources() {
            vk::Format colorFormat = swapchain.getSwapchainSurfaceFormat().format;

            std::tie(colorImage, colorImageMemory) = device.createImage(swapchain.getSwapchainExtent().width,
                                                                        swapchain.getSwapchainExtent().height,
                                                                        1,
                                                                        device.getMaxUsableSampleCount(),
                                                                        colorFormat,
                                                                        vk::ImageTiling::eOptimal,
                                                                        vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
                                                                        vk::MemoryPropertyFlagBits::eDeviceLocal);
            colorImageView = device.createImageView(colorImage, colorFormat, vk::ImageAspectFlagBits::eColor, 1);
	    }

        void createDepthResources() {
            vk::Format depthFormat = pipeline.getDepthFormat();

            std::tie(depthImage, depthImageMemory) = device.createImage(swapchain.getSwapchainExtent().width,
                                                                          swapchain.getSwapchainExtent().height,
                                                                          1,
                                                                          device.getMaxUsableSampleCount(),
                                                                          depthFormat, 
                                                                          vk::ImageTiling::eOptimal, 
                                                                          vk::ImageUsageFlagBits::eDepthStencilAttachment, 
                                                                          vk::MemoryPropertyFlagBits::eDeviceLocal);
            depthImageView = device.createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, 1);
	    }

        void createUniformBuffers(const Device& device) {
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
                auto [buffer, bufferMem] = device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
                uniformBuffers.emplace_back(std::move(buffer));
                uniformBuffersMemory.emplace_back(std::move(bufferMem));
                uniformBuffersMapped.emplace_back(uniformBuffersMemory.back().mapMemory(0, bufferSize));
            }
	    }

        void createDescriptorPool() {
            std::array<vk::DescriptorPoolSize, 2> poolSize{
                {
                    {
                        .type = vk::DescriptorType::eUniformBuffer, 
                        .descriptorCount = MAX_FRAMES_IN_FLIGHT
                    }, 
                    {
                        .type = vk::DescriptorType::eCombinedImageSampler,
                        .descriptorCount = MAX_FRAMES_IN_FLIGHT
                    }
                }
            };
            vk::DescriptorPoolCreateInfo poolInfo{
                .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 
                .maxSets = MAX_FRAMES_IN_FLIGHT, 
                .poolSizeCount = static_cast<uint32_t>(poolSize.size()), 
                .pPoolSizes = poolSize.data()
            };
            descriptorPool = vk::raii::DescriptorPool(device.getLogicalDevice(), poolInfo);
	    }

        //----------------------------------------------------------------------
        //      Descriptor Sets
        //            |
        //            +-- Descriptor Set 0 (0th frame in flight)
        //            |           |
        //            |           +-- Descriptor 0 (binding 0, uniform buffer)
        //            |           `-- Descriptor 1 (binding 1, image sampler)
        //            `-- Descriptor Set 1 (1st frame in flight)
        //----------------------------------------------------------------------
        void createDescriptorSets(const Texture& texture) {
            std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *pipeline.getDescriptorSetLayout());
            vk::DescriptorSetAllocateInfo allocInfo{
                .descriptorPool     = descriptorPool,
                .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
                .pSetLayouts        = layouts.data()
            };

            descriptorSets = device.getLogicalDevice().allocateDescriptorSets(allocInfo);

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vk::DescriptorBufferInfo bufferInfo{
                    .buffer = uniformBuffers[i], 
                    .offset = 0, 
                    .range  = sizeof(UniformBufferObject)
                };
                vk::DescriptorImageInfo imageInfo{
                    .sampler     = texture.getTextureSampler(), 
                    .imageView   = texture.getTextureImageView(), 
                    .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
                };
                std::array<vk::WriteDescriptorSet, 2> descriptorWrites{
                    {
                        {
                            .dstSet          = descriptorSets[i],
                            .dstBinding      = 0,
                            .dstArrayElement = 0,
                            .descriptorCount = 1,
                            .descriptorType  = vk::DescriptorType::eUniformBuffer,
                            .pBufferInfo     = &bufferInfo

                        },
                        {
                            .dstSet          = descriptorSets[i],
                            .dstBinding      = 1,
                            .dstArrayElement = 0,
                            .descriptorCount = 1,
                            .descriptorType  = vk::DescriptorType::eCombinedImageSampler,
                            .pImageInfo      = &imageInfo
                        }
                    }

                };
                device.getLogicalDevice().updateDescriptorSets(descriptorWrites, {});
            }
	    }
        
        void createCommandBuffer() {
            vk::CommandBufferAllocateInfo allocInfo{
                .commandPool        = device.getCommandPool(), 
                .level              = vk::CommandBufferLevel::ePrimary, 
                .commandBufferCount = MAX_FRAMES_IN_FLIGHT
            };
            commandBuffers = vk::raii::CommandBuffers(device.getLogicalDevice(), allocInfo);
	    }

        void createSyncObjects(){
            assert(presentCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && inFlightFences.empty());

            for (size_t i = 0; i < swapchain.getSwapchainImages().size(); i++) {
                renderFinishedSemaphores.emplace_back(device.getLogicalDevice(), vk::SemaphoreCreateInfo());
            }

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                presentCompleteSemaphores.emplace_back(device.getLogicalDevice(), vk::SemaphoreCreateInfo());
                inFlightFences.emplace_back(device.getLogicalDevice(), vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
            }
        }

        bool drawFrame(bool saveFrameRequested, bool framebufferResized, const Camera& camera, const Mesh& mesh) {
            auto fenceResult = device.getLogicalDevice().waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);
            if (fenceResult != vk::Result::eSuccess) {
                throw std::runtime_error("failed to wait for fence!");
            }

            // Acquire swapchain image
		    auto [result, imageIndex] = swapchain.getSwapchain().acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);

            if (result == vk::Result::eErrorOutOfDateKHR) {
                return false;
		    }

		    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
                assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
                throw std::runtime_error("failed to acquire swap chain image!");
		    }

            updateUniformBuffer(frameIndex, camera);
            renderImGui(canCaptureFrame, saveFrameRequested, mesh.getVertices().size(), mesh.getIndices().size());

            // Create capture buffer and memory for current frame
            const bool captureFrame = saveFrameRequested && canCaptureFrame;
            vk::raii::Buffer captureBuffer = nullptr;
            vk::raii::DeviceMemory captureBufferMemory = nullptr;
            if (captureFrame) {
                const vk::DeviceSize captureSize = static_cast<vk::DeviceSize>(swapchain.getSwapchainExtent().width) * swapchain.getSwapchainExtent().height * 4;
                std::tie(captureBuffer, captureBufferMemory) = device.createBuffer(captureSize,
                                                                                     vk::BufferUsageFlagBits::eTransferDst,
                                                                                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            }

            device.getLogicalDevice().resetFences(*inFlightFences[frameIndex]);

            commandBuffers[frameIndex].reset();
		    recordCommandBuffer(imageIndex, captureFrame ? &*captureBuffer : nullptr, mesh);

		    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		    const vk::SubmitInfo   submitInfo{
                .waitSemaphoreCount   = 1,
                .pWaitSemaphores      = &*presentCompleteSemaphores[frameIndex],
                .pWaitDstStageMask    = &waitDestinationStageMask,
                .commandBufferCount   = 1,
                .pCommandBuffers      = &*commandBuffers[frameIndex],
                .signalSemaphoreCount = 1,
                .pSignalSemaphores    = &*renderFinishedSemaphores[imageIndex]
            };
		    device.getGraphicsQueue().submit(submitInfo, *inFlightFences[frameIndex]);

            // Capture current frame
            if (captureFrame) {
                auto fenceResult = device.getLogicalDevice().waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);
                if (fenceResult != vk::Result::eSuccess) {
                    throw std::runtime_error("failed to wait for captured frame!");
                }
                saveFrameToPng(captureBufferMemory);
            }

		    const vk::PresentInfoKHR presentInfoKHR{
                .waitSemaphoreCount = 1, 
                .pWaitSemaphores    = &*renderFinishedSemaphores[imageIndex], 
                .swapchainCount     = 1, 
                .pSwapchains        = &*swapchain.getSwapchain(), 
                .pImageIndices      = &imageIndex
            };

		    result = device.getGraphicsQueue().presentKHR(presentInfoKHR);

            if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR) || framebufferResized) {
                return false;  
		    } else {
			    assert(result == vk::Result::eSuccess);
            }

            frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

            return true;
        }

        void updateUniformBuffer(uint32_t currentImage, const Camera& camera) {
            static auto startTime = std::chrono::high_resolution_clock::now();
            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float>(currentTime - startTime).count();

            UniformBufferObject ubo{};
            ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) *
                        // rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
                        // rotate(glm::mat4(1.0f), time * glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
                        // rotate(glm::mat4(1.0f), /*time*/ glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
                        glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));

            const glm::vec3 cameraDirection(
                std::cos(camera.pitch) * std::sin(camera.yaw),
                std::sin(camera.pitch),
                std::cos(camera.pitch) * std::cos(camera.yaw)
            );
            ubo.view = lookAt(camera.position, camera.position + cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f));
            ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapchain.getSwapchainExtent().width) / static_cast<float>(swapchain.getSwapchainExtent().height), 0.1f, 100.0f);

            memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	    }

        void recordCommandBuffer(uint32_t imageIndex, const vk::Buffer *captureBuffer, const Mesh& mesh) {
            auto &commandBuffer = commandBuffers[frameIndex];

		    commandBuffer.begin({});

            transition_image_layout(
                swapchain.getSwapchainImages()[imageIndex],
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eColorAttachmentOptimal,
                {},
                vk::AccessFlagBits2::eColorAttachmentWrite,
                vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                vk::ImageAspectFlagBits::eColor
            );

            transition_image_layout(
                *colorImage,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eColorAttachmentOptimal,
                vk::AccessFlagBits2::eColorAttachmentWrite,
                vk::AccessFlagBits2::eColorAttachmentWrite,
                vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                vk::ImageAspectFlagBits::eColor
            );

            transition_image_layout(
                *depthImage,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eDepthAttachmentOptimal,
                vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
                vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
                vk::ImageAspectFlagBits::eDepth
            );

		    vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
            vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

            vk::RenderingAttachmentInfo colorAttachmentInfo = {
                .imageView          = colorImageView,
                .imageLayout        = vk::ImageLayout::eColorAttachmentOptimal,
                .resolveMode        = vk::ResolveModeFlagBits::eAverage,
                .resolveImageView   = swapchain.getSwapchainImageViews()[imageIndex],
                .resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                .loadOp             = vk::AttachmentLoadOp::eClear,
                .storeOp            = vk::AttachmentStoreOp::eStore,
                .clearValue         = clearColor
            };
            vk::RenderingAttachmentInfo depthAttachmentInfo = {
                .imageView   = depthImageView,
                .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
                .loadOp      = vk::AttachmentLoadOp::eClear,
                .storeOp     = vk::AttachmentStoreOp::eDontCare,
                .clearValue  = clearDepth
            };
            vk::RenderingInfo renderingInfo = {
                .renderArea           = {.offset = {0, 0}, .extent = swapchain.getSwapchainExtent()},
                .layerCount           = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments    = &colorAttachmentInfo,
                .pDepthAttachment     = &depthAttachmentInfo
            };

            // Render to swapchain image
            commandBuffer.beginRendering(renderingInfo);
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.getGraphicsPipeline());
            commandBuffer.bindVertexBuffers(0, *mesh.getVertexBuffer(), {0});
            commandBuffers[frameIndex].bindIndexBuffer(*mesh.getIndexBuffer(), 0, vk::IndexType::eUint32);
            commandBuffer.setViewport(0, vk::Viewport(0.0f, static_cast<float>(swapchain.getSwapchainExtent().height), static_cast<float>(swapchain.getSwapchainExtent().width), -static_cast<float>(swapchain.getSwapchainExtent().height), 0.0f, 1.0f));
            commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain.getSwapchainExtent()));
            commandBuffers[frameIndex].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.getPipelineLayout(), 0, *descriptorSets[frameIndex], nullptr);
            commandBuffer.drawIndexed(static_cast<uint32_t>(mesh.getIndices().size()), 1, 0, 0, 0);
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), static_cast<VkCommandBuffer>(*commandBuffer));
            commandBuffer.endRendering();

            // Copy image to the capture buffer
            // Transition image to be present
            // Transition capture buffer to be read by host
            if (captureBuffer) {
                transition_image_layout(
                    swapchain.getSwapchainImages()[imageIndex],
                    vk::ImageLayout::eColorAttachmentOptimal,
                    vk::ImageLayout::eTransferSrcOptimal,
                    vk::AccessFlagBits2::eColorAttachmentWrite,
                    vk::AccessFlagBits2::eTransferRead,
                    vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                    vk::PipelineStageFlagBits2::eTransfer,
                    vk::ImageAspectFlagBits::eColor
                );
                vk::BufferImageCopy copyRegion{
                    .imageSubresource = {
                        .aspectMask = vk::ImageAspectFlagBits::eColor,
                        .mipLevel = 0,
                        .baseArrayLayer = 0,
                        .layerCount = 1
                    },
                    .imageExtent = {swapchain.getSwapchainExtent().width, swapchain.getSwapchainExtent().height, 1}
                };
                commandBuffer.copyImageToBuffer(swapchain.getSwapchainImages()[imageIndex],
                                                vk::ImageLayout::eTransferSrcOptimal,
                                                *captureBuffer,
                                                copyRegion);
                vk::BufferMemoryBarrier2 captureBarrier{
                    .srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
                    .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
                    .dstStageMask = vk::PipelineStageFlagBits2::eHost,
                    .dstAccessMask = vk::AccessFlagBits2::eHostRead,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .buffer = *captureBuffer,
                    .offset = 0,
                    .size = VK_WHOLE_SIZE
                };
                vk::DependencyInfo captureDependency{
                    .bufferMemoryBarrierCount = 1,
                    .pBufferMemoryBarriers = &captureBarrier
                };
                commandBuffer.pipelineBarrier2(captureDependency);
                transition_image_layout(
                    swapchain.getSwapchainImages()[imageIndex],
                    vk::ImageLayout::eTransferSrcOptimal,
                    vk::ImageLayout::ePresentSrcKHR,
                    vk::AccessFlagBits2::eTransferRead,
                    {},
                    vk::PipelineStageFlagBits2::eTransfer,
                    vk::PipelineStageFlagBits2::eBottomOfPipe,
                    vk::ImageAspectFlagBits::eColor
                );
            } else {
                transition_image_layout(
                    swapchain.getSwapchainImages()[imageIndex],
                    vk::ImageLayout::eColorAttachmentOptimal,
                    vk::ImageLayout::ePresentSrcKHR,
                    vk::AccessFlagBits2::eColorAttachmentWrite,
                    {},
                    vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                    vk::PipelineStageFlagBits2::eBottomOfPipe,
                    vk::ImageAspectFlagBits::eColor
                );
            }
		    commandBuffer.end();
	    }

	    void transition_image_layout(vk::Image               image,
                                     vk::ImageLayout         old_layout,
                                     vk::ImageLayout         new_layout,
                                     vk::AccessFlags2        src_access_mask,
                                     vk::AccessFlags2        dst_access_mask,
                                     vk::PipelineStageFlags2 src_stage_mask,
                                     vk::PipelineStageFlags2 dst_stage_mask,
                                     vk::ImageAspectFlags    image_aspect_flags) {

            //----------------------------------------------------------------------
            // .srcStageMask: preceding work stage
            // .srcAccessMask: memory accesses the preceding work performs
            // .dstStageMask: the subsequent work stage
            // .dstAccessMask: memory accesses the subsequent work performs
            //----------------------------------------------------------------------
            vk::ImageMemoryBarrier2 barrier = {
                .srcStageMask        = src_stage_mask,
                .srcAccessMask       = src_access_mask,
                .dstStageMask        = dst_stage_mask,
                .dstAccessMask       = dst_access_mask,
                .oldLayout           = old_layout,
                .newLayout           = new_layout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image               = image,
                .subresourceRange    = {
                    .aspectMask     = image_aspect_flags,
                    .baseMipLevel   = 0,
                    .levelCount     = 1,
                    .baseArrayLayer = 0,
                    .layerCount     = 1
                }
            };
            vk::DependencyInfo dependency_info = {
                .dependencyFlags         = {},
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers    = &barrier
            };
		    commandBuffers[frameIndex].pipelineBarrier2(dependency_info);
	    }

        void saveFrameToPng(vk::raii::DeviceMemory &captureMemory) {
            const uint32_t width = swapchain.getSwapchainExtent().width;
            const uint32_t height = swapchain.getSwapchainExtent().height;
            const size_t pixelCount = static_cast<size_t>(width) * height;
            std::vector<unsigned char> pixels(pixelCount * 4);

            void *mapped = captureMemory.mapMemory(0, pixels.size());
            memcpy(pixels.data(), mapped, pixels.size());
            captureMemory.unmapMemory();

            const bool isBgra = swapchain.getSwapchainSurfaceFormat().format == vk::Format::eB8G8R8A8Srgb ||
                                swapchain.getSwapchainSurfaceFormat().format == vk::Format::eB8G8R8A8Unorm;
            if (isBgra) {
                for (size_t i = 0; i < pixelCount; ++i) {
                    std::swap(pixels[i * 4], pixels[i * 4 + 2]);
                }
            }

            if (width > static_cast<uint32_t>(std::numeric_limits<int>::max() / 4) ||
                height > static_cast<uint32_t>(std::numeric_limits<int>::max()) ||
                stbi_write_png("frame.png", static_cast<int>(width), static_cast<int>(height), 4,
                                pixels.data(), static_cast<int>(width * 4)) == 0) {
                throw std::runtime_error("failed to save frame.png!");
            }
            std::cout << "Saved frame to frame.png" << std::endl;
        }
};
}
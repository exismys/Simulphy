#include "Device.hpp"
#include "Pipeline.hpp"
#include "Surface.hpp"
#include "Swapchain.hpp"
#include <vulkan/vulkan_raii.hpp>

#include <SDL.h>
#include <SDL_vulkan.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/geometric.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <unordered_map>

#include "Vertex.hpp"
#include "Window.hpp"
#include "Instance.hpp"
#include "Device.hpp"

constexpr uint32_t WIDTH = 1200;
constexpr uint32_t HEIGHT = 720;

const std::string  MODEL_PATH = "models/test-models/MenlogateLow.obj";
const std::string  TEXTURE_PATH = "textures/test-textures/Menlogate.jpg";

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

// const std::vector<Vertex> vertices = {
//     {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//     {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//     {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//     {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

//     {{-0.5f, -0.5f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//     {{0.5f, -0.5f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//     {{0.5f, 0.5f, -1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//     {{-0.5f, 0.5f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
// };

// const std::vector<uint16_t> indices = {
//     0, 1, 2, 2, 3, 0,
//     4, 7, 6, 6, 5, 4,

//     4, 0, 3, 3, 7, 4,
//     1, 5, 6, 6, 2, 1,

//     4, 5, 1, 1, 0, 4,
//     3, 2, 6, 6, 7, 3
// };

constexpr glm::vec3 cameraPosition = {0, 2, 15};
constexpr glm::vec3 targetPosition = {0, 0, 0};

struct UniformBufferObject{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class Application {
    public:
        void run() {
            initVulkan();
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

        VkFormat imguiColorFormat = VK_FORMAT_UNDEFINED;

        vk::raii::Image colorImage = nullptr;
        vk::raii::DeviceMemory colorImageMemory = nullptr;
        vk::raii::ImageView colorImageView = nullptr;

        vk::raii::Image depthImage = nullptr;
        vk::raii::DeviceMemory depthImageMemory = nullptr;
        vk::raii::ImageView depthImageView = nullptr;

        uint32_t mipLevels = 0;
        vk::raii::Image textureImage = nullptr;
	    vk::raii::DeviceMemory textureImageMemory = nullptr;
        vk::raii::ImageView textureImageView = nullptr;
        vk::raii::Sampler textureSampler = nullptr;

        std::vector<Simulphy::Vertex> vertices;
        std::vector<uint32_t> indices;
        vk::raii::Buffer vertexBuffer  = nullptr;
	    vk::raii::DeviceMemory vertexBufferMemory = nullptr;
        vk::raii::Buffer indexBuffer = nullptr;
        vk::raii::DeviceMemory indexBufferMemory = nullptr;

        std::vector<vk::raii::Buffer> uniformBuffers;
        std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
        std::vector<void *> uniformBuffersMapped;

        vk::raii::DescriptorPool descriptorPool = nullptr;
	    std::vector<vk::raii::DescriptorSet> descriptorSets;

        vk::raii::CommandPool commandPool = nullptr;
        std::vector<vk::raii::CommandBuffer> commandBuffers;

        std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
        std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
        std::vector<vk::raii::Fence> inFlightFences;
        uint32_t frameIndex = 0;

        bool framebufferResized = false;
        bool windowShouldClose = false;
        bool mouseLookActive = false;
        bool relativeMouseJustEnabled = false;

        bool canCaptureFrame = false;
        bool saveFrameRequested = false;

        glm::vec3 normalizedDir = glm::normalize(targetPosition - cameraPosition);

        float cameraYaw = std::atan2(normalizedDir.x, normalizedDir.z);
        float cameraPitch = std::asin(normalizedDir.y);

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

	    void initVulkan() {
            createCommandPool();
            createColorResources();
            createDepthResources();
            createTextureImage();
            createTextureImageView();
            createTextureSampler();
            loadModel();
            createVertexBuffer();
            createIndexBuffer();
            createUniformBuffers();
            createDescriptorPool();
            createDescriptorSets();
            createCommandBuffer();
            createSyncObjects();
            initImGui();
        }

        void mainLoop() {
            while (!windowShouldClose) {
                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                    ImGui_ImplSDL2_ProcessEvent(&event);
                    processEvent(event);
                }
                handleRelativeMouseMovement();
                if (windowShouldClose) {
                    break;
                }
                drawFrame();
            }
            device.getLogicalDevice().waitIdle();
        }

        void cleanup() {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext();
            swapchain.cleanupSwapchain();
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

            swapchain = Simulphy::Swapchain(surface, device, window);
            createColorResources();
            createDepthResources();
            initImGuiVulkan();
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

        void createCommandPool() {
            vk::CommandPoolCreateInfo poolInfo{
                .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                .queueFamilyIndex = device.getQueueIndex()
            };
            commandPool = vk::raii::CommandPool(device.getLogicalDevice(), poolInfo);
	    }

        void createColorResources() {
            vk::Format colorFormat = swapchain.getSwapchainSurfaceFormat().format;

            std::tie(colorImage, colorImageMemory) = createImage(swapchain.getSwapchainExtent().width,
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

            std::tie(depthImage, depthImageMemory) = createImage(swapchain.getSwapchainExtent().width,
                                                                          swapchain.getSwapchainExtent().height,
                                                                          1,
                                                                          device.getMaxUsableSampleCount(),
                                                                          depthFormat, 
                                                                          vk::ImageTiling::eOptimal, 
                                                                          vk::ImageUsageFlagBits::eDepthStencilAttachment, 
                                                                          vk::MemoryPropertyFlagBits::eDeviceLocal);
            depthImageView = device.createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, 1);
	    }

        void createTextureImage() {
            int texWidth, texHeight, texChannels;
            stbi_uc *pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            vk::DeviceSize imageSize = texWidth * texHeight * 4;
            mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

            if (!pixels) {
                throw std::runtime_error("failed to load texture image!");
            }

            auto [stagingBuffer, stagingBufferMemory] = createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

            void *data = stagingBufferMemory.mapMemory(0, imageSize);
            memcpy(data, pixels, imageSize);
            stagingBufferMemory.unmapMemory();

            stbi_image_free(pixels);

            std::tie(textureImage, textureImageMemory) = createImage(texWidth,
                                                                              texHeight,
                                                                              mipLevels,
                                                                              vk::SampleCountFlagBits::e1,
                                                                              vk::Format::eR8G8B8A8Srgb,
                                                                              vk::ImageTiling::eOptimal,
                                                                              vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                                                                              vk::MemoryPropertyFlagBits::eDeviceLocal);

            vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands();
            transitionImageLayout(commandBuffer, textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
            copyBufferToImage(commandBuffer, stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
            generateMipmaps(commandBuffer, textureImage, vk::Format::eR8G8B8A8Srgb, texWidth, texHeight, mipLevels);
            // transitionImageLayout(commandBuffer, textureImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, mipLevels);
            endSingleTimeCommands(std::move(commandBuffer));
	    }

        void generateMipmaps(vk::raii::CommandBuffer &commandBuffer,
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


        void createTextureImageView() {
		    textureImageView = device.createImageView(*textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, mipLevels);
	    }

        void createTextureSampler() {
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

        std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(uint32_t width, 
                                                                       uint32_t height, 
                                                                       uint32_t mipLevels,
                                                                       vk::SampleCountFlagBits numSamples,
                                                                       vk::Format format, 
                                                                       vk::ImageTiling tiling, 
                                                                       vk::ImageUsageFlags usage, 
                                                                       vk::MemoryPropertyFlags properties) {
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

            vk::raii::Image image = vk::raii::Image(device.getLogicalDevice(), imageInfo);

            vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
            vk::MemoryAllocateInfo allocInfo{
                .allocationSize  = memRequirements.size,
                .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
            };
            vk::raii::DeviceMemory imageMemory = vk::raii::DeviceMemory(device.getLogicalDevice(), allocInfo);
            image.bindMemory(imageMemory, 0);

            return {std::move(image), std::move(imageMemory)};
	    }

        void transitionImageLayout(vk::raii::CommandBuffer &commandBuffer, const vk::raii::Image &image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels) {
            vk::ImageMemoryBarrier barrier{
                .oldLayout           = oldLayout,
                .newLayout           = newLayout,
                .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
                .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
                .image               = image,
                .subresourceRange    = {.aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = mipLevels, .layerCount = 1}
            };

            vk::PipelineStageFlags sourceStage;
            vk::PipelineStageFlags destinationStage;

            if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
                barrier.srcAccessMask = {};
                barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

                sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
                destinationStage = vk::PipelineStageFlagBits::eTransfer;
            } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

                sourceStage = vk::PipelineStageFlagBits::eTransfer;
                destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
            } else {
                throw std::invalid_argument("unsupported layout transition!");
            }
            commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);
	    }

        void copyBufferToImage(vk::raii::CommandBuffer &commandBuffer, const vk::raii::Buffer &buffer, vk::raii::Image &image, uint32_t width, uint32_t height) {
            vk::BufferImageCopy region{
                .bufferOffset      = 0,
                .bufferRowLength   = 0,
                .bufferImageHeight = 0,
                .imageSubresource  = {.aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
                .imageOffset       = {0, 0, 0},
                .imageExtent       = {width, height, 1}
            };
            commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
	    }

        void loadModel() {
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
                throw std::runtime_error(warn + err);
		    }

            std::unordered_map<Simulphy::Vertex, uint32_t> uniqueVertices{};

            for (const auto &shape : shapes) {
                for (const auto &index : shape.mesh.indices) {
                    Simulphy::Vertex vertex{};

                    vertex.pos = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    vertex.texCoord = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    };

                    vertex.color = {1.0f, 1.0f, 1.0f};

                    auto [it, inserted] = uniqueVertices.insert({vertex, static_cast<uint32_t>(vertices.size())});
                    if (inserted) {
                        vertices.push_back(vertex);
                    }

                    indices.push_back(it->second);
                }
            }
        }

        std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, 
                                                                         vk::BufferUsageFlags usage, 
                                                                         vk::MemoryPropertyFlags properties) {
            vk::BufferCreateInfo bufferInfo{.size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};
            vk::raii::Buffer buffer = vk::raii::Buffer(device.getLogicalDevice(), bufferInfo);
            vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
            vk::MemoryAllocateInfo allocInfo{
                .allocationSize = memRequirements.size, 
                .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
            };
            vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(device.getLogicalDevice(), allocInfo);
            buffer.bindMemory(*bufferMemory, 0);
            return {std::move(buffer), std::move(bufferMemory)};
	    }

        void createVertexBuffer() {
            vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

            auto [stagingBuffer, stagingBufferMemory] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

            void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
            memcpy(dataStaging, vertices.data(), bufferSize);
            stagingBufferMemory.unmapMemory();

            std::tie(vertexBuffer, vertexBufferMemory) = createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);

            copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
	    }

        void createIndexBuffer() {
            vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

            auto [stagingBuffer, stagingBufferMemory] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

            void *data = stagingBufferMemory.mapMemory(0, bufferSize);
            memcpy(data, indices.data(), (size_t) bufferSize);
            stagingBufferMemory.unmapMemory();

            std::tie(indexBuffer, indexBufferMemory) = createBuffer(bufferSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);

            copyBuffer(stagingBuffer, indexBuffer, bufferSize);
        }

        void createUniformBuffers() {
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
                auto [buffer, bufferMem] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
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

        void createDescriptorSets() {
            std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *pipeline.getDescriptorSetLayout());
            vk::DescriptorSetAllocateInfo allocInfo{
                .descriptorPool = descriptorPool,
                .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
                .pSetLayouts = layouts.data()
            };

            descriptorSets = device.getLogicalDevice().allocateDescriptorSets(allocInfo);

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vk::DescriptorBufferInfo bufferInfo{.buffer = uniformBuffers[i], .offset = 0, .range = sizeof(UniformBufferObject)};
                vk::DescriptorImageInfo  imageInfo{.sampler = textureSampler, .imageView = textureImageView, .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};
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

        void copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size) {
            vk::raii::CommandBuffer commandCopyBuffer = beginSingleTimeCommands();
            commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy{.size = size});
            endSingleTimeCommands(std::move(commandCopyBuffer));
	    }

        vk::raii::CommandBuffer beginSingleTimeCommands() {
            vk::CommandBufferAllocateInfo allocInfo{.commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1};
            vk::raii::CommandBuffer commandBuffer = std::move(vk::raii::CommandBuffers(device.getLogicalDevice(), allocInfo).front());

            vk::CommandBufferBeginInfo beginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
            commandBuffer.begin(beginInfo);

            return std::move(commandBuffer);
        }

        void endSingleTimeCommands(vk::raii::CommandBuffer &&commandBuffer) {
            commandBuffer.end();

            vk::SubmitInfo submitInfo{.commandBufferCount = 1, .pCommandBuffers = &*commandBuffer};
            device.getGraphicsQueue().submit(submitInfo, nullptr);
            device.getGraphicsQueue().waitIdle();
        }

        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
            vk::PhysicalDeviceMemoryProperties memProperties = device.getPhysicalDevice().getMemoryProperties();

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


        void createCommandBuffer() {
            vk::CommandBufferAllocateInfo allocInfo{.commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = MAX_FRAMES_IN_FLIGHT};
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
            cameraYaw -= static_cast<float>(relX) * sensitivity;
            // std::cout << "cameraYaw: " << cameraYaw << std::endl;
            cameraPitch -= static_cast<float>(relY) * sensitivity;
            // std::cout << "cameraPitch: " << cameraPitch << std::endl;
            cameraPitch = std::clamp(cameraPitch, -1.5f, 1.5f);
        }

        void updateUniformBuffer(uint32_t currentImage) {
            static auto startTime = std::chrono::high_resolution_clock::now();
            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float>(currentTime - startTime).count();

            UniformBufferObject ubo{};
            ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
                        // rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
                        // rotate(glm::mat4(1.0f), time * glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
                        // rotate(glm::mat4(1.0f), /*time*/ glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

            const glm::vec3 cameraDirection(
                std::cos(cameraPitch) * std::sin(cameraYaw),
                std::sin(cameraPitch),
                std::cos(cameraPitch) * std::cos(cameraYaw)
            );
            ubo.view = lookAt(cameraPosition, cameraPosition + cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f));
            ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapchain.getSwapchainExtent().width) / static_cast<float>(swapchain.getSwapchainExtent().height), 0.1f, 100.0f);

            memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	    }

        void drawFrame() {
            auto fenceResult = device.getLogicalDevice().waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);
            if (fenceResult != vk::Result::eSuccess) {
                throw std::runtime_error("failed to wait for fence!");
            }

		    auto [result, imageIndex] = swapchain.getSwapchain().acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);

            if (result == vk::Result::eErrorOutOfDateKHR) {
                recreateSwapChain();
                return;
		    }

		    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
                assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
                throw std::runtime_error("failed to acquire swap chain image!");
		    }

            updateUniformBuffer(frameIndex);
            renderImGui();

            const bool captureFrame = saveFrameRequested && canCaptureFrame;
            saveFrameRequested = false;

            vk::raii::Buffer captureBuffer = nullptr;
            vk::raii::DeviceMemory captureBufferMemory = nullptr;
            if (captureFrame) {
                const vk::DeviceSize captureSize = static_cast<vk::DeviceSize>(swapchain.getSwapchainExtent().width) * swapchain.getSwapchainExtent().height * 4;
                std::tie(captureBuffer, captureBufferMemory) = createBuffer(captureSize,
                                                                                     vk::BufferUsageFlagBits::eTransferDst,
                                                                                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            }

            device.getLogicalDevice().resetFences(*inFlightFences[frameIndex]);

            commandBuffers[frameIndex].reset();
		    recordCommandBuffer(imageIndex, captureFrame ? &*captureBuffer : nullptr);

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
                framebufferResized = false;
                recreateSwapChain();
		    } else {
			    assert(result == vk::Result::eSuccess);
            }

            frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
        }

        void renderImGui() {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            const ImGuiIO &io = ImGui::GetIO();
            ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(0.35f);
            ImGui::Begin("Frame rate", nullptr,
                         ImGuiWindowFlags_NoDecoration |
                         ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_NoNav);
            ImGui::Text("FPS: %.1f", io.Framerate);
            ImGui::Text("Number of Vertices: %s", formatNumberLocale(static_cast<int>(vertices.size())).c_str());
            ImGui::Text("Number of Triangles: %s", formatNumberLocale(static_cast<int>(indices.size() / 3)).c_str());
            if (!canCaptureFrame) {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button("save frame")) {
                saveFrameRequested = true;
            }
            if (!canCaptureFrame) {
                ImGui::EndDisabled();
                ImGui::TextDisabled("Frame capture is not supported");
            }
            ImGui::End();
            ImGui::Render();
        }

        std::string formatNumberLocale(int number) {
            std::stringstream ss;
            ss.imbue(std::locale("en_US.UTF-8"));
            ss << std::fixed << number;
            return ss.str();
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

        void recordCommandBuffer(uint32_t imageIndex, const vk::Buffer *captureBuffer) {
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

            commandBuffer.beginRendering(renderingInfo);
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.getGraphicsPipeline());
            commandBuffer.bindVertexBuffers(0, *vertexBuffer, {0});
            commandBuffers[frameIndex].bindIndexBuffer(*indexBuffer, 0, vk::IndexTypeValue<decltype(indices)::value_type>::value);
            commandBuffer.setViewport(0, vk::Viewport(0.0f, static_cast<float>(swapchain.getSwapchainExtent().height), static_cast<float>(swapchain.getSwapchainExtent().width), -static_cast<float>(swapchain.getSwapchainExtent().height), 0.0f, 1.0f));
            commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain.getSwapchainExtent()));
            commandBuffers[frameIndex].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.getPipelineLayout(), 0, *descriptorSets[frameIndex], nullptr);
            commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), static_cast<VkCommandBuffer>(*commandBuffer));
            commandBuffer.endRendering();

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
};

int main() {
    try {
        Application app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

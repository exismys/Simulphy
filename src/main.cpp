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
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <unordered_map>

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

const std::string  MODEL_PATH = "models/viking_room.obj";
const std::string  TEXTURE_PATH = "textures/viking_room.png";

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<char const*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
    glm::vec2 texCoord;

	static vk::VertexInputBindingDescription getBindingDescription() {
		return {.binding = 0, .stride = sizeof(Vertex), .inputRate = vk::VertexInputRate::eVertex};
	}

	static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions() {
		return {
            {
                {.location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, pos)},
                {.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, color)},
                {.location = 2, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, texCoord)}
            }
        };
	}

    bool operator==(const Vertex &other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

template <>
struct std::hash<Vertex> {
	size_t operator()(Vertex const &vertex) const noexcept {
		return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
	}
};

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

constexpr glm::vec3 cameraPosition = {0, 2, 2};
constexpr glm::vec3 targetPosition = {0, 0, 0};

struct UniformBufferObject{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class Application {
    public:
        void run() {
            initWindow();
            initVulkan();
            mainLoop();
            cleanup();
        }

    private:
        SDL_Window *window = nullptr;
        vk::raii::Context context;
        vk::raii::Instance instance = nullptr;
        vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
        vk::raii::PhysicalDevice physicalDevice = nullptr;
        vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;
        vk::raii::Device device = nullptr;
        uint32_t queueIndex = ~0;
        vk::raii::Queue graphicsQueue = nullptr;
        vk::raii::SurfaceKHR surface = nullptr;
        vk::Extent2D swapChainExtent;
        vk::SurfaceFormatKHR swapChainSurfaceFormat;
        vk::raii::SwapchainKHR swapChain = nullptr;
        std::vector<vk::Image> swapChainImages;
        std::vector<vk::raii::ImageView> swapChainImageViews;

        VkFormat imguiColorFormat = VK_FORMAT_UNDEFINED;

        vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
        vk::raii::PipelineLayout pipelineLayout = nullptr;
        vk::raii::Pipeline graphicsPipeline = nullptr;

        vk::raii::Image colorImage = nullptr;
        vk::raii::DeviceMemory colorImageMemory = nullptr;
        vk::raii::ImageView colorImageView = nullptr;

        vk::Format depthFormat;
        vk::raii::Image depthImage = nullptr;
        vk::raii::DeviceMemory depthImageMemory = nullptr;
        vk::raii::ImageView depthImageView = nullptr;

        uint32_t mipLevels = 0;
        vk::raii::Image textureImage = nullptr;
	    vk::raii::DeviceMemory textureImageMemory = nullptr;
        vk::raii::ImageView textureImageView = nullptr;
        vk::raii::Sampler textureSampler = nullptr;

        std::vector<Vertex> vertices;
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

        void initWindow() {
	        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
	            throw std::runtime_error(std::string("failed to initialize SDL: ") + SDL_GetError());
	        }

	        window = SDL_CreateWindow(
	            "Simulphy",
	            SDL_WINDOWPOS_CENTERED,
	            SDL_WINDOWPOS_CENTERED,
	            static_cast<int>(WIDTH),
	            static_cast<int>(HEIGHT),
	            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
	        );
	        if (window == nullptr) {
	            throw std::runtime_error(std::string("failed to create SDL window: ") + SDL_GetError());
	        }
	    }

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
            createInstance();
            setupDebugMessenger();
            createSurface();
            pickPhysicalDevice();
            createLogicalDevice();
            createSwapChain();
            createImageViews();
            createDescriptorSetLayout();
            createGraphicsPipeline();
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
            device.waitIdle();
        }

        void cleanupSwapChain() {
            swapChainImageViews.clear();
            swapChain = nullptr;
	    }

        void cleanup() {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext();
            cleanupSwapChain();

            SDL_DestroyWindow(window);
            SDL_Quit();
        }

        void recreateSwapChain() {
            int width = 0, height = 0;
            SDL_Vulkan_GetDrawableSize(window, &width, &height);
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
                SDL_Vulkan_GetDrawableSize(window, &width, &height);
            }
            if (windowShouldClose) {
                return;
            }

            device.waitIdle();
            ImGui_ImplVulkan_Shutdown();

            cleanupSwapChain();

            createSwapChain();
            createImageViews();
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

            if (!ImGui_ImplSDL2_InitForVulkan(window)) {
                throw std::runtime_error("failed to initialize ImGui SDL2 backend!");
            }
            initImGuiVulkan();
        }

        void initImGuiVulkan() {
            imguiColorFormat = static_cast<VkFormat>(swapChainSurfaceFormat.format);
            ImGui_ImplVulkan_InitInfo initInfo{};
            initInfo.ApiVersion = VK_API_VERSION_1_3;
            initInfo.Instance = static_cast<VkInstance>(*instance);
            initInfo.PhysicalDevice = static_cast<VkPhysicalDevice>(*physicalDevice);
            initInfo.Device = static_cast<VkDevice>(*device);
            initInfo.QueueFamily = queueIndex;
            initInfo.Queue = static_cast<VkQueue>(*graphicsQueue);
            initInfo.DescriptorPoolSize = 1000;
            initInfo.MinImageCount = 2;
            initInfo.ImageCount = static_cast<uint32_t>(swapChainImages.size());
            initInfo.MSAASamples = static_cast<VkSampleCountFlagBits>(msaaSamples);
            initInfo.UseDynamicRendering = true;
            initInfo.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
            initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &imguiColorFormat;
            initInfo.PipelineRenderingCreateInfo.depthAttachmentFormat = static_cast<VkFormat>(depthFormat);

            if (!ImGui_ImplVulkan_Init(&initInfo)) {
                throw std::runtime_error("failed to initialize ImGui Vulkan backend!");
            }
        }

        void createInstance() {
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

            auto requiredExtensions = getRequiredInstanceExtensions();

            auto extensionProperties = context.enumerateInstanceExtensionProperties();
            auto unsupportedPropertyIt = std::ranges::find_if(requiredExtensions, [&extensionProperties](auto const &requiredExtension) {
                return std::ranges::none_of(extensionProperties, [requiredExtension](auto const &extensionProperty) {
                    return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
                });
            });

            if (unsupportedPropertyIt != requiredExtensions.end()) {
                throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
            }

            constexpr vk::ApplicationInfo appInfo{
                .pApplicationName   = "BeLight",
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

        std::vector<const char*> getRequiredInstanceExtensions() {
            unsigned int extensionCount = 0;
            if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr)) {
                throw std::runtime_error(std::string("failed to get SDL Vulkan extension count: ") + SDL_GetError());
            }
            std::vector<const char*> extensions(extensionCount);
            if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions.data())) {
                throw std::runtime_error(std::string("failed to get SDL Vulkan extensions: ") + SDL_GetError());
            }
            if (enableValidationLayers) {
                extensions.push_back(vk::EXTDebugUtilsExtensionName);
            }

            return extensions;
        }

        static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
                                                              vk::DebugUtilsMessageTypeFlagsEXT              type,
                                                              const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData,
                                                              void *                                         pUserData) {

            if (
                severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning ||
                severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
            ) {
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

        void pickPhysicalDevice() {
            std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
            auto const devIter = std::ranges::find_if(physicalDevices, [&](auto const & physicalDevice) {
                return isDeviceSuitable(physicalDevice); 
            });

            if (devIter == physicalDevices.end()) {
                throw std::runtime_error( "failed to find a suitable GPU!" );
            }
            physicalDevice = *devIter;
            msaaSamples = getMaxUsableSampleCount();
            std::cout << "MSAA samples: " << static_cast<uint32_t>(msaaSamples) << '\n';
        }

        bool isDeviceSuitable(vk::raii::PhysicalDevice const & physicalDevice) {
            bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;

            auto queueFamilies = physicalDevice.getQueueFamilyProperties();
            bool supportsGraphics = std::ranges::any_of(queueFamilies, [](auto const & qfp) {
                return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); 
            });

            auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
            bool supportsAllRequiredExtensions = std::ranges::all_of( requiredDeviceExtension,[&availableDeviceExtensions](auto const & requiredDeviceExtension) {
                return std::ranges::any_of(availableDeviceExtensions, [requiredDeviceExtension](auto const & availableDeviceExtension) { 
                    return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; 
                });
            });

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

            return vk::SampleCountFlagBits::e4;
	    }

        void createSurface() {
            VkSurfaceKHR _surface;
            if (!SDL_Vulkan_CreateSurface(window, *instance, &_surface)) {
                throw std::runtime_error("failed to create window surface!");
            }
            surface = vk::raii::SurfaceKHR(instance, _surface);
        }

        void createLogicalDevice() {
            std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

            for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++) {
                if (
                    (queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) && 
                    physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface)
                ) {
                    queueIndex = qfpIndex;
                    break;
                }
            }
            if (queueIndex == ~0) {
                throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
            }

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
                .queueCount = 1, 
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

        void createSwapChain() {
            vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR( *surface );
            swapChainExtent = chooseSwapExtent(surfaceCapabilities);
            uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);

            std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
            swapChainSurfaceFormat = chooseSwapSurfaceFormat(availableFormats);

            const bool hasSupportedPixelFormat = swapChainSurfaceFormat.format == vk::Format::eB8G8R8A8Srgb ||
                                                 swapChainSurfaceFormat.format == vk::Format::eB8G8R8A8Unorm ||
                                                 swapChainSurfaceFormat.format == vk::Format::eR8G8B8A8Srgb ||
                                                 swapChainSurfaceFormat.format == vk::Format::eR8G8B8A8Unorm;
            const auto formatProperties = physicalDevice.getFormatProperties(swapChainSurfaceFormat.format);
            canCaptureFrame = (surfaceCapabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferSrc) &&
                              (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eTransferSrc) &&
                              hasSupportedPixelFormat;

            std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);
		    vk::PresentModeKHR presentMode = chooseSwapPresentMode(availablePresentModes);

            vk::SwapchainCreateInfoKHR swapChainCreateInfo{
                .surface          = *surface,
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

            swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
            swapChainImages = swapChain.getImages();
        }

        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats) {
            assert(!availableFormats.empty());
            const auto formatIt = std::ranges::find_if(availableFormats, [](const auto &format) { 
                return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; 
            });
            return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
        }

        vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes) {
            assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { 
                return presentMode == vk::PresentModeKHR::eFifo; 
            }));
            return std::ranges::any_of(availablePresentModes, [](const vk::PresentModeKHR value) { 
                return vk::PresentModeKHR::eMailbox == value; 
            }) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
        }

        vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities){
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                return capabilities.currentExtent;
            }

            int width, height;
            SDL_Vulkan_GetDrawableSize(window, &width, &height);

            return {
                std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
                std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
            };
        }

        uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities) {
            auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
            if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount)) {
                minImageCount = surfaceCapabilities.maxImageCount;
            }
            return minImageCount;
        }

        void createImageViews() {
            assert(swapChainImageViews.empty());

            swapChainImageViews.reserve(swapChainImages.size());
            for (auto &image : swapChainImages) {
                swapChainImageViews.emplace_back(createImageView(image, swapChainSurfaceFormat.format, vk::ImageAspectFlagBits::eColor, 1));
            }
        }

        void createDescriptorSetLayout() {
            std::array<vk::DescriptorSetLayoutBinding, 2> bindings{
                {
                    {
                        .binding = 0, 
                        .descriptorType = vk::DescriptorType::eUniformBuffer, 
                        .descriptorCount = 1, 
                        .stageFlags = vk::ShaderStageFlagBits::eVertex
                    },
                    {
                        .binding = 1, 
                        .descriptorType = vk::DescriptorType::eCombinedImageSampler, 
                        .descriptorCount = 1, 
                        .stageFlags = vk::ShaderStageFlagBits::eFragment
                    }
                }
            };
            vk::DescriptorSetLayoutCreateInfo layoutInfo{
                .bindingCount = static_cast<uint32_t>(bindings.size()), 
                .pBindings = bindings.data()
            };
            descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
	    }

        void createGraphicsPipeline() {
            vk::raii::ShaderModule shaderModule = createShaderModule(readFile("shaders/slang.spv"));

            vk::PipelineShaderStageCreateInfo vertShaderStageInfo{.stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModule, .pName = "vertMain"};
            vk::PipelineShaderStageCreateInfo fragShaderStageInfo{.stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModule, .pName = "fragMain"};
            vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

            auto bindingDescription = Vertex::getBindingDescription();
		    auto attributeDescriptions = Vertex::getAttributeDescriptions();

            vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
                .vertexBindingDescriptionCount   = 1,
                .pVertexBindingDescriptions      = &bindingDescription,
                .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
                .pVertexAttributeDescriptions    = attributeDescriptions.data()
            };
            
            vk::PipelineInputAssemblyStateCreateInfo inputAssembly{.topology = vk::PrimitiveTopology::eTriangleList};
            vk::PipelineViewportStateCreateInfo      viewportState{.viewportCount = 1, .scissorCount = 1};

            vk::PipelineRasterizationStateCreateInfo rasterizer{
                .depthClampEnable        = vk::False,
                .rasterizerDiscardEnable = vk::False,
                .polygonMode             = vk::PolygonMode::eFill,
                .cullMode                = vk::CullModeFlagBits::eBack,
                .frontFace               = vk::FrontFace::eCounterClockwise,
                .depthBiasEnable         = vk::False,
                .lineWidth               = 1.0f
            };

            vk::PipelineMultisampleStateCreateInfo multisampling{.rasterizationSamples = msaaSamples, .sampleShadingEnable = vk::False};

            vk::PipelineDepthStencilStateCreateInfo depthStencil{
                .depthTestEnable       = vk::True,
                .depthWriteEnable      = vk::True,
                .depthCompareOp        = vk::CompareOp::eLess,
                .depthBoundsTestEnable = vk::False,
                .stencilTestEnable     = vk::False
            };

            vk::PipelineColorBlendAttachmentState colorBlendAttachment{
                .blendEnable    = vk::False,
                .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
            };

            vk::PipelineColorBlendStateCreateInfo colorBlending{
                .logicOpEnable = vk::False, 
                .logicOp = vk::LogicOp::eCopy, 
                .attachmentCount = 1, 
                .pAttachments = &colorBlendAttachment
            };

            std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
            vk::PipelineDynamicStateCreateInfo dynamicState{
                .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), 
                .pDynamicStates = dynamicStates.data()
            };

            vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = 1, .pSetLayouts = &*descriptorSetLayout, .pushConstantRangeCount = 0};
            pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

            vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{ 
                .colorAttachmentCount = 1, 
                .pColorAttachmentFormats = &swapChainSurfaceFormat.format 
            };

            depthFormat = findDepthFormat();

            vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
                {
                    .stageCount          = 2,
                    .pStages             = shaderStages,
                    .pVertexInputState   = &vertexInputInfo,
                    .pInputAssemblyState = &inputAssembly,
                    .pViewportState      = &viewportState,
                    .pRasterizationState = &rasterizer,
                    .pMultisampleState   = &multisampling,
                    .pDepthStencilState  = &depthStencil,
                    .pColorBlendState    = &colorBlending,
                    .pDynamicState       = &dynamicState,
                    .layout              = pipelineLayout,
                    .renderPass          = nullptr
                },
                {
                    .colorAttachmentCount = 1, 
                    .pColorAttachmentFormats = &swapChainSurfaceFormat.format,
                    .depthAttachmentFormat = depthFormat
                }
            };

            graphicsPipeline = vk::raii::Pipeline(device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
        }

        [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char> &code) const {
            vk::ShaderModuleCreateInfo createInfo{.codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t *>(code.data())};
            vk::raii::ShaderModule     shaderModule{device, createInfo};

            return shaderModule;
	    }

        static std::vector<char> readFile(const std::string &filename) {
            std::ifstream file(filename, std::ios::ate | std::ios::binary);
            if (!file.is_open()) {
                throw std::runtime_error("failed to open file!");
            }
            std::vector<char> buffer(file.tellg());
            file.seekg(0, std::ios::beg);
            file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
            file.close();
            return buffer;
	    }

        void createCommandPool() {
            vk::CommandPoolCreateInfo poolInfo{
                .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                .queueFamilyIndex = queueIndex
            };
            commandPool = vk::raii::CommandPool(device, poolInfo);
	    }

        void createColorResources() {
            vk::Format colorFormat = swapChainSurfaceFormat.format;

            std::tie(colorImage, colorImageMemory) = createImage(swapChainExtent.width,
                                                                        swapChainExtent.height,
                                                                        1,
                                                                        msaaSamples,
                                                                        colorFormat,
                                                                        vk::ImageTiling::eOptimal,
                                                                        vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
                                                                        vk::MemoryPropertyFlagBits::eDeviceLocal);
            colorImageView = createImageView(colorImage, colorFormat, vk::ImageAspectFlagBits::eColor, 1);
	    }

        void createDepthResources() {
            vk::Format depthFormat = findDepthFormat();

            std::tie(depthImage, depthImageMemory) = createImage(swapChainExtent.width,
                                                                          swapChainExtent.height,
                                                                          1,
                                                                          msaaSamples,
                                                                          depthFormat, 
                                                                          vk::ImageTiling::eOptimal, 
                                                                          vk::ImageUsageFlagBits::eDepthStencilAttachment, 
                                                                          vk::MemoryPropertyFlagBits::eDeviceLocal);
            depthImageView = createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, 1);
	    }

        vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
            for (const auto format : candidates) {
                vk::FormatProperties props = physicalDevice.getFormatProperties(format);
                if (
                    ((tiling == vk::ImageTiling::eLinear) && ((props.linearTilingFeatures & features) == features)) ||
                    ((tiling == vk::ImageTiling::eOptimal) && ((props.optimalTilingFeatures & features) == features))
                ) {
                    return format;
                }
            }

            throw std::runtime_error("failed to find supported format!");
        }

        vk::Format findDepthFormat() {
            return findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                                       vk::ImageTiling::eOptimal,
                                       vk::FormatFeatureFlagBits::eDepthStencilAttachment);
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

		    vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(imageFormat);
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
		    textureImageView = createImageView(*textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, mipLevels);
	    }

        vk::raii::ImageView createImageView(vk::Image const &image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels) {
            vk::ImageViewCreateInfo viewInfo{
                .image            = image,
                .viewType         = vk::ImageViewType::e2D,
                .format           = format,
                .subresourceRange = {.aspectMask = aspectFlags, .baseMipLevel = 0, .levelCount = mipLevels, .baseArrayLayer = 0, .layerCount = 1}
            };
            return vk::raii::ImageView(device, viewInfo);
	    }

        void createTextureSampler() {
            vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
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
            textureSampler = vk::raii::Sampler(device, samplerInfo);
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

            std::unordered_map<Vertex, uint32_t> uniqueVertices{};

            for (const auto &shape : shapes) {
                for (const auto &index : shape.mesh.indices) {
                    Vertex vertex{};

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
            vk::raii::Buffer buffer = vk::raii::Buffer(device, bufferInfo);
            vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
            vk::MemoryAllocateInfo allocInfo{
                .allocationSize = memRequirements.size, 
                .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
            };
            vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
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
            descriptorPool = vk::raii::DescriptorPool(device, poolInfo);
	    }

        void createDescriptorSets() {
            std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
            vk::DescriptorSetAllocateInfo allocInfo{
                .descriptorPool = descriptorPool,
                .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
                .pSetLayouts = layouts.data()
            };

            descriptorSets = device.allocateDescriptorSets(allocInfo);

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
                device.updateDescriptorSets(descriptorWrites, {});
            }
	    }

        void copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size) {
            vk::raii::CommandBuffer commandCopyBuffer = beginSingleTimeCommands();
            commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy{.size = size});
            endSingleTimeCommands(std::move(commandCopyBuffer));
	    }

        vk::raii::CommandBuffer beginSingleTimeCommands() {
            vk::CommandBufferAllocateInfo allocInfo{.commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1};
            vk::raii::CommandBuffer commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());

            vk::CommandBufferBeginInfo beginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
            commandBuffer.begin(beginInfo);

            return std::move(commandBuffer);
        }

        void endSingleTimeCommands(vk::raii::CommandBuffer &&commandBuffer) {
            commandBuffer.end();

            vk::SubmitInfo submitInfo{.commandBufferCount = 1, .pCommandBuffers = &*commandBuffer};
            graphicsQueue.submit(submitInfo, nullptr);
            graphicsQueue.waitIdle();
        }

        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
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


        void createCommandBuffer() {
            vk::CommandBufferAllocateInfo allocInfo{.commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = MAX_FRAMES_IN_FLIGHT};
            commandBuffers = vk::raii::CommandBuffers(device, allocInfo);
	    }

        void createSyncObjects(){
            assert(presentCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && inFlightFences.empty());

            for (size_t i = 0; i < swapChainImages.size(); i++) {
                renderFinishedSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
            }

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                presentCompleteSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
                inFlightFences.emplace_back(device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
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
            ubo.model = 
                        rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
                        rotate(glm::mat4(1.0f), time * glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
                        rotate(glm::mat4(1.0f), /*time*/ glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

            const glm::vec3 cameraDirection(
                std::cos(cameraPitch) * std::sin(cameraYaw),
                std::sin(cameraPitch),
                std::cos(cameraPitch) * std::cos(cameraYaw)
            );
            ubo.view = lookAt(cameraPosition, cameraPosition + cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f));
            ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height), 0.1f, 10.0f);

            memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	    }

        void drawFrame() {
            auto fenceResult = device.waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);
            if (fenceResult != vk::Result::eSuccess) {
                throw std::runtime_error("failed to wait for fence!");
            }

		    auto [result, imageIndex] = swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);

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
                const vk::DeviceSize captureSize = static_cast<vk::DeviceSize>(swapChainExtent.width) * swapChainExtent.height * 4;
                std::tie(captureBuffer, captureBufferMemory) = createBuffer(captureSize,
                                                                                     vk::BufferUsageFlagBits::eTransferDst,
                                                                                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            }

            device.resetFences(*inFlightFences[frameIndex]);

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
		    graphicsQueue.submit(submitInfo, *inFlightFences[frameIndex]);

            if (captureFrame) {
                auto fenceResult = device.waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);
                if (fenceResult != vk::Result::eSuccess) {
                    throw std::runtime_error("failed to wait for captured frame!");
                }
                saveFrameToPng(captureBufferMemory);
            }

		    const vk::PresentInfoKHR presentInfoKHR{
                .waitSemaphoreCount = 1, 
                .pWaitSemaphores    = &*renderFinishedSemaphores[imageIndex], 
                .swapchainCount     = 1, 
                .pSwapchains        = &*swapChain, 
                .pImageIndices      = &imageIndex
            };

		    result = graphicsQueue.presentKHR(presentInfoKHR);

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

        void saveFrameToPng(vk::raii::DeviceMemory &captureMemory) {
            const uint32_t width = swapChainExtent.width;
            const uint32_t height = swapChainExtent.height;
            const size_t pixelCount = static_cast<size_t>(width) * height;
            std::vector<unsigned char> pixels(pixelCount * 4);

            void *mapped = captureMemory.mapMemory(0, pixels.size());
            memcpy(pixels.data(), mapped, pixels.size());
            captureMemory.unmapMemory();

            const bool isBgra = swapChainSurfaceFormat.format == vk::Format::eB8G8R8A8Srgb ||
                                swapChainSurfaceFormat.format == vk::Format::eB8G8R8A8Unorm;
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
                swapChainImages[imageIndex],
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
                .resolveImageView   = swapChainImageViews[imageIndex],
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
                .renderArea           = {.offset = {0, 0}, .extent = swapChainExtent},
                .layerCount           = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments    = &colorAttachmentInfo,
                .pDepthAttachment     = &depthAttachmentInfo
            };

            commandBuffer.beginRendering(renderingInfo);
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);
            commandBuffer.bindVertexBuffers(0, *vertexBuffer, {0});
            commandBuffers[frameIndex].bindIndexBuffer(*indexBuffer, 0, vk::IndexTypeValue<decltype(indices)::value_type>::value);
            commandBuffer.setViewport(0, vk::Viewport(0.0f, static_cast<float>(swapChainExtent.height), static_cast<float>(swapChainExtent.width), -static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
            commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));
            commandBuffers[frameIndex].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, *descriptorSets[frameIndex], nullptr);
            commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), static_cast<VkCommandBuffer>(*commandBuffer));
            commandBuffer.endRendering();

            if (captureBuffer) {
                transition_image_layout(
                    swapChainImages[imageIndex],
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
                    .imageExtent = {swapChainExtent.width, swapChainExtent.height, 1}
                };
                commandBuffer.copyImageToBuffer(swapChainImages[imageIndex],
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
                    swapChainImages[imageIndex],
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
                    swapChainImages[imageIndex],
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

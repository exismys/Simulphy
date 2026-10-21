#pragma once

#include <cstdint>
#include <vulkan/vulkan_raii.hpp>

#include "Device.hpp"
#include "Swapchain.hpp"
#include "Utilities.hpp"
#include "Vertex.hpp"

namespace Simulphy {
class Pipeline {
    private:
        // vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
        vk::raii::DescriptorSetLayout cameraDescriptorSetLayout = nullptr;
        vk::raii::DescriptorSetLayout materialDescriptorSetLayout = nullptr;

        vk::raii::PipelineLayout pipelineLayout = nullptr;
        vk::raii::Pipeline graphicsPipeline = nullptr;

        vk::Format depthFormat;

    public:
        Pipeline(const Device& device, const Swapchain& swapchain) {
            createDescriptorSetLayouts(device);
            createGraphicsPipeline(device, swapchain);
        }

        const vk::Format getDepthFormat() const {
            return depthFormat;
        }

        // const vk::raii::DescriptorSetLayout& getDescriptorSetLayout() const {
        //     return descriptorSetLayout;
        // }

        const vk::raii::DescriptorSetLayout& getCameraDescriptorSetLayout() const {
            return cameraDescriptorSetLayout;
        }

        const vk::raii::DescriptorSetLayout& getMaterialDescriptorSetLayout() const {
            return materialDescriptorSetLayout;
        }

        const vk::raii::Pipeline& getGraphicsPipeline() const {
            return graphicsPipeline;
        }

        const vk::raii::PipelineLayout& getPipelineLayout() const {
            return pipelineLayout;
        }

        //----------------------------------------------------------------------
        // Define the layout of a single descriptor set
        //                        ^^^^^^^^^^^^^^^^^^^^^
        //                               `- may container multiple descriptors
        //                                  with bindings 0, 1, 2, ...
        //----------------------------------------------------------------------
        // void createDescriptorSetLayout(const Device& device) {
        //     std::array<vk::DescriptorSetLayoutBinding, 2> bindings{
        //         {
        //             {
        //                 .binding = 0,
        //                 .descriptorType = vk::DescriptorType::eUniformBuffer,
        //                 .descriptorCount = 1,
        //                 .stageFlags = vk::ShaderStageFlagBits::eVertex
        //             },
        //             {
        //                 .binding = 1,
        //                 .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        //                 .descriptorCount = 1,
        //                 .stageFlags = vk::ShaderStageFlagBits::eFragment
        //             }
        //         }
        //     };
        //     vk::DescriptorSetLayoutCreateInfo layoutInfo{
        //         .bindingCount = static_cast<uint32_t>(bindings.size()),
        //         .pBindings = bindings.data()
        //     };
        //     descriptorSetLayout = vk::raii::DescriptorSetLayout(device.getLogicalDevice(), layoutInfo);
        // }

        void createDescriptorSetLayouts(const Device& device) {
            // Set 0: camera
            vk::DescriptorSetLayoutBinding cameraBinding{
                .binding = 0,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eVertex
            };

            vk::DescriptorSetLayoutCreateInfo cameraLayoutInfo{
                .bindingCount = 1,
                .pBindings = &cameraBinding
            };

            cameraDescriptorSetLayout = vk::raii::DescriptorSetLayout(device.getLogicalDevice(), cameraLayoutInfo);

            // Set 1: Material
            vk::DescriptorSetLayoutBinding materialBinding{
                .binding = 0,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eFragment
            };

            vk::DescriptorSetLayoutCreateInfo materialLayoutInfo{
                .bindingCount = 1,
                .pBindings = &materialBinding
            };

            materialDescriptorSetLayout = vk::raii::DescriptorSetLayout(device.getLogicalDevice(), materialLayoutInfo);

        }

        void createGraphicsPipeline(const Device& device, const Swapchain& swapchain) {
            vk::raii::ShaderModule shaderModule = createShaderModule(device, readFile("shaders/slang.spv"));
            vk::PipelineShaderStageCreateInfo vertShaderStageInfo{.stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModule, .pName = "vertMain"};
            vk::PipelineShaderStageCreateInfo fragShaderStageInfo{.stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModule, .pName = "fragMain"};
            vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

            auto bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();

            vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
                .vertexBindingDescriptionCount = 1,
                .pVertexBindingDescriptions = &bindingDescription,
                .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
                .pVertexAttributeDescriptions = attributeDescriptions.data()
            };
            
            vk::PipelineInputAssemblyStateCreateInfo inputAssembly{.topology = vk::PrimitiveTopology::eTriangleList};
            vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1, .scissorCount = 1};

            vk::PipelineRasterizationStateCreateInfo rasterizer{
                .depthClampEnable        = vk::False,
                .rasterizerDiscardEnable = vk::False,
                .polygonMode             = vk::PolygonMode::eFill,
                .cullMode                = vk::CullModeFlagBits::eBack,
                .frontFace               = vk::FrontFace::eCounterClockwise,
                .depthBiasEnable         = vk::False,
                .lineWidth               = 1.0f
            };

            vk::PipelineMultisampleStateCreateInfo multisampling{
                .rasterizationSamples = device.getMsaaSampleCount(), 
                .sampleShadingEnable  = vk::False
            };

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
                .logicOpEnable   = vk::False, 
                .logicOp         = vk::LogicOp::eCopy, 
                .attachmentCount = 1, 
                .pAttachments    = &colorBlendAttachment
            };

            std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
            vk::PipelineDynamicStateCreateInfo dynamicState{
                .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), 
                .pDynamicStates    = dynamicStates.data()
            };

            std::array<vk::DescriptorSetLayout, 2> setLayouts{
                *cameraDescriptorSetLayout,
                *materialDescriptorSetLayout
            };
            vk::PushConstantRange modelPushConstantRange{
                .stageFlags = vk::ShaderStageFlagBits::eVertex,
                .offset = 0,
                .size = sizeof(glm::mat4)
            };
            vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
                .setLayoutCount         = static_cast<uint32_t>(setLayouts.size()), 
                .pSetLayouts            = setLayouts.data(), 
                .pushConstantRangeCount = 1,
                .pPushConstantRanges    = &modelPushConstantRange
            };
            pipelineLayout = vk::raii::PipelineLayout(device.getLogicalDevice(), pipelineLayoutInfo);

            vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{ 
                .colorAttachmentCount    = 1, 
                .pColorAttachmentFormats = &swapchain.getSwapchainSurfaceFormat().format 
            };

            depthFormat = findDepthFormat(device);

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
                    .pColorAttachmentFormats = &swapchain.getSwapchainSurfaceFormat().format,
                    .depthAttachmentFormat = depthFormat
                }
            };

            graphicsPipeline = vk::raii::Pipeline(device.getLogicalDevice(), nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
        }

        [[nodiscard]] vk::raii::ShaderModule createShaderModule(const Device& device, const std::vector<char> &code) const {
            vk::ShaderModuleCreateInfo createInfo{.codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t *>(code.data())};
            vk::raii::ShaderModule shaderModule{device.getLogicalDevice(), createInfo};

            return shaderModule;
	    }

        vk::Format findDepthFormat(const Device& device) {
            return findSupportedFormat(device,
                                       {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                                       vk::ImageTiling::eOptimal,
                                       vk::FormatFeatureFlagBits::eDepthStencilAttachment);
        }

        vk::Format findSupportedFormat(const Device& device, 
                                       const std::vector<vk::Format> &candidates, 
                                       vk::ImageTiling tiling, 
                                       vk::FormatFeatureFlags features) {
            for (const auto format : candidates) {
                vk::FormatProperties props = device.getPhysicalDevice().getFormatProperties(format);
                if (
                    ((tiling == vk::ImageTiling::eLinear) && ((props.linearTilingFeatures & features) == features)) ||
                    ((tiling == vk::ImageTiling::eOptimal) && ((props.optimalTilingFeatures & features) == features))
                ) {
                    return format;
                }
            }
            throw std::runtime_error("failed to find supported format!");
        }

};
}
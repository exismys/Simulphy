#pragma once

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Vertex.hpp"
#include "Device.hpp"
#include "Utilities.hpp"

const std::string MODEL_PATH = "models/test-models/MenlogateLow.obj";

namespace Simulphy {

class Mesh {
    private:
        std::vector<Simulphy::Vertex> vertices;
        std::vector<uint32_t> indices;
        vk::raii::Buffer vertexBuffer  = nullptr;
	    vk::raii::DeviceMemory vertexBufferMemory = nullptr;
        vk::raii::Buffer indexBuffer = nullptr;
        vk::raii::DeviceMemory indexBufferMemory = nullptr;

    public:
        Mesh(const Device& device) {
            loadModel(MODEL_PATH);
            createVertexBuffer(device);
            createIndexBuffer(device);
        }

        const vk::raii::Buffer& getVertexBuffer() const {
            return vertexBuffer;
        }

        const vk::raii::Buffer& getIndexBuffer() const {
            return indexBuffer;
        }

        const std::vector<uint32_t>& getIndices() const {
            return indices;
        }

        const std::vector<Simulphy::Vertex>& getVertices() const {
            return vertices;
        }

        void loadModel(const std::string modelPath) {
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
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

        void createVertexBuffer(const Device& device) {
            vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

            auto [stagingBuffer, stagingBufferMemory] = device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

            void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
            memcpy(dataStaging, vertices.data(), bufferSize);
            stagingBufferMemory.unmapMemory();

            std::tie(vertexBuffer, vertexBufferMemory) = device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);

            copyBuffer(device, stagingBuffer, vertexBuffer, bufferSize);
	    }

        void createIndexBuffer(const Device& device) {
            vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

            auto [stagingBuffer, stagingBufferMemory] = device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

            void *data = stagingBufferMemory.mapMemory(0, bufferSize);
            memcpy(data, indices.data(), (size_t) bufferSize);
            stagingBufferMemory.unmapMemory();

            std::tie(indexBuffer, indexBufferMemory) = device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);

            copyBuffer(device, stagingBuffer, indexBuffer, bufferSize);
        }

        void createCubeMesh() {
            vertices = {
                {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
                {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
                {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
                {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

                {{-0.5f, -0.5f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
                {{0.5f, -0.5f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
                {{0.5f, 0.5f, -1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
                {{-0.5f, 0.5f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}

            };

            indices = {
                0, 1, 2, 2, 3, 0,
                4, 7, 6, 6, 5, 4,

                4, 0, 3, 3, 7, 4,
                1, 5, 6, 6, 2, 1,

                4, 5, 1, 1, 0, 4,
                3, 2, 6, 6, 7, 3
            };
        }
};
}

#pragma once

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

#include "renderer/ResourceAllocator.h"
#include "profiler/StatsManager.h"

#include "VulkanUploadContext.h"

namespace liquid {

class VulkanResourceAllocator : public ResourceAllocator {
public:
  /**
   * @brief Constructor
   *
   * Sets allocator for creating buffers
   *
   * @param uploadContext Upload context
   * @param allocator Vma Allocator
   * @param device Vulkan device
   * @param statsManager Stats manager
   */
  VulkanResourceAllocator(const VulkanUploadContext &uploadContext,
                          VmaAllocator allocator, VkDevice device,
                          const SharedPtr<StatsManager> &statsManager);

  /**
   * @brief Create empty vertex buffer with size
   *
   * @param size Size
   * @return Vertex buffer
   */
  HardwareBuffer *createVertexBuffer(size_t size) override;

  /**
   * @brief Creates vertex buffer from vertices
   *
   * @param vertices List of vertices
   * @return Vertex buffer
   */
  HardwareBuffer *
  createVertexBuffer(const std::vector<Vertex> &vertices) override;

  /**
   * @brief Create empty index buffer with size
   *
   * @param size Size
   * @return Index buffer
   */
  HardwareBuffer *createIndexBuffer(size_t size) override;

  /**
   * @brief Creates index buffer from indices
   *
   * @param vertices List of indices
   * @return Index buffer
   */
  HardwareBuffer *
  createIndexBuffer(const std::vector<uint32_t> &indices) override;

  /**
   * @brief Create uniform buffer from arbitrary data
   *
   * @param bufferSize Buffer size
   * @return Uniform buffer
   */
  HardwareBuffer *createUniformBuffer(size_t bufferSize) override;

  /**
   * @brief Create 2D texture from incoming data
   *
   * @param data Texture data
   * @return 2D texture
   */
  SharedPtr<Texture> createTexture2D(const TextureData &data) override;

  /**
   * @brief Create cubemap texture from incoming data
   *
   * @param data Texture data
   * @return Cubemap texture
   */
  SharedPtr<Texture>
  createTextureCubemap(const TextureCubemapData &data) override;

  /**
   * @brief Create texture that is filled by framebuffer
   *
   * @param data Framebuffer texture data
   * @return Framebuffer texture
   */
  SharedPtr<Texture>
  createTextureFramebuffer(const TextureFramebufferData &data) override;

private:
  const VulkanUploadContext &uploadContext;
  VmaAllocator allocator;
  VkDevice device;
  SharedPtr<StatsManager> statsManager;
};

} // namespace liquid

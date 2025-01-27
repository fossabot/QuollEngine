#pragma once

#include "quoll/rhi/RenderHandle.h"

namespace quoll::rhi {

class VulkanBuffer;
class VulkanTexture;
class VulkanTextureView;
class VulkanRenderPass;
class VulkanFramebuffer;
class VulkanPipeline;
class VulkanShader;
class VulkanSampler;

/**
 * @brief Vulkan resource registry
 *
 * Stores all the resources associated
 * with a device
 */
class VulkanResourceRegistry {

  /**
   * @brief Resource map for Vulkan resources
   *
   * @tparam THandle Handle
   * @tparam TResource Resource
   */
  template <class THandle, class TResource> struct ResourceMap {
    /**
     * @brief Map type
     */
    using Map = std::unordered_map<THandle, std::unique_ptr<TResource>>;

    /**
     * @brief Map of handles and resources
     */
    Map map;

    /**
     * @brief Last handle
     *
     * Used for auto generation
     */
    u32 lastHandle = 1;
  };

  using ShaderMap = ResourceMap<ShaderHandle, VulkanShader>;
  using BufferMap = ResourceMap<BufferHandle, VulkanBuffer>;
  using TextureMap = ResourceMap<TextureHandle, VulkanTexture>;
  using SamplerMap = ResourceMap<SamplerHandle, VulkanSampler>;
  using RenderPassMap = ResourceMap<RenderPassHandle, VulkanRenderPass>;
  using FramebufferMap = ResourceMap<FramebufferHandle, VulkanFramebuffer>;
  using PipelineMap = ResourceMap<PipelineHandle, VulkanPipeline>;

public:
  /**
   * @brief Set shader
   *
   * @param shader Vulkan shader
   * @param handle Shader handle
   */
  void setShader(std::unique_ptr<VulkanShader> &&shader, ShaderHandle handle);

  /**
   * @brief Delete shader
   *
   * @param handle Shader handle
   */
  void deleteShader(ShaderHandle handle);

  /**
   * @brief Get shaders
   *
   * @return List of shaders
   */
  inline const ShaderMap::Map &getShaders() const { return mShaders.map; }

  /**
   * @brief Set buffer
   *
   * @param buffer Vulkan buffer
   * @return New buffer handle
   */
  BufferHandle setBuffer(std::unique_ptr<VulkanBuffer> &&buffer);

  /**
   * @brief Delete buffer
   *
   * @param handle Buffer handle
   */
  void deleteBuffer(BufferHandle handle);

  /**
   * @brief Check if buffer exists
   *
   * @param handle Buffer handle
   * @retval true Buffer exists
   * @retval false Buffer does not exist
   */
  inline bool hasBuffer(BufferHandle handle) const {
    return mBuffers.map.find(handle) != mBuffers.map.end();
  }

  /**
   * @brief Get buffers
   *
   * @return List of buffers
   */
  inline const BufferMap::Map &getBuffers() const { return mBuffers.map; }

  /**
   * @brief Set texture
   *
   * @param texture Vulkan texture
   * @param handle Texture handle
   */
  void setTexture(std::unique_ptr<VulkanTexture> &&texture,
                  TextureHandle handle);

  /**
   * @brief Delete texture
   *
   * @param handle Texture handle
   */
  void deleteTexture(TextureHandle handle);

  /**
   * @brief Get textures
   *
   * @return List of textures
   */
  inline const TextureMap::Map &getTextures() const { return mTextures.map; }

  /**
   * @brief Set sampler
   *
   * @param sampler Vulkan sampler
   * @param handle Sampler handle
   */
  void setSampler(std::unique_ptr<VulkanSampler> &&sampler,
                  SamplerHandle handle);

  /**
   * @brief Delete sampler
   *
   * @param handle Sampler handle
   */
  void deleteSampler(SamplerHandle handle);

  /**
   * @brief Get samplers
   *
   * @return List of samplers
   */
  inline const SamplerMap::Map &getSamplers() const { return mSamplers.map; }

  /**
   * @brief Set render pass
   *
   * @param renderPass Vulkan render pass
   * @param handle Render pass handle
   */
  void setRenderPass(std::unique_ptr<VulkanRenderPass> &&renderPass,
                     RenderPassHandle handle);

  /**
   * @brief Delete render pass
   *
   * @param handle Render pass handle
   */
  void deleteRenderPass(rhi::RenderPassHandle handle);

  /**
   * @brief Get render passes
   *
   * @return List of render passes
   */
  inline const RenderPassMap::Map &getRenderPasses() const {
    return mRenderPasses.map;
  }

  /**
   * @brief Set framebuffer
   *
   * @param framebuffer Vulkan framebuffer
   * @param handle Framebuffer handle
   */
  void setFramebuffer(std::unique_ptr<VulkanFramebuffer> &&framebuffer,
                      FramebufferHandle handle);

  /**
   * @brief Delete framebuffer
   *
   * @param handle Framebuffer handle
   */
  void deleteFramebuffer(rhi::FramebufferHandle handle);

  /**
   * @brief Get framebuffers
   *
   * @return List of framebuffers
   */
  inline const FramebufferMap::Map &getFramebuffers() const {
    return mFramebuffers.map;
  }

  /**
   * @brief Set pipeline
   *
   * @param pipeline Vulkan pipeline
   * @param handle Pipeline handle
   */
  void setPipeline(std::unique_ptr<VulkanPipeline> &&pipeline,
                   PipelineHandle handle);

  /**
   * @brief Delete pipeline
   *
   * @param handle Pipeline handle
   */
  void deletePipeline(PipelineHandle handle);

  /**
   * @brief Check if pipeline exists in registry
   *
   * @param handle Pipeline handle
   * @retval true Pipeline exists
   * @retval false Pipeline does not exist
   */
  inline bool hasPipeline(PipelineHandle handle) const {
    return mPipelines.map.find(handle) != mPipelines.map.end();
  }

  /**
   * @brief Get pipelines
   *
   * @return List of pipelines
   */
  inline const PipelineMap::Map &getPipelines() const { return mPipelines.map; }

private:
  BufferMap mBuffers;
  TextureMap mTextures;
  SamplerMap mSamplers;
  ShaderMap mShaders;
  RenderPassMap mRenderPasses;
  FramebufferMap mFramebuffers;
  PipelineMap mPipelines;
};

} // namespace quoll::rhi

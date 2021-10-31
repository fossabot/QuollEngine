#pragma once

#include "core/Base.h"
#include "RenderCommand.h"

namespace liquid {

class RenderCommandList {
public:
  /**
   * @brief Begin render pass
   *
   * @param renderPass Render pass
   * @param framebuffer Framebuffer
   * @param renderAreaOffset Render area offset
   * @param renderAreaSize Render area size
   * @param clearValues Clear values
   */
  void beginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer,
                       const glm::ivec2 &renderAreaOffset,
                       const glm::uvec2 &renderAreaSize,
                       const std::vector<VkClearValue> &clearValues);

  /**
   * @brief End render pass
   */
  void endRenderPass();

  /**
   * @brief Bind pipeline
   *
   * @param pipeline Pipeline
   * @param bindPoint Pipeline bind point
   */
  void bindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint);

  /**
   * @brief Bind descriptor sets
   *
   * @param pipelineLayout Pipeline layout
   * @param bindPoint Pipeline bind point
   * @param firstSet First set
   * @param descriptorSets Descriptor sets
   * @param dynamicOffsets Dynamic offsets
   */
  void bindDescriptorSets(VkPipelineLayout pipelineLayout,
                          VkPipelineBindPoint bindPoint, uint32_t firstSet,
                          const std::vector<VkDescriptorSet> &descriptorSets,
                          const std::vector<uint32_t> &dynamicOffsets);

  /**
   * @brief Bind vertex buffer
   *
   * @param buffer Vertex buffer
   */
  void bindVertexBuffer(const SharedPtr<HardwareBuffer> &buffer);

  /**
   * @brief Bind index buffer
   *
   * @param buffer Index buffer
   * @param indexType Index buffer data type
   */
  void bindIndexBuffer(const SharedPtr<HardwareBuffer> &buffer,
                       VkIndexType indexType);

  /**
   * @brief Push constants
   *
   * @param pipelineLayout Pipeline layout
   * @param stageFlags Stage flags
   * @param offset Offset
   * @param size Size
   * @param data Data
   */
  void pushConstants(VkPipelineLayout pipelineLayout,
                     VkShaderStageFlags stageFlags, uint32_t offset,
                     uint32_t size, void *data);

  /**
   * @brief Draw
   *
   * @param vertexCount Vertex count
   * @param firstVertex First vertex
   */
  void draw(size_t vertexCount, uint32_t firstVertex);

  /**
   * @brief Draw indexed
   *
   * @param indexCount Index count
   * @param firstIndex Offset of first index
   * @param vertexOffset Vertex offset
   */
  void drawIndexed(size_t indexCount, uint32_t firstIndex,
                   int32_t vertexOffset);

  /**
   * @brief Set viewport
   *
   * @param offset Viewport offset
   * @param size Viewport size
   * @param depthRange Viewport depth range
   */
  void setViewport(const glm::vec2 &offset, const glm::vec2 &size,
                   const glm::vec2 &depthRange);

  /**
   * @brief Set scissor
   *
   * @param offset Scissor offset
   * @param size Scissor size
   */
  void setScissor(const glm::vec2 &offset, const glm::vec2 &size);

  /**
   * @brief Get recorded commands
   *
   * @return Recorded commands
   */
  inline const std::vector<std::unique_ptr<RenderCommandBase>> &
  getRecordedCommands() const {
    return commands;
  }

private:
  /**
   * @brief Record command
   *
   * @param command Command
   */
  void record(RenderCommandBase *command);

private:
  std::vector<std::unique_ptr<RenderCommandBase>> commands;
};

} // namespace liquid

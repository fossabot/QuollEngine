#pragma once

#include "quoll/window/Window.h"

#include "quoll/rhi/RenderCommandList.h"
#include "quoll/renderer/RenderGraph.h"
#include "quoll/rhi/RenderDevice.h"
#include "quoll/renderer/RenderStorage.h"
#include "quoll/renderer/RendererOptions.h"

#include "quoll/imgui/Imgui.h"

namespace quoll {

/**
 * @brief Imgui render pass data
 */
struct ImguiRenderPassData {
  /**
   * Imgui pass
   */
  RenderGraphPass &pass;

  /**
   * Imgui texture
   */
  RenderGraphResource<rhi::TextureHandle> imguiColor;
};

/**
 * @brief Imgui renderer
 */
class ImguiRenderer {
  /**
   * @brief Imgui frame data
   */
  struct FrameData {
    /**
     * Vertex buffer
     */
    rhi::Buffer vertexBuffer;

    /**
     * Vertex buffer size
     */
    usize vertexBufferSize = 0;

    /**
     * Vertex buffer data
     */
    void *vertexBufferData = nullptr;

    /**
     * Index buffer
     */
    rhi::Buffer indexBuffer;

    /**
     * Index buffer size
     */
    usize indexBufferSize = 0;

    /**
     * Index buffer data
     */
    void *indexBufferData = nullptr;
  };

  static constexpr const glm::vec4 DefaultClearColor{0.0f, 0.0f, 0.0f, 1.0f};

public:
  /**
   * @brief Create imgui renderer
   *
   * @param window Window
   * @param renderStorage Render storage
   */
  ImguiRenderer(Window &window, RenderStorage &renderStorage);

  /**
   * @brief Destroy imgui renderer
   */
  ~ImguiRenderer();

  ImguiRenderer(const ImguiRenderer &rhs) = delete;
  ImguiRenderer(ImguiRenderer &&rhs) = delete;
  ImguiRenderer &operator=(const ImguiRenderer &rhs) = delete;
  ImguiRenderer &operator=(ImguiRenderer &&rhs) = delete;

  /**
   * @brief Attach render passes to render graph
   *
   * @param graph Render graph
   * @param options Renderer options
   * @return Imgui render pass data
   */
  ImguiRenderPassData attach(RenderGraph &graph,
                             const RendererOptions &options);

  /**
   * @brief Set clear color
   *
   * @param clearColor Clear color
   */
  void setClearColor(const glm::vec4 &clearColor);

  /**
   * @brief Build font atlas
   */
  void buildFonts();

  /**
   * @brief Begin imgui rendering
   */
  void beginRendering();

  /**
   * @brief End imgui rendering
   *
   * Prepares all the buffers for drawing
   */
  void endRendering();

  /**
   * @brief Update frame data
   *
   * @param frameIndex
   */
  void updateFrameData(u32 frameIndex);

  /**
   * @brief Send imgui data to command list
   *
   * @param commandList Command list
   * @param pipeline Pipeline
   * @param frameIndex Frame index
   */
  void draw(rhi::RenderCommandList &commandList, rhi::PipelineHandle pipeline,
            u32 frameIndex);

private:
  /**
   * @brief Setup remder states
   *
   * @param data Imgui data
   * @param commandList Command list
   * @param fbWidth Framebuffer width
   * @param fbHeight Framebuffer height
   * @param pipeline Pipeline
   * @param frameIndex Frame index
   */
  void setupRenderStates(ImDrawData *data, rhi::RenderCommandList &commandList,
                         int fbWidth, int fbHeight,
                         rhi::PipelineHandle pipeline, u32 frameIndex);

private:
  RenderStorage &mRenderStorage;
  rhi::TextureHandle mFontTexture = rhi::TextureHandle::Null;

  std::array<FrameData, rhi::RenderDevice::NumFrames> mFrameData;

  glm::vec4 mClearColor{DefaultClearColor};

  bool mReady = false;

  rhi::RenderDevice *mDevice;
};

} // namespace quoll

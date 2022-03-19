#pragma once

#include <imgui.h>

#include "liquid/window/Window.h"

#include "liquid/rhi/RenderCommandList.h"
#include "liquid/rhi/ResourceRegistry.h"

namespace liquid {

class ImguiRenderer {
  struct FrameData {
    rhi::BufferHandle vertexBuffer = rhi::BufferHandle::Invalid;
    size_t vertexBufferSize = 0;
    void *vertexBufferData = nullptr;

    rhi::BufferHandle indexBuffer = rhi::BufferHandle::Invalid;
    void *indexBufferData = nullptr;
    size_t indexBufferSize = 0;

    bool firstTime = true;
  };

public:
  ImguiRenderer(Window &window, rhi::ResourceRegistry &registry);
  ~ImguiRenderer();

  ImguiRenderer(const ImguiRenderer &rhs) = delete;
  ImguiRenderer(ImguiRenderer &&rhs) = delete;
  ImguiRenderer &operator=(const ImguiRenderer &rhs) = delete;
  ImguiRenderer &operator=(ImguiRenderer &&rhs) = delete;

  static void beginRendering();
  static void endRendering();

  void draw(rhi::RenderCommandList &commandList, rhi::PipelineHandle pipeline);

private:
  void loadFonts();

  void setupRenderStates(ImDrawData *draw_data,
                         rhi::RenderCommandList &commandList, int fbWidth,
                         int fbHeight, rhi::PipelineHandle pipeline);

private:
  rhi::ResourceRegistry &mRegistry;

  rhi::TextureHandle mFontTexture = rhi::TextureHandle::Invalid;

  std::vector<FrameData> mFrameData;
  uint32_t mCurrentFrame = 0;
};

} // namespace liquid

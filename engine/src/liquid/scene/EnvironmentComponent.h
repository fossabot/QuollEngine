#pragma once

namespace liquid {

struct EnvironmentComponent {
  rhi::TextureHandle irradianceMap = rhi::TextureHandle::Invalid;
  rhi::TextureHandle specularMap = rhi::TextureHandle::Invalid;
  rhi::TextureHandle brdfLUT = rhi::TextureHandle::Invalid;
};

} // namespace liquid

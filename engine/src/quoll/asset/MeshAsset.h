#pragma once

#include "quoll/rhi/RenderHandle.h"
#include "quoll/rhi/Buffer.h"

#include "Asset.h"

namespace quoll {

/**
 * @brief Base geometry asset data
 */
struct BaseGeometryAsset {
  /**
   * Positions
   */
  std::vector<glm::vec3> positions;

  /**
   * Normals
   */
  std::vector<glm::vec3> normals;

  /**
   * Tangents
   */
  std::vector<glm::vec4> tangents;

  /**
   * Texture coordinates for index 0
   */
  std::vector<glm::vec2> texCoords0;

  /**
   * Texture coordinates for index 1
   */
  std::vector<glm::vec2> texCoords1;

  /**
   * Joints
   */
  std::vector<glm::uvec4> joints;

  /**
   * Weights
   */
  std::vector<glm::vec4> weights;

  /**
   * List of indices
   */
  std::vector<u32> indices;
};

/**
 * @brief Mesh asset data
 */
struct MeshAsset {
  /**
   * List of geometries
   */
  std::vector<BaseGeometryAsset> geometries;

  /**
   * @brief Vertex buffers for all geometries
   */
  std::vector<rhi::BufferHandle> vertexBuffers;

  /**
   * Vertex buffer binding offsets
   */
  std::vector<u64> vertexBufferOffsets;

  /**
   * @brief Index buffer for all geometries
   */
  rhi::BufferHandle indexBuffer = rhi::BufferHandle::Null;
};

} // namespace quoll

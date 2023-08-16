#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTextureCoord0;
layout(location = 4) in vec2 inTextureCoord1;

layout(location = 0) out vec3 outWorldPosition;
layout(location = 1) out vec2 outTextureCoord[2];
layout(location = 3) out mat3 outTBN;
layout(location = 6) out uint outMaterialIndex;

#include "bindless/base.glsl"
#include "bindless/mesh.glsl"
#include "bindless/camera.glsl"
#include "bindless/material.glsl"

layout(set = 0, binding = 0) uniform sampler2D uGlobalTextures[];
layout(set = 0, binding = 1) writeonly uniform image2D uGlobalImages[];

layout(set = 1, binding = 0) uniform DrawParameters {
  MaterialsArray materials;
  TransformsArray meshTransforms;
  MaterialRangeArray meshMaterialRanges;
  TransformsArray skinnedMeshTransforms;
  MaterialRangeArray skinnedMeshMaterialRanges;
  SkeletonsArray skeletons;
  Camera camera;
  Empty scene;
  Empty directionalLights;
  Empty pointLights;
  Empty shadows;
}
uDrawParams;

layout(std430, push_constant) uniform PushConstants { uint submeshIndex; }
uMeshParams;

void main() {
  mat4 modelMatrix = getMeshTransform(gl_InstanceIndex).modelMatrix;

  vec4 worldPosition = modelMatrix * vec4(inPosition, 1.0f);

  mat4 normalMatrix = transpose(inverse(modelMatrix));

  vec3 normal = normalize(vec3(normalMatrix * vec4(inNormal.xyz, 0.0)));
  vec3 tangent = normalize(vec3(modelMatrix * vec4(inTangent.xyz, 0.0)));
  vec3 bitangent = normalize(cross(normal, tangent));

  outWorldPosition = worldPosition.xyz;
  outTBN = mat3(tangent, bitangent, normal);
  outTextureCoord[0] = inTextureCoord0;
  outTextureCoord[1] = inTextureCoord1;
  gl_Position = getCamera().viewProj * worldPosition;

  outMaterialIndex =
      min(uDrawParams.meshMaterialRanges.items[gl_InstanceIndex].start +
              uMeshParams.submeshIndex,
          uDrawParams.meshMaterialRanges.items[gl_InstanceIndex].end);
}

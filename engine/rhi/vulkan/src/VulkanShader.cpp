#include "quoll/core/Base.h"
#include "quoll/core/Engine.h"

#include "VulkanShader.h"
#include "VulkanError.h"
#include "VulkanMapping.h"

#define SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
#include <spirv_reflect.h>

namespace quoll::rhi {

VulkanShader::VulkanShader(const ShaderDescription &description,
                           VulkanDeviceObject &device)
    : mDevice(device), mPath(description.path) {
  const auto &shaderBytes = VulkanShader::readShaderFile(description.path);

  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.flags = 0;
  createInfo.pNext = nullptr;

  createInfo.codeSize = shaderBytes.size();
  createInfo.pCode = reinterpret_cast<const u32 *>(shaderBytes.data());

  checkForVulkanError(
      vkCreateShaderModule(device, &createInfo, nullptr, &mShaderModule),
      "Failed to create shader module from \"" + mPath.filename().string() +
          "\"");

  auto debugName =
      std::filesystem::relative(mPath, std::filesystem::current_path())
          .string();

  device.setObjectName(debugName, VK_OBJECT_TYPE_SHADER_MODULE, mShaderModule);

  Engine::getLogger().info() << "[VK] Shader loaded: \"" + debugName + "\"";

  createReflectionInfo(shaderBytes);
}

VulkanShader::~VulkanShader() {
  if (mShaderModule) {
    vkDestroyShaderModule(mDevice, mShaderModule, nullptr);

    Engine::getLogger().info()
        << "[VK] Shader unloaded: \"" +
               std::filesystem::relative(mPath, std::filesystem::current_path())
                   .string() +
               "\"";
  }
}

std::vector<char> VulkanShader::readShaderFile(const Path &filepath) {
  std::ifstream file(filepath, std::ios::ate | std::ios::binary);

  QuollAssert(file.is_open(),
              "Failed to open shader file \"" + filepath.string() + "\"");

  std::streamsize fileSize = file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}

void VulkanShader::createReflectionInfo(const std::vector<char> &bytes) {
  SpvReflectShaderModule shaderReflectModule;
  SpvReflectResult result = spvReflectCreateShaderModule(
      bytes.size(), bytes.data(), &shaderReflectModule);

  QuollAssert(result == SPV_REFLECT_RESULT_SUCCESS,
              "Failed to read reflection data from shader " +
                  mPath.filename().string());

  mStage = static_cast<VkShaderStageFlagBits>(shaderReflectModule.shader_stage);

  // Push constants
  {
    u32 count = 0;
    spvReflectEnumeratePushConstantBlocks(&shaderReflectModule, &count,
                                          nullptr);

    if (count > 0) {
      std::vector<SpvReflectBlockVariable *> blocks(count, nullptr);
      spvReflectEnumeratePushConstantBlocks(&shaderReflectModule, &count,
                                            &blocks.at(0));

      mReflectionData.pushConstantRanges.reserve(count);

      for (auto &blk : blocks) {
        const SpvReflectBlockVariable &reflectBlock = *blk;
        VkPushConstantRange range{};
        range.offset = reflectBlock.offset;
        range.size = reflectBlock.size;
        range.stageFlags = mStage;

        mReflectionData.pushConstantRanges.push_back(range);
      }
    }
  }

  // Descriptor layouts
  {
    u32 count = 0;
    spvReflectEnumerateDescriptorSets(&shaderReflectModule, &count, nullptr);

    if (count > 0) {
      std::vector<SpvReflectDescriptorSet *> descriptors(count, nullptr);
      spvReflectEnumerateDescriptorSets(&shaderReflectModule, &count,
                                        &descriptors.at(0));

      for (auto &ds : descriptors) {
        const SpvReflectDescriptorSet &reflectDescriptorSet = *ds;

        DescriptorLayoutDescription description{};

        std::map<u32, DescriptorLayoutBindingDescription> bindingsMap;

        for (u32 i = 0; i < reflectDescriptorSet.binding_count; ++i) {
          // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)

          auto *reflectBinding = reflectDescriptorSet.bindings[i];
          String name(reflectBinding->name);
          if (bindingsMap.find(reflectBinding->binding) != bindingsMap.end() &&
              name.rfind("uGlobal") != 0) {
            continue;
          }

          DescriptorLayoutBindingDescription layoutBinding{};

          layoutBinding.name = name;
          layoutBinding.binding = reflectBinding->binding;
          layoutBinding.shaderStage = VulkanMapping::getShaderStage(mStage);
          layoutBinding.descriptorType = VulkanMapping::getDescriptorType(
              static_cast<VkDescriptorType>(reflectBinding->descriptor_type));
          layoutBinding.descriptorCount = 1;

          for (u32 j = 0; j < reflectBinding->array.dims_count; ++j) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            layoutBinding.descriptorCount *= reflectBinding->array.dims[j];
          }

          bindingsMap.insert_or_assign(reflectBinding->binding, layoutBinding);
        }

        for (auto &[_, binding] : bindingsMap) {
          description.bindings.push_back(binding);
        }

        mReflectionData.descriptorLayouts.insert({ds->set, description});
      }
    }
  }

  spvReflectDestroyShaderModule(&shaderReflectModule);
}

} // namespace quoll::rhi

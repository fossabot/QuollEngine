#pragma once

#include "quoll/asset/Asset.h"
#include "InputDataType.h"

namespace quoll {

/**
 * @brief Input map asset reference
 *
 * Stores reference to the asset
 */
struct InputMapAssetRef {
  /**
   * Asset handle
   */
  InputMapAssetHandle handle = InputMapAssetHandle::Null;

  /**
   * Default scheme
   */
  usize defaultScheme = 0;
};

/**
 * @brief Binding table for input map
 */
struct InputMapBindingTable {
  /**
   * Input key to command map
   */
  std::unordered_map<int, usize> inputKeyToCommandMap;

  /**
   * Used to identify what part of a value
   * the key represents
   *
   * Used non-boolean value types
   */
  std::unordered_map<int, InputDataTypeField> inputKeyFields;
};

/**
 * @brief Input map component
 */
struct InputMap {
  /**
   * Data types representing the commands
   */
  std::vector<InputDataType> commandDataTypes;

  /**
   * Command name to internal command key map
   */
  std::unordered_map<String, usize> commandNameMap;

  /**
   * Command values
   */
  std::vector<std::variant<bool, glm::vec2>> commandValues;

  /**
   * Binding tables based on scheme
   */
  std::vector<InputMapBindingTable> schemes;

  /**
   * Scheme name to internal scheme key map
   */
  std::unordered_map<String, usize> schemeNameMap;

  /**
   * Currently active scheme
   */
  usize activeScheme = 0;
};

} // namespace quoll

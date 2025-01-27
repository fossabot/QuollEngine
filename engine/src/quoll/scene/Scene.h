#pragma once

#include "quoll/entity/EntityDatabase.h"

namespace quoll {

/**
 * @brief Scene structure
 *
 * Stores entities and metadata about the scene
 */
struct Scene {
  /**
   * Entity database
   */
  EntityDatabase entityDatabase;

  /**
   * Active camera in the scene
   */
  Entity activeCamera = Entity::Null;

  /**
   * Dummy camera
   *
   * Used as a fallback if there
   * are no cameras in the scene
   */
  Entity dummyCamera = Entity::Null;

  /**
   * Environment
   */
  Entity activeEnvironment = Entity::Null;

  /**
   * Dummy environment
   *
   * Used as a fallback if there are no
   * environments in the scene
   */
  Entity dummyEnvironment = Entity::Null;
};

} // namespace quoll

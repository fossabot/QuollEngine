#pragma

#include "quoll/asset/AssetRegistry.h"
#include "quoll/asset/Result.h"
#include "quoll/yaml/Yaml.h"
#include "quoll/entity/EntityDatabase.h"

namespace quoll::detail {

/**
 * @brief Serialize entity for writing
 */
class EntitySerializer {
public:
  /**
   * @brief Create serializer
   *
   * @param assetRegistry Asset registry
   * @param entityDatabase Entity database
   */
  EntitySerializer(AssetRegistry &assetRegistry,
                   EntityDatabase &entityDatabase);

  /**
   * @brief Serialize entity
   *
   * @param entity Entity
   * @return YAML node on success, error on failure
   */
  Result<YAML::Node> serialize(Entity entity);

  /**
   * @brief Create YAML node for entity components
   *
   * @param entity Entity
   * @return YAML node for entity components
   */
  YAML::Node createComponentsNode(Entity entity);

private:
  AssetRegistry &mAssetRegistry;
  EntityDatabase &mEntityDatabase;
};

} // namespace quoll::detail

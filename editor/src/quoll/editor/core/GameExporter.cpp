#include "quoll/core/Base.h"
#include "quoll/core/Engine.h"
#include "quoll/yaml/Yaml.h"

#include "GameExporter.h"

namespace quoll::editor {

void GameExporter::exportGame(const Project &project, const Path &destination) {
  using co = std::filesystem::copy_options;

  auto gameName = destination.filename();

  if (gameName.empty()) {
    return;
  }

  auto destinationAssetsPath = destination / project.assetsPath.filename();

  // Copy game data to destination
  std::filesystem::create_directory(destination);
  std::filesystem::copy(project.assetsCachePath, destinationAssetsPath,
                        co::overwrite_existing | co::recursive);

  // Copy engine data
  auto enginePath = Engine::getEnginePath();
  std::filesystem::copy(enginePath, destination / enginePath.filename(),
                        co::overwrite_existing | co::recursive);

  // Copy runtime
  Path runtimePath;

  for (auto entry :
       std::filesystem::directory_iterator(std::filesystem::current_path())) {
    if (entry.is_regular_file() &&
        entry.path().stem().string() == "QuollRuntime") {
      runtimePath = entry.path();
    }
  }

  auto gameExecutable = destination / gameName;
  gameExecutable.replace_extension(runtimePath.extension());
  std::filesystem::copy(runtimePath, gameExecutable);

  // Create launch file
  YAML::Node node;
  node["name"] = gameName.string();
  node["startingScene"] = project.startingScene;

  std::ofstream stream(destination / "launch.yml", std::ios::out);
  stream << node;
  stream.close();

  Engine::getLogger().info() << "Game exported to " << destination;
}

} // namespace quoll::editor

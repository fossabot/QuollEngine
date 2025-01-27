#include "quoll/core/Base.h"
#include "quoll/yaml/Yaml.h"
#include "quoll/platform/tools/FileDialog.h"

#include "ProjectManager.h"

namespace quoll::editor {

static const String GitIgnoreContents = R"""(cache/
settings/
)""";

bool ProjectManager::createProjectInPath() {
  auto projectPath = platform::FileDialog::getFilePathFromCreateDialog(
      {{"Quoll project", {"quoll"}}});

  if (projectPath.empty()) {
    return false;
  }

  mProject.name = projectPath.stem().filename().string();
  mProject.version = "0.0.1";
  mProject.assetsPath = projectPath / "assets";
  mProject.assetsCachePath = projectPath / "cache";
  mProject.settingsPath = projectPath / "settings";

  std::filesystem::create_directory(projectPath);
  std::filesystem::create_directory(mProject.assetsPath);
  std::filesystem::create_directory(mProject.assetsCachePath);
  std::filesystem::create_directory(mProject.settingsPath);

  std::filesystem::create_directory(mProject.assetsPath / "scenes");
  std::filesystem::create_directory(mProject.assetsPath / "prefabs");
  std::filesystem::create_directory(mProject.assetsPath / "textures");
  std::filesystem::create_directory(mProject.assetsPath / "fonts");
  std::filesystem::create_directory(mProject.assetsPath / "audio");
  std::filesystem::create_directory(mProject.assetsPath / "scripts");
  std::filesystem::create_directory(mProject.assetsPath / "animators");

  {
    YAML::Node sceneObj;
    sceneObj["name"] = "MainScene";
    sceneObj["version"] = "0.1";
    sceneObj["type"] = "scene";

    YAML::Node mainZone;
    mainZone["name"] = "MainZone";
    sceneObj["zones"][0] = mainZone;
    sceneObj["entities"] = YAML::Node(YAML::NodeType::Sequence);

    std::ofstream stream(mProject.assetsPath / "scenes" / "main.scene");
    stream << sceneObj;
    stream.close();
  }

  {
    YAML::Node projectObj;
    projectObj["name"] = mProject.name;
    projectObj["version"] = mProject.version;
    projectObj["paths"]["assets"] =
        std::filesystem::relative(mProject.assetsPath, projectPath).string();
    projectObj["paths"]["assetsCache"] =
        std::filesystem::relative(mProject.assetsCachePath, projectPath)
            .string();
    projectObj["paths"]["settings"] =
        std::filesystem::relative(mProject.settingsPath, projectPath).string();

    auto projectFile = projectPath / (mProject.name + ".quoll");

    std::ofstream stream(projectFile, std::ios::out);
    stream << projectObj;
    stream.close();
  }

  {
    std::ofstream stream(projectPath / ".gitignore", std::ios::out);
    stream << GitIgnoreContents;
    stream.close();
  }

  return true;
}

bool ProjectManager::openProjectInPath() {
  auto projectFilePath = platform::FileDialog::getFilePathFromDialog(
      {{"Quoll project", {"quoll"}}});
  if (projectFilePath.empty()) {
    return false;
  }

  auto directory = std::filesystem::path(projectFilePath).parent_path();

  YAML::Node projectObj;

  std::ifstream in(projectFilePath, std::ios::in);

  try {
    projectObj = YAML::Load(in);
    in.close();
  } catch (std::exception &) {
    in.close();
    return false;
  }

  mProject.name = projectObj["name"].as<String>();
  mProject.version = projectObj["version"].as<String>();
  mProject.assetsPath =
      directory / String(projectObj["paths"]["assets"].as<String>());
  mProject.assetsCachePath =
      directory / String(projectObj["paths"]["assetsCache"].as<String>());
  mProject.settingsPath =
      directory / String(projectObj["paths"]["settings"].as<String>());

  return true;
}

} // namespace quoll::editor

#include "core/EngineGlobals.h"
#include "VulkanContext.h"
#include "VulkanWindow.h"
#include "VulkanError.h"

namespace liquid {

VulkanContext::VulkanContext(GLFWWindow *window_, bool enableValidations_)
    : window(window_), enableValidations(enableValidations_) {
  createInstance("Engine", "Engine");
  surface = createSurfaceFromWindow(instance, window);
  pickPhysicalDevice();
  createLogicalDevice();
}

VulkanContext::~VulkanContext() {
  if (device) {
    vkDestroyDevice(device, nullptr);
    device = nullptr;
    LOG_DEBUG("[Vulkan] Device destroyed");
  }

  if (surface) {
    vkDestroySurfaceKHR(instance, surface, nullptr);
    surface = VK_NULL_HANDLE;
    LOG_DEBUG("[Vulkan] Surface destroyed");
  }

  validator.detachFromInstance(instance);

  if (instance) {
    vkDestroyInstance(instance, nullptr);
    instance = nullptr;
    LOG_DEBUG("[Vulkan] Vulkan instance destroyed");
  }
}

void VulkanContext::createInstance(const String &engineName,
                                   const String &applicationName) {
  std::vector<const char *> extensions{
      "VK_EXT_metal_surface", "VK_MVK_macos_surface", "VK_KHR_surface"};

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pNext = nullptr;
  appInfo.pApplicationName = applicationName.c_str();
  appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 11, 0);
  appInfo.pEngineName = engineName.c_str();
  appInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 11, 0);
  appInfo.apiVersion = VK_API_VERSION_1_2;

  VkInstanceCreateInfo createInstanceInfo{};
  createInstanceInfo.flags = 0;
  createInstanceInfo.pNext = nullptr;
  createInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInstanceInfo.pApplicationInfo = &appInfo;
  createInstanceInfo.enabledLayerCount = 0;
  createInstanceInfo.ppEnabledLayerNames = nullptr;

  if (enableValidations) {
    LOG_DEBUG("[Vulkan] Validations layers enabled");
    validator.attachToInstanceCreateConfig(createInstanceInfo);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  createInstanceInfo.enabledExtensionCount =
      static_cast<uint32_t>(extensions.size());
  createInstanceInfo.ppEnabledExtensionNames = extensions.data();

  checkForVulkanError(vkCreateInstance(&createInstanceInfo, nullptr, &instance),
                      "Failed to create instance");

  if (enableValidations) {
    validator.attachToInstance(instance);
  }

  LOG_DEBUG("[Vulkan] Vulkan instance created");
}

void VulkanContext::pickPhysicalDevice() {
  auto &&devices = VulkanPhysicalDevice::getPhysicalDevices(instance, surface);
  auto it =
      std::find_if(devices.begin(), devices.end(), [](const auto &device) {
        return device.getQueueFamilyIndices().isComplete() &&
               device.supportsSwapchain() &&
               !device.getSurfaceFormats().empty() &&
               !device.getPresentModes().empty();
      });

  if (it == devices.end()) {
    throw VulkanError("No suitable physical device found", VK_SUCCESS);
  }

  physicalDevice = *it;
  LOG_DEBUG("[Vulkan] Physical device selected: " << physicalDevice.getName());
}

void VulkanContext::createLogicalDevice() {
  float queuePriority = 1.0f;

  VkDeviceQueueCreateInfo createGraphicsQueueInfo{};
  createGraphicsQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  createGraphicsQueueInfo.flags = 0;
  createGraphicsQueueInfo.pNext = nullptr;
  createGraphicsQueueInfo.queueFamilyIndex =
      physicalDevice.getQueueFamilyIndices().graphicsFamily.value();
  createGraphicsQueueInfo.queueCount = 1;
  createGraphicsQueueInfo.pQueuePriorities = &queuePriority;

  VkDeviceQueueCreateInfo createPresentQueueInfo{};
  createPresentQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  createPresentQueueInfo.flags = 0;
  createPresentQueueInfo.pNext = nullptr;
  createPresentQueueInfo.queueFamilyIndex =
      physicalDevice.getQueueFamilyIndices().presentFamily.value();
  createPresentQueueInfo.queueCount = 1;
  createPresentQueueInfo.pQueuePriorities = &queuePriority;

  std::vector<VkDeviceQueueCreateInfo> queueInfos;
  queueInfos.push_back(createGraphicsQueueInfo);
  if (physicalDevice.getQueueFamilyIndices().graphicsFamily.value() !=
      physicalDevice.getQueueFamilyIndices().presentFamily.value()) {
  }
  std::vector<const char *> extensions;
  extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  extensions.push_back("VK_KHR_portability_subset");
  extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
  extensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);

  VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures{};
  descriptorIndexingFeatures.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
  descriptorIndexingFeatures.pNext = nullptr;
  descriptorIndexingFeatures.descriptorBindingPartiallyBound = true;

  auto &features = physicalDevice.getFeatures();

  VkDeviceCreateInfo createDeviceInfo{};
  createDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createDeviceInfo.pNext = &descriptorIndexingFeatures;
  createDeviceInfo.flags = 0;

  createDeviceInfo.pQueueCreateInfos = queueInfos.data();
  createDeviceInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueInfos.size());
  createDeviceInfo.pEnabledFeatures = &features;
  createDeviceInfo.enabledLayerCount = 0;
  createDeviceInfo.ppEnabledLayerNames = nullptr;
  createDeviceInfo.enabledExtensionCount =
      static_cast<uint32_t>(extensions.size());
  createDeviceInfo.ppEnabledExtensionNames = extensions.data();

  checkForVulkanError(vkCreateDevice(physicalDevice.getVulkanDevice(),
                                     &createDeviceInfo, nullptr, &device),
                      "Failed to create device");

  LOG_DEBUG("[Vulkan] Vulkan Device created");

  vkGetDeviceQueue(device,
                   physicalDevice.getQueueFamilyIndices().presentFamily.value(),
                   0, &presentQueue);
  vkGetDeviceQueue(
      device, physicalDevice.getQueueFamilyIndices().graphicsFamily.value(), 0,
      &graphicsQueue);
}

} // namespace liquid

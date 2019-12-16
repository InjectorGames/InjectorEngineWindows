
// Copyright 2019 Nikita Fediuchin

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//    http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include "Engine/EngineInfo.hpp"
#include "Engine/Vulkan/VulkanDebug.hpp"
#include "Engine/Vulkan/WindowDeviceInfo.hpp"

#include <map>

// Returns Vulkan required extensions array
static const vector<const char*> GetVulkanRequiredExtensions(const vector<const char*>& additionalExtensions)
{
	uint32_t glfwExtensionCount;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	for (const char* name : additionalExtensions)
		extensions.push_back(name);

	return extensions;
}

// Creates a new vulkan instance (with debug if enabled)
static VkInstance CreateVulkanInstance(string appName, uint32_t appVersion, const vector<const char*>& vulkanExtensions, const vector<const char*>& validationLayers, VulkanDebug& vulkanDebug)
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = appName.c_str();
	appInfo.applicationVersion = appVersion;
	appInfo.pEngineName = EngineName.c_str();
	appInfo.engineVersion = EngineVersion;
	appInfo.apiVersion = VulkanVersion;

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	auto extensions = GetVulkanRequiredExtensions(vulkanExtensions);
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};

	if (validationLayers.size() > 0)
		EnableVulkanDebug(instanceCreateInfo, debugCreateInfo, validationLayers);

	VkInstance instance;
	auto result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

	if (result != VK_SUCCESS)
		throw VulkanException("Failed to create Vulkan instance. Result: " + to_string(result));

	if(validationLayers.size() > 0)
		vulkanDebug = CreateVulkanDebugInstance(instance, debugCreateInfo);

	return instance;
}

// Creates a new Vulkan window surface instance
static VkSurfaceKHR CreateWindowSurfaceInstance(VkInstance vkInstance, GlfwWindow glfwWindow)
{
	VkSurfaceKHR surafce;
	auto result = glfwCreateWindowSurface(vkInstance, glfwWindow, nullptr, &surafce);

	if (result != VK_SUCCESS)
		throw VulkanException("Failed to create Vulkan window surface. Result: " + to_string(result));

	return surafce;
}

// Returns most suitable physical device
static VkPhysicalDevice FindMostSuitablePhysicalDevice(VkInstance vkInstance, DeviceInfo* deviceInfo)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

	if (deviceCount == 0)
		throw VulkanException("Failed to find GPUs with Vulkan support");

	vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

	multimap<int, VkPhysicalDevice> candidates;

	for (const auto& device : devices)
	{
		deviceInfo->UpdateValues(device);
		auto score = deviceInfo->GetPhysicalDeviceScore(device);
		candidates.insert(make_pair(score, device));
	}

	if (candidates.rbegin()->first == 0)
		throw VulkanException("Failed to find a Vulkan suitable GPU");

	return candidates.rbegin()->second;
}

// Creates a new Vulkan logical device instance
static VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice, const vector<VkDeviceQueueCreateInfo>& queueCreateInfos, const vector<const char*>& validationLayers, const vector<const char*>& deviceExtensions)
{
	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	if (validationLayers.size() == 0)
	{
		createInfo.enabledLayerCount = 0;
	}
	else
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	VkDevice device;

	auto result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
	if (result != VK_SUCCESS)
		throw VulkanException("Failed to create Vulkan logical device. Result: " + to_string(result));

	return device;
}

// Creates a new Vulkan swapchain instance
static VkSwapchainKHR CreateSwapchainInstance(VkDevice device, WindowDeviceInfo deviceInfo)
{
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = deviceInfo->GetSurface();

	auto surfaceCapabilities = deviceInfo->GetSurfaceCapabilities();
	auto imageCount = surfaceCapabilities.minImageCount + 1;

	if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount)
		imageCount = surfaceCapabilities.maxImageCount;

	createInfo.minImageCount = imageCount;

	auto surfaceFormat = deviceInfo->GetSurfaceFormat();
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;

	auto surfaceExtent = deviceInfo->GetSurfaceExtent();
	createInfo.imageExtent = surfaceExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilies[] =
	{
		deviceInfo->GetGraphicsFamily(),
		deviceInfo->GetPresentFamily(),
	};

	if (queueFamilies[0] != queueFamilies[1])
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilies;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = surfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	auto presentMode = deviceInfo->GetPresentMode();
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkSwapchainKHR swapchain;
	auto result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain);

	if (result != VK_SUCCESS)
		throw VulkanException("Failed to create swap chain instance. Result: " + to_string(result));

	return swapchain;
}

// Creates a new vulkan swapchain image view array
static const vector<VkImageView> CreateSwapchainImageViews(VkDevice device, WindowDeviceInfo deviceInfo, const vector<VkImage>& swapchainImages)
{
	auto imageCount = swapchainImages.size();
	auto surfaceFormat = deviceInfo->GetSurfaceFormat();
	vector<VkImageView> swapchainImageViews(imageCount);

	for (size_t i = 0; i < imageCount; i++)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = surfaceFormat.format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		auto result = vkCreateImageView(device, &createInfo, nullptr, &swapchainImageViews[i]);
		if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to create image view instance. Result: " + to_string(result));
	}

	return swapchainImageViews;
}
// Destroys vulkan swapchain image view array
static void DestroySwapchainImageViews(VkDevice device, const vector<VkImageView>& swapchainImageViews)
{
	for (auto imageView : swapchainImageViews)
		vkDestroyImageView(device, imageView, nullptr);
}

class VulkanWindow_T
{
protected:
	// Vulkan instance
	VkInstance instance;
	// Vulkan debug instance
	VulkanDebug vulkanDebug;
	// Vulkan surface instance
	VkSurfaceKHR surface;

	// Vulkan window device information
	WindowDeviceInfo deviceInfo;
	// Vulkan physical device
	VkPhysicalDevice physicalDevice;
	// Vulkan logical device
	VkDevice logicalDevice;

	// Vulkan graphics queue
	VkQueue graphicsQueue;
	// Vulkan present queue (image to surface)
	VkQueue presentQueue;

	// Vulkan swapchain instance
	VkSwapchainKHR swapchain;
	// Vulkan swapchain image array
	vector<VkImage> swapchainImages;
	// Vulkan swapchain image view array
	vector<VkImageView> swapchainImageViews;

public:
	// Creates a new vulkan window class instance
	VulkanWindow_T(GlfwWindow glfwWindow, VkExtent2D windowSize, string appName, uint32_t appVersion, const vector<const char*>& vulkanExtensions, const vector<const char*>& validationLayers, const vector<const char*>& deviceExtensions)
	{
		instance = CreateVulkanInstance(appName, appVersion, vulkanExtensions, validationLayers, vulkanDebug);
		surface = CreateWindowSurfaceInstance(instance, glfwWindow);
		deviceInfo = CreateWindowDeviceInfoInstance(surface, windowSize, deviceExtensions);
		physicalDevice = FindMostSuitablePhysicalDevice(instance, deviceInfo);

		auto queueCreateInfos = deviceInfo->GetQueueCreateInfos();
		logicalDevice = CreateLogicalDevice(physicalDevice, queueCreateInfos, validationLayers, deviceExtensions);
		
		vkGetDeviceQueue(logicalDevice, deviceInfo->GetGraphicsFamily(), 0, &graphicsQueue);
		vkGetDeviceQueue(logicalDevice, deviceInfo->GetPresentFamily(), 0, &presentQueue);

		swapchain = CreateSwapchainInstance(logicalDevice, deviceInfo);

		uint32_t imageCount;
		vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);

		swapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, swapchainImages.data());

		swapchainImageViews.resize(imageCount);
		swapchainImageViews = CreateSwapchainImageViews(logicalDevice, deviceInfo, swapchainImages);
	}
	// Destroys vulkan window class instance
	~VulkanWindow_T()
	{
		DestroySwapchainImageViews(logicalDevice, swapchainImageViews);
		vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);
		vkDestroyDevice(logicalDevice, nullptr);
		DestroyWindowDeviceInfoInstance(deviceInfo);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		DestroyVulkanDebugInstance(vulkanDebug);
		vkDestroyInstance(instance, nullptr);
	}
};

typedef VulkanWindow_T* VulkanWindow;

// Creates a new vulkan window instance
static VulkanWindow CreateVulkanWindowInstance(GlfwWindow glfwWindow, VkExtent2D windowSize, string appName, uint32_t appVersion, const vector<const char*>& validationLayers, const vector<const char*>& vulkanExtensions, const vector<const char*>& deviceExtensions)
{
	return new VulkanWindow_T(glfwWindow, windowSize, appName, appVersion, validationLayers, vulkanExtensions, deviceExtensions);
}
// Destroys vulkan window instance
static void DestroyVulkanWindowInstance(VulkanWindow instance)
{
	delete instance;
}

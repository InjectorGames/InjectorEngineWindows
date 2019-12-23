
// Copyright 2019 Nikita Fediuchin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include "DeviceInfo.hpp"

#include <optional>

namespace Vulkan
{
	// Returns best available vulkan surface format
	static VkSurfaceFormatKHR GetBestSurfaceFormat(const vector<VkSurfaceFormatKHR>& formats)
	{
		for (const auto& availableFormat : formats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		}

		return formats[0];
	}

	// Returns best available vulkan present mode
	static VkPresentModeKHR GetBestPresentMode(const vector<VkPresentModeKHR>& presentModes)
	{
		for (const auto& availablePresentMode : presentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return availablePresentMode;
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	// Returns best vulkan surface extent
	static VkExtent2D GetBestExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D surfaceExtent)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			surfaceExtent.width = clamp(surfaceExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			surfaceExtent.height = clamp(surfaceExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return surfaceExtent;
		}
	}

	// Window device information container class
	struct WindowDeviceInfo_T : public DeviceInfo_T
	{
	protected:
		// Window size
		VkExtent2D windowSize;
		// Surface instance
		VkSurfaceKHR surface;
		// Physical device extension array
		vector<const char*> extensions;

		// Queue family priority instance
		float* queuePriority;
		// Graphics queue family
		optional<uint32_t> graphicsFamily;
		// Present queue family
		optional<uint32_t> presentFamily;

		// Physical device surface capabilities
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		// Physical device surface format
		VkSurfaceFormatKHR surfaceFormat;
		// Physical device present mode
		VkPresentModeKHR presentMode;
		// Physical device surface extent
		VkExtent2D surfaceExtent;

		// Updates device queue families
		void UpdateQueueFamilies(VkPhysicalDevice physicalDevice)
		{
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

			vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

			for (uint32_t i = 0; i < queueFamilyCount; i++)
			{
				auto queueFamily = queueFamilies[i];

				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
					graphicsFamily = i;

				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

				if (presentSupport)
					presentFamily = i;

				if (graphicsFamily.has_value() && presentFamily.has_value())
					break;
			}
		}

		// Updates device swapchain values
		void UpdateSwapchain(VkPhysicalDevice physicalDevice)
		{
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
			
			if (formatCount == 0)
				throw VulkanException("Failed to get physical device surafce formats");

			vector<VkSurfaceFormatKHR> surfaceFormats;
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
			
			if (presentModeCount == 0)
				throw VulkanException("Failed to get physical device present modes");

			vector<VkPresentModeKHR> presentModes;
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

			surfaceFormat = GetBestSurfaceFormat(surfaceFormats);
			presentMode = GetBestPresentMode(presentModes);
			surfaceExtent = GetBestExtent(surfaceCapabilities, windowSize);
		}

	public:
		// Creates a new window device information container instance
		WindowDeviceInfo_T(VkSurfaceKHR _surface, VkExtent2D _windowSize, const vector<const char*>& _extensions)
		{
			surface = _surface;
			windowSize = _windowSize;
			extensions = _extensions;

			queuePriority = new float[1]{ 1.0f };
		}
		// Destroys window device information container instance
		~WindowDeviceInfo_T()
		{
			delete queuePriority;
		}

		// Returns vulakn window size
		VkExtent2D GetWindowSize() { return windowSize; }
		// Returns vulkan surface instance
		VkSurfaceKHR GetSurface() { return surface; }
		// Returns vulkan surface instance
		vector<const char*> GetExtensions() { return extensions; }

		// Returns graphics family value
		uint32_t GetGraphicsFamily() { return graphicsFamily.value(); }
		// Returns present family value
		uint32_t GetPresentFamily() { return presentFamily.value(); }

		// Returns physical device surface capabilities
		VkSurfaceCapabilitiesKHR GetSurfaceCapabilities() { return surfaceCapabilities; }
		// Returns physical device surface format
		VkSurfaceFormatKHR GetSurfaceFormat() { return surfaceFormat; }
		// Returns physical device present mode
		VkPresentModeKHR GetPresentMode() { return presentMode; }
		// Returns physical device surface extent
		VkExtent2D GetSurfaceExtent() { return surfaceExtent; }

		// Updates device information container values
		void UpdateValues(VkPhysicalDevice physicalDevice)
		{
			UpdateQueueFamilies(physicalDevice);
			UpdateSwapchain(physicalDevice);
		}

		// Returns true if device information is valid
		bool IsValid(VkPhysicalDevice physicalDevice)
		{
			try
			{
				CheckDeviceExtensionsSupport(physicalDevice, extensions);
				return graphicsFamily.has_value() && presentFamily.has_value() && !surfaceFormats.empty() && !presentModes.empty();
			}
			catch (VulkanException&)
			{
				return false;
			}
		}

		// Returns physical device score
		int GetPhysicalDeviceScore(VkPhysicalDevice physicalDevice)
		{
			if (!IsValid(physicalDevice))
				return 0;

			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

			//VkPhysicalDeviceFeatures deviceFeatures;
			//vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				return 2;
			else
				return 1;
		}

		// Returns device queue create information array
		vector<VkDeviceQueueCreateInfo> GetQueueCreateInfos()
		{
			vector<VkDeviceQueueCreateInfo> queueCreateInfos = {};

			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = queuePriority;

			if (graphicsFamily == presentFamily)
			{
				queueCreateInfo.queueFamilyIndex = graphicsFamily.value();
				queueCreateInfos.push_back(queueCreateInfo);
			}
			else
			{
				queueCreateInfo.queueFamilyIndex = graphicsFamily.value();
				queueCreateInfos.push_back(queueCreateInfo);

				queueCreateInfo.queueFamilyIndex = presentFamily.value();
				queueCreateInfos.push_back(queueCreateInfo);
			}

			return queueCreateInfos;
		}
	};

	typedef WindowDeviceInfo_T* WindowDeviceInfo;

	// Creates a new vulkan window device information container instance
	static WindowDeviceInfo CreateWindowDeviceInfoInstance(VkSurfaceKHR surface, VkExtent2D windowSize, const vector<const char*>& extensions)
	{
		return new WindowDeviceInfo_T(surface, windowSize, extensions);
	}
	// Destroys vulkan window device information container instance
	static void DestroyWindowDeviceInfoInstance(WindowDeviceInfo instance)
	{
		delete instance;
	}
}

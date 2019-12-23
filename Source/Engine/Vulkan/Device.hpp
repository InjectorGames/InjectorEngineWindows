
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
#include "Vulkan.hpp"
#include "DeviceInfo.hpp"

#include <map>

using namespace std;

namespace Vulkan
{
	// Returns most suitable vulkan physical device
	static VkPhysicalDevice FindMostSuitablePhysicalDevice(VkInstance instance, DeviceInfo deviceInfo)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0)
			throw VulkanException("Failed to find GPUs with Vulkan support");

		vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

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

	// Creates a new vulkan logical device instance
	static VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice, const vector<VkDeviceQueueCreateInfo>& queueCreateInfos, const vector<const char*>& validationLayers, const vector<const char*>& extensions)
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

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDevice device;

		auto result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
		if (result != VK_SUCCESS)
			throw VulkanException("Failed to create Vulkan logical device. Result: " + to_string(result));

		return device;
	}

	// Vulkan device containder class
	class Device_T
	{
	protected:
		// Vulkan device information
		DeviceInfo deviceInfo;
		// Vulkan physical device instance
		VkPhysicalDevice physicalDevice;

		// Vulkan logical device instance
		VkDevice instance;

	public:
		// Creates a new vulkan device class instance
		Device_T(DeviceInfo _deviceInfo, VkInstance vkInstance, VkSurfaceKHR surface, const vector<const char*>& validationLayers, const vector<const char*>& extensions)
		{
			deviceInfo = _deviceInfo;
			physicalDevice = FindMostSuitablePhysicalDevice(vkInstance, _deviceInfo);

			auto queueCreateInfos = deviceInfo->GetQueueCreateInfos();
			instance = CreateLogicalDevice(physicalDevice, queueCreateInfos, validationLayers, extensions);
		}
		// Destroys vulkan device class instance
		~Device_T()
		{
			vkDestroyDevice(instance, nullptr);
		}

		// Returns vulkan device information
		DeviceInfo GetDeviceInfo() { return deviceInfo; }
		// Returns vulkan physical device instance
		VkPhysicalDevice GetPhysicalDevice() { return physicalDevice; }

		// Returns vulkan logical device instance
		VkDevice GetInstance() { return instance; }
	};

	typedef Device_T* Device;

	// Creates a new vulkan device class instance
	static Device CreateDeviceInstance(DeviceInfo deviceInfo, VkInstance instance, VkSurfaceKHR surface, const vector<const char*>& validationLayers, const vector<const char*>& extensions)
	{
		return new Device_T(deviceInfo, instance, surface, validationLayers, extensions);
	}
	// Destroys vulkan device class instance
	static void DestroyDeviceInstance(Device instance)
	{
		delete instance;
	}
}

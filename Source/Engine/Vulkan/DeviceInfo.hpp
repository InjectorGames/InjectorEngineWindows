
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
#include "Exceptions.hpp"

#include <vector>
#include <algorithm>

using namespace std;

namespace Vulkan
{
	// Checks if all vulkan device extensions is supported
	static void CheckDeviceExtensionsSupport(VkPhysicalDevice physicalDevice, const vector<const char*>& extensions)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

		vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

		for (const char* name : extensions)
		{
			auto found = false;

			for (const auto& properties : availableExtensions)
			{
				if (strcmp(name, properties.extensionName) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
				throw VulkanException("Some of physical device extensions is not supported");
		}
	}

	// Vulkan device information container base class
	struct DeviceInfo_T
	{
	public:
		// Updates device information container values
		virtual void UpdateValues(VkPhysicalDevice physicalDevice) {}
		// Returns true device information container is valid
		virtual bool IsValid(VkPhysicalDevice physicalDevice) { return false; }
		// Returns physical device score
		virtual int GetPhysicalDeviceScore(VkPhysicalDevice physicalDevice) { return 0; }
		// Returns device queue create information array
		virtual vector<VkDeviceQueueCreateInfo> GetQueueCreateInfos() { return vector<VkDeviceQueueCreateInfo>(); }
	};

	// Vulkan device information container instance
	typedef DeviceInfo_T* DeviceInfo;

	// Creates a new vulkan device information container instance
	static DeviceInfo CreateDeviceInfoInstance()
	{
		return new DeviceInfo_T();
	}
	// Destroys vulkan device information container instance
	static void DestroyDeviceInfoInstance(DeviceInfo instance)
	{
		delete instance;
	}
}

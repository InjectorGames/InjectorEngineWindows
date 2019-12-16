
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
#include "Engine/Vulkan/Vulkan.hpp"
#include "Engine/Vulkan/VulkanExceptions.hpp"

#include <vector>
#include <algorithm>

using namespace std;

// Checks if all device array extensions is supported
static void CheckDeviceExtensionsSupport(VkPhysicalDevice device, const vector<const char*>& extensions)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

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

// Device information container base class
struct DeviceInfo
{
public:
	// Updates device information container values
	virtual void UpdateValues(VkPhysicalDevice physicalDevice) {}
	// Returns true device information container is valid
	virtual bool IsValid(VkPhysicalDevice physicalDevice) { return false; }
	// Returns physical device score
	virtual int GetPhysicalDeviceScore(VkPhysicalDevice physicalDevice) { return 0; }
};
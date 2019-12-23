
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
#include <iostream>

namespace Vulkan
{
	// Creates a new vulkan debug utilities messenger extension instance
	static VkResult VkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		return func != nullptr ? func(instance, pCreateInfo, pAllocator, pDebugMessenger) : VK_ERROR_EXTENSION_NOT_PRESENT;
	}
	// Destroys vulkan debug utilities messenger extension instance
	static void VkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) func(instance, debugMessenger, pAllocator);
	}

	// Vulcan debug callback method
	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		cerr << "Validation layer: " << pCallbackData->pMessage << "\n";
		return VK_FALSE;
	}

	// Checks if all array vulkan validation layers is supported
	static void CheckValidationLayersSupport(const vector<const char*>& layers)
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* name : layers)
		{
			auto found = false;

			for (const auto& properties : availableLayers)
			{
				if (strcmp(name, properties.layerName) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
				throw VulkanException("Some of validation layers is not supported");
		}
	}

	// Adds validation layer support to the vulkan instance
	static void EnableDebug(VkInstanceCreateInfo& createInfo, VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo, const vector<const char*>& validationLayers)
	{
		CheckValidationLayersSupport(validationLayers);

		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = VulkanDebugCallback;
		debugCreateInfo.pUserData = nullptr;

		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}

	// Vulkan debug class
	class Debug_T
	{
	protected:
		// Vulkan instance
		VkInstance instance;
		// Vulkan debug messenger instance
		VkDebugUtilsMessengerEXT messenger;

	public:
		// Creates a new vulkan debug class instance
		Debug_T(VkInstance _instance, const VkDebugUtilsMessengerCreateInfoEXT& createInfo)
		{
			instance = _instance;

			auto result = VkCreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &messenger);
			if (result != VK_SUCCESS)
				throw VulkanException("Failed to create Vulkan debug messenger instance. Result: " + to_string(result));
		}
		// Destroys vulkan debug class instance
		~Debug_T()
		{
			VkDestroyDebugUtilsMessengerEXT(instance, messenger, nullptr);
		}

		// Returns vulkan debug utilities messenger instance
		VkDebugUtilsMessengerEXT GetMessenger() { return messenger; }
	};

	// Vulkan debug class instance
	typedef Debug_T* Debug;

	// Creates a new vulkan debug class instance
	static Debug CreateDebugInstance(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		return new Debug_T(instance, createInfo);
	}
	// Destroys vulkan debug class instance
	static void DestroyDebugInstance(Debug instance)
	{
		delete instance;
	}
}

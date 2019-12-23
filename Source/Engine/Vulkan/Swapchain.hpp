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
#include "WindowDeviceInfo.hpp"

#include <vector>

using namespace std;

namespace Vulkan
{
	// Creates a new vulkan swapchain instance
	static VkSwapchainKHR CreateVulkanSwapchainInstance(VkDevice device, WindowDeviceInfo deviceInfo)
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
	static const vector<VkImageView> CreateImageViews(VkDevice device, WindowDeviceInfo deviceInfo, const vector<VkImage>& images)
	{
		auto imageCount = images.size();
		auto surfaceFormat = deviceInfo->GetSurfaceFormat();
		vector<VkImageView> swapchainImageViews(imageCount);

		for (size_t i = 0; i < imageCount; i++)
		{
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = images[i];
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
	static void DestroyImageViews(VkDevice device, const vector<VkImageView>& imageViews)
	{
		for (auto imageView : imageViews)
			vkDestroyImageView(device, imageView, nullptr);
	}

	// Vulkan window swapchain class
	class WindowSwapchain_T
	{
	protected:
		// Vulkan device instance
		VkDevice device;

		// Vulkan swapchain instance
		VkSwapchainKHR instance;
		// Vulkan image array
		vector<VkImage> images;
		// Vulkan image view array
		vector<VkImageView> imageViews;

	public:
		// Creates a new vulkan window swapchain class instance
		WindowSwapchain_T(VkDevice _device, WindowDeviceInfo deviceInfo)
		{
			device = _device;
			instance = CreateVulkanSwapchainInstance(_device, deviceInfo);

			uint32_t imageCount;
			vkGetSwapchainImagesKHR(_device, instance, &imageCount, nullptr);

			images.resize(imageCount);
			vkGetSwapchainImagesKHR(_device, instance, &imageCount, images.data());

			imageViews.resize(imageCount);
			imageViews = CreateImageViews(_device, deviceInfo, images);
		}
		// Destroys vulkan window swapchain class instance
		~WindowSwapchain_T()
		{
			DestroyImageViews(device, imageViews);
			vkDestroySwapchainKHR(device, instance, nullptr);
		}

		// Returns vulkan device instance
		VkDevice GetDevice() { return device; }

		// Returns vulkan swapchain instance
		VkSwapchainKHR GetInstance() { return instance; }
		// Returns vulkan image array
		vector<VkImage> GetImages() { return images; }
		// Returns vulkan image view array
		vector<VkImageView> GetImageViews() { return imageViews; }
	};

	// Vulkan window swapchain class instance
	typedef WindowSwapchain_T* Swapchain;

	// Creates a new vulkan window swapchain class instance
	static Swapchain CreateSwapchainInstance(VkDevice device, WindowDeviceInfo deviceInfo)
	{
		return new WindowSwapchain_T(device, deviceInfo);
	}
	// Destroys vulkan window swapchain class instance
	static void DestroySwapchainInstance(Swapchain instance)
	{
		delete instance;
	}
}
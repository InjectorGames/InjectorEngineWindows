
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
#include "Debug.hpp"
#include "Device.hpp"
#include "Pipeline.hpp"
#include "Swapchain.hpp"
#include "CommandPool.hpp"
#include "Engine/EngineInfo.hpp"

namespace Vulkan
{
	// Returns vulkan required extension array
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
	static VkInstance CreateVulkanInstance(string appName, uint32_t appVersion, const vector<const char*>& additionalExtensions, const vector<const char*>& validationLayers, Debug& debug)
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

		auto extensions = GetVulkanRequiredExtensions(additionalExtensions);
		instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};

		if (validationLayers.size() > 0)
			EnableDebug(instanceCreateInfo, debugCreateInfo, validationLayers);

		VkInstance instance;
		auto result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

		if (result != VK_SUCCESS)
			throw VulkanException("Failed to create Vulkan instance. Result: " + to_string(result));

		if (validationLayers.size() > 0)
			debug = CreateDebugInstance(instance, debugCreateInfo);

		return instance;
	}

	// Creates a new vulkan window surface instance
	static VkSurfaceKHR CreateWindowSurfaceInstance(VkInstance instance, GlfwWindow window)
	{
		VkSurfaceKHR surafce;
		auto result = glfwCreateWindowSurface(instance, window, nullptr, &surafce);

		if (result != VK_SUCCESS)
			throw VulkanException("Failed to create Vulkan window surface. Result: " + to_string(result));

		return surafce;
	}

	// Vulkan window class
	class Window_T
	{
	protected:
		// Vulkan instance
		VkInstance instance;
		// Vulkan surface instance
		VkSurfaceKHR surface;

		// Vulkan graphics queue
		VkQueue graphicsQueue;
		// Vulkan present queue (image to surface)
		VkQueue presentQueue;

		// Vulkan debug instance
		Debug debug;
		// Vulkan device instance
		Device device;
		// Vulkan swapchain instance
		Swapchain swapchain;
		// Vulkan graphics pipeline instance
		Pipeline graphicsPipeline;
		// VUlkan command pool instance
		CommandPool commandPool;

		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;

	public:
		// Creates a new vulkan window class instance
		Window_T(GlfwWindow glfwWindow, VkExtent2D windowSize, string appName, uint32_t appVersion, const vector<const char*>& vulkanExtensions, const vector<const char*>& validationLayers, const vector<const char*>& deviceExtensions)
		{
			instance = CreateVulkanInstance(appName, appVersion, vulkanExtensions, validationLayers, debug);
			surface = CreateWindowSurfaceInstance(instance, glfwWindow);

			auto deviceInfo = CreateWindowDeviceInfoInstance(surface, windowSize, deviceExtensions);
			device = CreateDeviceInstance(deviceInfo, instance, surface, validationLayers, deviceExtensions);

			auto logicalDevice = device->GetInstance();
			vkGetDeviceQueue(logicalDevice, deviceInfo->GetGraphicsFamily(), 0, &graphicsQueue);
			vkGetDeviceQueue(logicalDevice, deviceInfo->GetPresentFamily(), 0, &presentQueue);

			swapchain = CreateSwapchainInstance(logicalDevice, deviceInfo);
			graphicsPipeline = CreatePipelineInstance(logicalDevice, deviceInfo, swapchain->GetImageViews());
			commandPool = CreateCommandPoolInstance(logicalDevice, deviceInfo, graphicsPipeline->renderPass, deviceInfo->GetSurfaceExtent(), graphicsPipeline->instance, graphicsPipeline->framebuffers);

			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
				vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS) {
				throw std::runtime_error("failed to create semaphores!");
			}

		}
		// Destroys vulkan window class instance
		~Window_T()
		{
			vkDestroySemaphore(device->GetInstance(), renderFinishedSemaphore, nullptr);
			vkDestroySemaphore(device->GetInstance(), imageAvailableSemaphore, nullptr);

			DestroyCommandPoolInstance(commandPool);
			DestroyPipelineInstance(graphicsPipeline);
			DestroySwapchainInstance(swapchain);
			DestroyDeviceInstance(device);
			vkDestroySurfaceKHR(instance, surface, nullptr);
			DestroyDebugInstance(debug);
			vkDestroyInstance(instance, nullptr);
		}

		void DrawFrame()
		{
			uint32_t imageIndex;
			vkAcquireNextImageKHR(device->GetInstance(), swapchain->GetInstance(), UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandPool->commandBuffers[imageIndex];

			VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
				throw std::runtime_error("failed to submit draw command buffer!");
			}

			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			VkSwapchainKHR swapChains[] = { swapchain->GetInstance() };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;
			presentInfo.pImageIndices = &imageIndex;
			presentInfo.pResults = nullptr; // Optional

			vkQueuePresentKHR(presentQueue, &presentInfo);
		}
	};

	// Vulkan window class instance
	typedef Window_T* Window;

	// Creates a new vulkan window class instance
	static Window CreateWindowInstance(GlfwWindow glfwWindow, VkExtent2D windowSize, string appName, uint32_t appVersion, const vector<const char*>& validationLayers, const vector<const char*>& vulkanExtensions, const vector<const char*>& deviceExtensions)
	{
		return new Window_T(glfwWindow, windowSize, appName, appVersion, validationLayers, vulkanExtensions, deviceExtensions);
	}
	// Destroys vulkan window class instance
	static void DestroyWindowInstance(Window instance)
	{
		delete instance;
	}
}

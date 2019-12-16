
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
#include "Engine/Vulkan/VulkanWindow.hpp"

class Graphics
{
protected:
	// GLFW window instance
	GlfwWindow glfwWindow;
	// Vulkan instance
	VulkanWindow vulkanWindow;
	
public:
	// Creates a new graphics class instance
	Graphics(VkExtent2D windowSize, string appName, uint32_t appVersion, const vector<const char*>& vulkanExtensions, const vector<const char*>& validationLayers, const vector<const char*>& deviceExtensions)
	{
		if (glfwInit() == GLFW_FALSE)
			throw GraphicsException("Failed to initialize GLFW");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		glfwWindow = glfwCreateWindow(windowSize.width, windowSize.height, appName.c_str(), nullptr, nullptr);

		if (glfwVulkanSupported() == GLFW_FALSE)
			throw VulkanException("Vulkan is not supported on this machine");

		vulkanWindow = CreateVulkanWindowInstance(glfwWindow, windowSize, appName, appVersion, vulkanExtensions, validationLayers, deviceExtensions);
	}
	// Disposes graphic class instance
	~Graphics()
	{
		DestroyVulkanWindowInstance(vulkanWindow);
		glfwDestroyWindow(glfwWindow);
		glfwTerminate();
	}

	// Enters program graphics loop
	void EnterLoop()
	{
		while (!glfwWindowShouldClose(glfwWindow))
		{
			glfwPollEvents();
		}
	}
};
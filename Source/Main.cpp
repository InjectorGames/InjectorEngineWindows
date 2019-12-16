
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

#include "Engine/Graphics.hpp"

const int windowWidth = 800;
const int windowHeight = 600;

const VkExtent2D windowSize = { 800, 600 };
const string appName = "Engine Dev";
const uint32_t appVersion = VK_MAKE_VERSION(0, 1, 0);

vector<const char*> vulkanExtensions = {};
vector<const char*> validationLayers = {};
vector<const char*> deviceExtensions = {};

int main()
{
	try
	{
#if !defined(NDEBUG)
		vulkanExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		validationLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		auto graphics = Graphics(windowSize, appName, appVersion, vulkanExtensions, validationLayers, deviceExtensions);
		graphics.EnterLoop();
	}
	catch (const std::exception & e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

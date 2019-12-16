
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
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

using namespace std;

typedef GLFWwindow* GlfwWindow;

// Injector engine name
const string EngineName = "Injector Engine";
// Injector engine version
const uint32_t EngineVersion = VK_MAKE_VERSION(0, 1, 0);
// Vulkan API version
const uint32_t VulkanVersion = VK_API_VERSION_1_1;


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
#include <vector>
#include "Component.hpp"

using namespace std;

class Entity
{
protected:
	// Entity component array
	vector<Component*> components;
public:
	// Returns component from the entity component array
	void GetComponent(const Component* component)
	{
		for (size_t i = 0; i < length; i++)
		{

		}
	}
	// Adds a new component to the entity component array
	void AddComponent(Component* const component)
	{
		components.push_back(component);
	}
	// Removes component from the entity component array
	void RemoveComponent(Component* const component)
	{
		components.erase(component);
	}
};
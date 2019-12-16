
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
#include "Exception.hpp"
#include "Engine/Entity.hpp"

class System
{
protected:
	// System entity array size
	size_t size;
	// System entity array expand size
	size_t expandSize;
	// System entity array
	Entity** entities;

public:
	// Default entity array expand size
	const int DefaultExpandSize = 64;

	// Creates a new system instance
	System()
	{
		size = expandSize = DefaultExpandSize;
		entities = new Entity * [DefaultExpandSize];

		/*for (size_t i = 0; i < DefaultExpandSize; i++)
			entities[i] = nullptr;*/
	}
	// Creates a new system instance
	System(size_t _expandSize)
	{
		size = expandSize = _expandSize;
		entities = new Entity * [_expandSize];

		/*for (size_t i = 0; i < _expandSize; i++)
			entities[i] = nullptr;*/
	}

	// Adds a new entity to the system
	size_t Add(Entity* entity)
	{
		if (!entity)
			throw ArgumentNullException();

		for (size_t i = 0; i < size; i++)
		{
			if (entities[i])
				continue;

			entities[i] = entity;
			return i;
		}

		auto newEntities = new Entity * [size + expandSize];
		memcpy(newEntities, entities, size * sizeof(Entity*));

		delete[] entities;
		entities = newEntities;

		entities[size] = entity;
		size += expandSize;

		return size - expandSize;
	}

	// Removes entity from the system
	Entity* Remove(size_t index)
	{
		if (index >= size)
			throw ArgumentOutOfRangeException();

		auto entity = entities[index];
		entities[index] = nullptr;
		return entity;
	}

	// Removes entity from the system
	Entity* Get(size_t index)
	{
		if (index >= size)
			throw ArgumentOutOfRangeException();

		return entities[index];
	}

	// TODO: Add mutex thread lock on entity add/get/remove. Add mutex lock on work with specified entity.
};
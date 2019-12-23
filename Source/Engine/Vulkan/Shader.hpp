#pragma once
#include "Vulkan.hpp"
#include "Exceptions.hpp"

#include <vector>
#include <fstream>

using namespace std;
using namespace filesystem;

namespace Vulkan
{
	class Shader
	{
	public:
		// Reads compiled shader module bytecode from the disk
		static vector<char> ReadBytecode(const string& filePath)
		{
			ifstream file(filePath, ios::ate | ios::binary);

			if (!file.is_open())
				throw VulkanException("Failed to open Vulkan binary shader module file");

			size_t fileSize = (size_t)file.tellg();
			vector<char> buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);
			return buffer;
		}
	};
}
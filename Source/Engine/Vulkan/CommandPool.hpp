#pragma once
#include "Vulkan.hpp"
#include "WindowDeviceInfo.hpp"

#include <vector>

using namespace std;

namespace Vulkan
{
	// Vulkan command pool class
	class CommandPool_T // TODO: this is graphics command pool, create universal
	{
	public:
		// Vulkan logical device instance
		VkDevice device;

		// Vulkan command pool instance
		VkCommandPool instance;
		// Vulkan command buffer array
		vector<VkCommandBuffer> commandBuffers;

		// Creates a new vulkan command buffer array
		static vector<VkCommandBuffer> CreateCommandBuffers(VkCommandPool commandPool, VkDevice device, VkRenderPass renderPass, VkExtent2D extent, VkPipeline pipeline, const vector<VkFramebuffer>& framebuffers)
		{
			vector<VkCommandBuffer> commandBuffers(framebuffers.size());

			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

			auto result = vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
			if (result != VK_SUCCESS)
				throw VulkanException("Failed to allocate command buffers. Result: " + to_string(result));

			for (size_t i = 0; i < commandBuffers.size(); i++)
			{
				VkCommandBufferBeginInfo beginInfo = {};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = 0; // Optional
				beginInfo.pInheritanceInfo = nullptr; // Optional

				auto result = vkBeginCommandBuffer(commandBuffers[i], &beginInfo);
				if (result != VK_SUCCESS)
					throw VulkanException("Failed to begin recording command buffer. Result: " + to_string(result));

				VkRenderPassBeginInfo renderPassInfo = {};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = renderPass;
				renderPassInfo.framebuffer = framebuffers[i];
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = extent;

				VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
				renderPassInfo.clearValueCount = 1;
				renderPassInfo.pClearValues = &clearColor;

				vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
				vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
				vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
				vkCmdEndRenderPass(commandBuffers[i]);

				result = vkEndCommandBuffer(commandBuffers[i]);
				if (result != VK_SUCCESS)
					throw VulkanException("Failed to record command buffer. Result: " + to_string(result));
			}

			return commandBuffers;
		}

	public:
		// Creates a new vulkan command buffer class instance
		CommandPool_T(VkDevice _device, WindowDeviceInfo deviceInfo, VkRenderPass renderPass, VkExtent2D extent, VkPipeline pipeline, const vector<VkFramebuffer>& framebuffers)
		{
			device = _device;

			VkCommandPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = deviceInfo->GetGraphicsFamily();
			poolInfo.flags = 0; // Optional

			auto result = vkCreateCommandPool(_device, &poolInfo, nullptr, &instance);
			if (result != VK_SUCCESS)
				throw VulkanException("Failed to create command pool. Result: " + to_string(result));

			commandBuffers = CreateCommandBuffers(instance, _device, renderPass, extent, pipeline, framebuffers);
		}
		// Destroys vulkan command buffer class instance
		~CommandPool_T()
		{
			vkDestroyCommandPool(device, instance, nullptr);
		}
	};


	// Vulkan command pool class instance
	typedef CommandPool_T* CommandPool;

	// Creates a new vulkan command pool class instance
	static CommandPool CreateCommandPoolInstance(VkDevice device, WindowDeviceInfo deviceInfo, VkRenderPass renderPass, VkExtent2D extent, VkPipeline pipeline, const vector<VkFramebuffer>& framebuffers)
	{
		return new CommandPool_T(device, deviceInfo, renderPass, extent, pipeline, framebuffers);
	}
	// Destroys vulkan command pool class instance
	static void DestroyCommandPoolInstance(CommandPool instance)
	{
		delete instance;
	}
}

#pragma once
#include "Vulkan.hpp"
#include "Shader.hpp"
#include "Exceptions.hpp"
#include "WindowDeviceInfo.hpp"

#include <vector>

using namespace std;

namespace Vulkan
{
	// Creates a new vulkan shader module instance
	static VkShaderModule CreateShaderModuleInstance(VkDevice device, const vector<char>& bytecode)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = bytecode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());

		VkShaderModule shaderModule;
		auto result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);

		if (result != VK_SUCCESS)
			throw VulkanException("Failed to create Vulkan shader module. Result: " + to_string(result));

		return shaderModule;
	}
	// Destroys vulkan shader module instance
	static void DestroyShaderModuleInstance(VkDevice device, VkShaderModule instance)
	{
		vkDestroyShaderModule(device, instance, nullptr);
	}

	// Vulkan graphics pipeline class
	class Pipeline_T
	{
	public:
		// Vulkan logical device instance
		VkDevice device;

		// Vulkan pipeline instance
		VkPipeline instance;
		// Vulkan render pass instance
		VkRenderPass renderPass;
		// Vulkan pipeline layout instance
		VkPipelineLayout layout;
		// Vulkan pipeline framebuffer array
		vector<VkFramebuffer> framebuffers;

		// Creates a new vulkan render pass instance
		static VkRenderPass CreateRenderPassInstance(VkDevice device, WindowDeviceInfo deviceInfo)
		{
			VkAttachmentDescription colorAttachment = {};
			colorAttachment.format = deviceInfo->GetSurfaceFormat().format;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			VkRenderPass renderPass;
			auto result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);

			if (result != VK_SUCCESS)
				throw VulkanException("Failed to create render pass. Result: " + to_string(result));

			return renderPass;
		}
		// Creates a new vulkan graphics pipeline framebuffer array
		static vector<VkFramebuffer> CreateFramebuffers(VkDevice device, VkRenderPass renderPass, VkExtent2D extent, const vector<VkImageView>& imageViews)
		{
			vector<VkFramebuffer> framebuffers(imageViews.size());

			for (size_t i = 0; i < imageViews.size(); i++)
			{
				VkImageView attachments[] =
				{
					imageViews[i]
				};

				VkFramebufferCreateInfo framebufferInfo = {};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = renderPass;
				framebufferInfo.attachmentCount = 1;
				framebufferInfo.pAttachments = attachments;
				framebufferInfo.width = extent.width;
				framebufferInfo.height = extent.height;
				framebufferInfo.layers = 1;

				auto result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]);
				if (result != VK_SUCCESS)
					throw VulkanException("Failed to create framebuffer. Result: " + to_string(result));
			}

			return framebuffers;
		}
		// Destroys vulkan graphics pipeline framebuffer array
		static void DestroyFramebuffers(VkDevice device, const vector<VkFramebuffer>& framebuffers)
		{
			for (auto framebuffer : framebuffers)
				vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

	public:
		// Creates a new vulkan graphics pipeline class instance
		Pipeline_T(VkDevice _device, WindowDeviceInfo deviceInfo, const vector<VkImageView>& swapchainImageViews)
		{
			device = _device;
			renderPass = CreateRenderPassInstance(_device, deviceInfo);

			auto vertShaderBytecode = Shader::ReadBytecode("Shaders/Engine/Unlit.vert.spv");
			auto vertShaderModule = CreateShaderModuleInstance(_device, vertShaderBytecode);

			auto fragShaderBytecode = Shader::ReadBytecode("Shaders/Engine/Unlit.frag.spv");
			auto fragShaderModule = CreateShaderModuleInstance(_device, fragShaderBytecode);

			VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = vertShaderModule;
			vertShaderStageInfo.pName = "main";
			vertShaderStageInfo.pSpecializationInfo = nullptr; // TODO

			VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = fragShaderModule;
			fragShaderStageInfo.pName = "main";
			fragShaderStageInfo.pSpecializationInfo = nullptr; // TODO

			VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

			VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = 0;
			vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
			vertexInputInfo.vertexAttributeDescriptionCount = 0;
			vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

			VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;

			VkExtent2D extent = deviceInfo->GetSurfaceExtent();
			viewport.width = (float)extent.width;
			viewport.height = (float)extent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };
			scissor.extent = extent;

			VkPipelineViewportStateCreateInfo viewportState = {};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;

			VkPipelineRasterizationStateCreateInfo rasterizer = {};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizer.depthBiasEnable = VK_FALSE;
			rasterizer.depthBiasConstantFactor = 0.0f; // Optional
			rasterizer.depthBiasClamp = 0.0f; // Optional
			rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

			VkPipelineMultisampleStateCreateInfo multisampling = {};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampling.minSampleShading = 1.0f; // Optional
			multisampling.pSampleMask = nullptr; // Optional
			multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
			multisampling.alphaToOneEnable = VK_FALSE; // Optional

			VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

			VkPipelineColorBlendStateCreateInfo colorBlending = {};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;
			colorBlending.blendConstants[0] = 0.0f; // Optional
			colorBlending.blendConstants[1] = 0.0f; // Optional
			colorBlending.blendConstants[2] = 0.0f; // Optional
			colorBlending.blendConstants[3] = 0.0f; // Optional

			VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = 0; // Optional
			pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
			pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
			pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

			auto result = vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &layout);
			if (result != VK_SUCCESS)
				throw VulkanException("Failed to create graphics pipeline layout. Result: " + to_string(result));

			VkGraphicsPipelineCreateInfo pipelineInfo = {};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaderStages;
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pDepthStencilState = nullptr; // Optional
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.pDynamicState = nullptr; // Optional
			pipelineInfo.layout = layout;
			pipelineInfo.renderPass = renderPass;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
			pipelineInfo.basePipelineIndex = -1; // Optional

			result = vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &instance);
			if (result != VK_SUCCESS)
				throw VulkanException("Failed to create graphics pipeline. Result: " + to_string(result));

			DestroyShaderModuleInstance(_device, vertShaderModule);
			DestroyShaderModuleInstance(_device, fragShaderModule);

			framebuffers = CreateFramebuffers(device, renderPass, extent, swapchainImageViews);
		}
		// Destroys vulkan graphics pipeline class instance
		~Pipeline_T()
		{
			DestroyFramebuffers(device, framebuffers);
			vkDestroyPipeline(device, instance, nullptr);
			vkDestroyPipelineLayout(device, layout, nullptr);
			vkDestroyRenderPass(device, renderPass, nullptr);
		}
	};

	// Vulkan graphics pipeline class instance
	typedef Pipeline_T* Pipeline;

	// Creates a new vulkan graphics pipeline class instance
	static Pipeline CreatePipelineInstance(VkDevice device, WindowDeviceInfo deviceInfo, const vector<VkImageView>& swapchainImageViews)
	{
		return new Pipeline_T(device, deviceInfo, swapchainImageViews);
	}
	// Destroys vulkan graphics pipeline class instance
	static void DestroyPipelineInstance(Pipeline instance)
	{
		delete instance;
	}
}

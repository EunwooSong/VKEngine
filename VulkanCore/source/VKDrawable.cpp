#include "VKDrawable.h"
#include "VKApplication.h"
#include "VKDevice.h"
#include "Wrappers.h"
#include "VKRenderer.h"

//********************VK_Drawable********************//
VKDrawable::VKDrawable(VKRenderer* parent)
{
	// Note: It's very important to initilize the member with 0 or respective value other wise it will break the system
	memset(&VertexBuffer, 0, sizeof(VertexBuffer));
	rendererObj = parent;
}

VKDrawable::~VKDrawable()
{
}

void VKDrawable::createVertexBuffer(const void* vertexData, uint32_t dataSize, uint32_t dataStride, bool useTexture)
{
	VKApplication* appObj = VKApplication::GetInstance();
	VKDevice* deviceObj = appObj->deviceObj;

	VkResult  result;
	bool  pass;

	// 버퍼 리소스의 메타 데이터 정보 설정
	VkBufferCreateInfo bufInfo = {};
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufInfo.pNext = NULL;
	bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufInfo.size = dataSize;
	bufInfo.queueFamilyIndexCount = 0;
	bufInfo.pQueueFamilyIndices = NULL;
	bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufInfo.flags = 0;

	// 버퍼 리소스 생성
	result = vkCreateBuffer(deviceObj->device, &bufInfo, NULL, &VertexBuffer.buf);
	assert(result == VK_SUCCESS);

	// 버퍼 리소스 요구사항 가져오기
	VkMemoryRequirements memRqrmnt;
	vkGetBufferMemoryRequirements(deviceObj->device, VertexBuffer.buf, &memRqrmnt);

	// 메모리 할당 메타 데이터 정보 설정
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.memoryTypeIndex = 0;
	allocInfo.allocationSize = memRqrmnt.size;

	// 호환되는 메모리 유형을 가져오기
	pass = deviceObj->memoryTypeFromProperties(memRqrmnt.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex);
	assert(pass);

	// 버퍼 리소스를 위한 물리적 저장 공간 할당
	result = vkAllocateMemory(deviceObj->device, &allocInfo, NULL, &(VertexBuffer.mem));
	assert(result == VK_SUCCESS);
	VertexBuffer.bufferInfo.range = memRqrmnt.size;
	VertexBuffer.bufferInfo.offset = 0;

	// 호스트에 물리적 장치 메모리 공간을 매핑
	uint8_t* pData;
	result = vkMapMemory(deviceObj->device, VertexBuffer.mem, 0, memRqrmnt.size, 0, (void**)&pData);
	assert(result == VK_SUCCESS);

	// 데이터를 매핑된 메모리에 복사
	memcpy(pData, vertexData, dataSize);

	// 장치 메모리를 매핑 해제
	vkUnmapMemory(deviceObj->device, VertexBuffer.mem);

	// 할당된 버퍼 리소스를 장치 메모리에 바인딩
	result = vkBindBufferMemory(deviceObj->device, VertexBuffer.buf, VertexBuffer.mem, 0);
	assert(result == VK_SUCCESS);

	// VkVertexInputBinding viIpBind는 버텍스 입력으로 들어오는 정보에서 정의된 속도로 저장
	
	viIpBind.binding = 0;
	viIpBind.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	viIpBind.stride = dataStride;

	// VkVertexInputAttribute(디스크립션) 구조체, 데이터 해석에 도움을 주는 정보를 저장
	// 위치, 색 특성과 관련된 정보.....
	viIpAttrb[0].binding = 0;
	viIpAttrb[0].location = 0;
	viIpAttrb[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	viIpAttrb[0].offset = 0;
	viIpAttrb[1].binding = 0;
	viIpAttrb[1].location = 1;
	viIpAttrb[1].format = useTexture ? VK_FORMAT_R32G32_SFLOAT : VK_FORMAT_R32G32B32A32_SFLOAT;
	viIpAttrb[1].offset = 16; // After, 4 components - RGBA  each of 4 bytes(32bits)
}

void VKDrawable::destroyVertexBuffer()
{
	VKDevice* deviceObj = rendererObj->getDevice();

	vkDestroyBuffer(deviceObj->device, VertexBuffer.buf, NULL);
	vkFreeMemory(deviceObj->device, VertexBuffer.mem, NULL);
}

void VKDrawable::destroyCommandBuffer()
{
	VKApplication* appObj = VKApplication::GetInstance();
	VKDevice* deviceObj = appObj->deviceObj;
	for (int i = 0; i < vecCmdDraw.size(); i++) {
		vkFreeCommandBuffers(deviceObj->device, rendererObj->cmdPool, 1, &vecCmdDraw[i]);
	}
}

void VKDrawable::destroySynchronizationObjects()
{
	VKApplication* appObj = VKApplication::GetInstance();
	VKDevice* deviceObj = appObj->deviceObj;
}

//********************VK_BACK_GROUND********************//
VKBackground::VKBackground(VKRenderer* parent)
{
	memset(&VertexBuffer, 0, sizeof(VertexBuffer));
	rendererObj = parent;
}

VKBackground::~VKBackground()
{
}

void VKBackground::prepare()
{
	VKDevice* deviceObj = rendererObj->getDevice();
	vecCmdDraw.resize(rendererObj->getSwapChain()->scPublicVars.colorBuffer.size()); // 스와프 버퍼 색상 이미지 만큼 커맨드 버퍼 할당

	// 각 스와프 버퍼의 색상 이미지 버퍼에 대해 해당 커맨드 버퍼를 할당..
	for (int i = 0; i < rendererObj->getSwapChain()->scPublicVars.colorBuffer.size(); i++) {
		// Allocate, create and start command buffer recording
		CommandBufferMgr::allocCommandBuffer(&deviceObj->device, *rendererObj->getCommandPool(), &vecCmdDraw[i]);
		CommandBufferMgr::beginCommandBuffer(vecCmdDraw[i]);

		// 렌더 패스 인스턴스 생성
		recordCommandBuffer(i, &vecCmdDraw[i]);

		// Finish the command buffer recording
		CommandBufferMgr::endCommandBuffer(vecCmdDraw[i]);
	}
}

void VKBackground::render()
{
	VKDevice* deviceObj = rendererObj->getDevice();
	VKSwapChain* swapChainObj = rendererObj->getSwapChain();

	uint32_t& currentColorImage = swapChainObj->scPublicVars.currentColorBuffer;
	VkSwapchainKHR& swapChain = swapChainObj->scPublicVars.swapChain;

	VkSemaphore presentCompleteSemaphore;
	VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo;
	presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	presentCompleteSemaphoreCreateInfo.pNext = NULL;
	presentCompleteSemaphoreCreateInfo.flags = 0;
	vkCreateSemaphore(deviceObj->device, &presentCompleteSemaphoreCreateInfo, NULL, &presentCompleteSemaphore);

	// 사용 가능한 다음 스와프 체인 이미지의 인덱스를 가져오기
	VkResult result = swapChainObj->fpAcquireNextImageKHR(deviceObj->device, swapChain,
		UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, &currentColorImage);

	// 커맨드 버퍼를 실행하기 위해 큐에 제출
	CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &vecCmdDraw[currentColorImage], NULL);

	//// 윈도에 이미지 표시
	//VkPresentInfoKHR present = {};
	//present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	//present.swapchainCount = 1;
	//present.pSwapchains = &swapChain;
	//present.pImageIndices = &currentColorImage;

	//// 표시를 위해 이미지를 큐에 제출
	//result = swapChainObj->fpQueuePresentKHR(deviceObj->queue, &present);
	//assert(result == VK_SUCCESS);

	vkDestroySemaphore(deviceObj->device, presentCompleteSemaphore, NULL);
}

void VKBackground::recordCommandBuffer(int currentBuffer, VkCommandBuffer* cmdDraw)
{
	// 지우기 색상값 지정
	VkClearValue clearValues[2];
	switch (currentBuffer)
	{
	case 0:
		clearValues[0].color = { 0.0f, 1.0f, 0.0f, 0.0f };
		break;

	case 1:
		clearValues[0].color = { 0.0f, 1.0f, 0.0f, 0.0f };
		break;

	case 2:
		clearValues[0].color = { 0.0f, 1.0f, 0.0f, 0.0f };
		break;

	default:
		clearValues[0].color = { 0.0f, 1.0f, 0.0f, 0.0f };
		break;
	}

	// 깊이/스텐실 지우기 값 지정
	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	// VkRenderPassBeginInfo 제어 구조체의 정의
	VkRenderPassBeginInfo renderPassBegin = {};
	renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBegin.pNext = NULL;
	renderPassBegin.renderPass = rendererObj->renderPass;
	renderPassBegin.framebuffer = rendererObj->framebuffers[currentBuffer];
	renderPassBegin.renderArea.offset.x = 0;
	renderPassBegin.renderArea.offset.y = 0;
	renderPassBegin.renderArea.extent.width = rendererObj->width;
	renderPassBegin.renderArea.extent.height = rendererObj->height;
	renderPassBegin.clearValueCount = 2;
	renderPassBegin.pClearValues = clearValues;

	// 렌더 패스 인스턴스의 레코딩 시작
	vkCmdBeginRenderPass(*cmdDraw, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

	// 렌더 패스 인스턴스의 레코딩 종료
	vkCmdEndRenderPass(*cmdDraw);
}

//********************VK_TRIANGLE********************//
VKTriangle::VKTriangle(VKRenderer* parent)
{
	// Note: It's very important to initilize the member with 0 or respective value other wise it will break the system
	memset(&VertexBuffer, 0, sizeof(VertexBuffer));
	rendererObj = parent;

	VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo;
	presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	presentCompleteSemaphoreCreateInfo.pNext = NULL;
	presentCompleteSemaphoreCreateInfo.flags = 0;

	VkSemaphoreCreateInfo drawingCompleteSemaphoreCreateInfo;
	drawingCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	drawingCompleteSemaphoreCreateInfo.pNext = NULL;
	drawingCompleteSemaphoreCreateInfo.flags = 0;

	VKDevice* deviceObj = VKApplication::GetInstance()->deviceObj;

	vkCreateSemaphore(deviceObj->device, &presentCompleteSemaphoreCreateInfo, NULL, &presentCompleteSemaphore);
	vkCreateSemaphore(deviceObj->device, &drawingCompleteSemaphoreCreateInfo, NULL, &drawingCompleteSemaphore);
}

VKTriangle::~VKTriangle()
{
}

void VKTriangle::prepare()
{
	VKDevice* deviceObj = rendererObj->getDevice();
	vecCmdDraw.resize(rendererObj->getSwapChain()->scPublicVars.colorBuffer.size());
	// For each swapbuffer color surface image buffer 
	// allocate the corresponding command buffer
	for (int i = 0; i < rendererObj->getSwapChain()->scPublicVars.colorBuffer.size(); i++) {
		// Allocate, create and start command buffer recording
		CommandBufferMgr::allocCommandBuffer(&deviceObj->device, *rendererObj->getCommandPool(), &vecCmdDraw[i]);
		CommandBufferMgr::beginCommandBuffer(vecCmdDraw[i]);

		// Create the render pass instance 
		recordCommandBuffer(i, &vecCmdDraw[i]);

		// Finish the command buffer recording
		CommandBufferMgr::endCommandBuffer(vecCmdDraw[i]);
	}
}

void VKTriangle::render()
{
	VKDevice* deviceObj = rendererObj->getDevice();
	VKSwapChain* swapChainObj = rendererObj->getSwapChain();

	uint32_t& currentColorImage = swapChainObj->scPublicVars.currentColorBuffer;
	VkSwapchainKHR& swapChain = swapChainObj->scPublicVars.swapChain;

	VkFence nullFence = VK_NULL_HANDLE;

	// Get the index of the next available swapchain image:
	VkResult result = swapChainObj->fpAcquireNextImageKHR(deviceObj->device, swapChain,
		UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, &currentColorImage);

	VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
	submitInfo.pWaitDstStageMask = &submitPipelineStages;
	submitInfo.commandBufferCount = (uint32_t)sizeof(&vecCmdDraw[currentColorImage]) / sizeof(VkCommandBuffer);
	submitInfo.pCommandBuffers = &vecCmdDraw[currentColorImage];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &drawingCompleteSemaphore;

	// Queue the command buffer for execution
	CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &vecCmdDraw[currentColorImage], &submitInfo);

	//// Present the image in the window
	//VkPresentInfoKHR present = {};
	//present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	//present.pNext = NULL;
	//present.swapchainCount = 1;
	//present.pSwapchains = &swapChain;
	//present.pImageIndices = &currentColorImage;
	//present.pWaitSemaphores = &drawingCompleteSemaphore;
	//present.waitSemaphoreCount = 1;
	//present.pResults = NULL;

	//// Queue the image for presentation,
	//result = swapChainObj->fpQueuePresentKHR(deviceObj->queue, &present);
}

void VKTriangle::initViewports(VkCommandBuffer* cmd)
{
	viewport.height = (float)rendererObj->height;
	viewport.width = (float)rendererObj->width;
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	viewport.x = 0;
	viewport.y = 0;
	vkCmdSetViewport(*cmd, 0, NUMBER_OF_VIEWPORTS, &viewport);
}

void VKTriangle::initScissors(VkCommandBuffer* cmd)
{
	scissor.extent.width = rendererObj->width;
	scissor.extent.height = rendererObj->height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(*cmd, 0, NUMBER_OF_SCISSORS, &scissor);
}

void VKTriangle::recordCommandBuffer(int currentBuffer, VkCommandBuffer* cmdDraw)
{
	VKDevice* deviceObj = rendererObj->getDevice();

	VkClearValue clearValues[2];
	clearValues[0].color.float32[0] = 0.0f;
	clearValues[0].color.float32[1] = 0.0f;
	clearValues[0].color.float32[2] = 0.0f;
	clearValues[0].color.float32[3] = 0.0f;

	// Specify the depth/stencil clear value
	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	// Define the VkRenderPassBeginInfo control structure
	VkRenderPassBeginInfo renderPassBegin = {};
	renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBegin.pNext = NULL;
	renderPassBegin.renderPass = rendererObj->renderPass;
	renderPassBegin.framebuffer = rendererObj->framebuffers[currentBuffer];
	renderPassBegin.renderArea.offset.x = 0;
	renderPassBegin.renderArea.offset.y = 0;
	renderPassBegin.renderArea.extent.width = rendererObj->width;
	renderPassBegin.renderArea.extent.height = rendererObj->height;
	renderPassBegin.clearValueCount = 2;
	renderPassBegin.pClearValues = clearValues;

	// Start recording the render pass instance
	vkCmdBeginRenderPass(*cmdDraw, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

	// Bound the command buffer with the graphics pipeline
	vkCmdBindPipeline(*cmdDraw, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);

	// Bound the command buffer with the graphics pipeline
	const VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(*cmdDraw, 0, 1, &VertexBuffer.buf, offsets);

	// Define the dynamic viewport here
	initViewports(cmdDraw);

	// Define the scissoring 
	initScissors(cmdDraw);

	// Issue the draw command with 3 vertex, 1 instance starting 
	// from first vertex
	vkCmdDraw(*cmdDraw, 3, 1, 0, 0);

	// End of render pass instance recording
	vkCmdEndRenderPass(*cmdDraw);
}
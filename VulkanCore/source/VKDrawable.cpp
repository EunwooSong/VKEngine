#include "VKDrawable.h"
#include "VKApplication.h"
#include "VKDevice.h"
#include "Wrappers.h"
#include "VKRenderer.h"

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

	// ���� ���ҽ��� ��Ÿ ������ ���� ����
	VkBufferCreateInfo bufInfo = {};
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufInfo.pNext = NULL;
	bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufInfo.size = dataSize;
	bufInfo.queueFamilyIndexCount = 0;
	bufInfo.pQueueFamilyIndices = NULL;
	bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufInfo.flags = 0;

	// ���� ���ҽ� ����
	result = vkCreateBuffer(deviceObj->device, &bufInfo, NULL, &VertexBuffer.buf);
	assert(result == VK_SUCCESS);

	// ���� ���ҽ� �䱸���� ��������
	VkMemoryRequirements memRqrmnt;
	vkGetBufferMemoryRequirements(deviceObj->device, VertexBuffer.buf, &memRqrmnt);

	// �޸� �Ҵ� ��Ÿ ������ ���� ����
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.memoryTypeIndex = 0;
	allocInfo.allocationSize = memRqrmnt.size;

	// ȣȯ�Ǵ� �޸� ������ ��������
	pass = deviceObj->memoryTypeFromProperties(memRqrmnt.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex);
	assert(pass);

	// ���� ���ҽ��� ���� ������ ���� ���� �Ҵ�
	result = vkAllocateMemory(deviceObj->device, &allocInfo, NULL, &(VertexBuffer.mem));
	assert(result == VK_SUCCESS);
	VertexBuffer.bufferInfo.range = memRqrmnt.size;
	VertexBuffer.bufferInfo.offset = 0;

	// ȣ��Ʈ�� ������ ��ġ �޸� ������ ����
	uint8_t* pData;
	result = vkMapMemory(deviceObj->device, VertexBuffer.mem, 0, memRqrmnt.size, 0, (void**)&pData);
	assert(result == VK_SUCCESS);

	// �����͸� ���ε� �޸𸮿� ����
	memcpy(pData, vertexData, dataSize);

	// ��ġ �޸𸮸� ���� ����
	vkUnmapMemory(deviceObj->device, VertexBuffer.mem);

	// �Ҵ�� ���� ���ҽ��� ��ġ �޸𸮿� ���ε�
	result = vkBindBufferMemory(deviceObj->device, VertexBuffer.buf, VertexBuffer.mem, 0);
	assert(result == VK_SUCCESS);

	// VkVertexInputBinding viIpBind�� ���ؽ� �Է����� ������ �������� ���ǵ� �ӵ��� ����
	
	viIpBind.binding = 0;
	viIpBind.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	viIpBind.stride = dataStride;

	// VkVertexInputAttribute(��ũ����) ����ü, ������ �ؼ��� ������ �ִ� ������ ����
	// ��ġ, �� Ư���� ���õ� ����.....
	viIpAttrb[0].binding = 0;
	viIpAttrb[0].location = 0;
	viIpAttrb[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	viIpAttrb[0].offset = 0;
	viIpAttrb[1].binding = 0;
	viIpAttrb[1].location = 1;
	viIpAttrb[1].format = useTexture ? VK_FORMAT_R32G32_SFLOAT : VK_FORMAT_R32G32B32A32_SFLOAT;
	viIpAttrb[1].offset = 16; // After, 4 components - RGBA  each of 4 bytes(32bits)
}

void VKDrawable::prepare()
{
	VKDevice* deviceObj = rendererObj->getDevice();
	vecCmdDraw.resize(rendererObj->getSwapChain()->scPublicVars.colorBuffer.size()); // ������ ���� ���� �̹��� ��ŭ Ŀ�ǵ� ���� �Ҵ�

	// �� ������ ������ ���� �̹��� ���ۿ� ���� �ش� Ŀ�ǵ� ���۸� �Ҵ�..
	for (int i = 0; i < rendererObj->getSwapChain()->scPublicVars.colorBuffer.size(); i++) {
		// Allocate, create and start command buffer recording
		CommandBufferMgr::allocCommandBuffer(&deviceObj->device, *rendererObj->getCommandPool(), &vecCmdDraw[i]);
		CommandBufferMgr::beginCommandBuffer(vecCmdDraw[i]);

		// ���� �н� �ν��Ͻ� ����
		recordCommandBuffer(i, &vecCmdDraw[i]);

		// Finish the command buffer recording
		CommandBufferMgr::endCommandBuffer(vecCmdDraw[i]);
	}
}

void VKDrawable::render()
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

	Sleep(1000);

	// ��� ������ ���� ������ ü�� �̹����� �ε����� ��������
	VkResult result = swapChainObj->fpAcquireNextImageKHR(deviceObj->device, swapChain,
		UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, &currentColorImage);

	// Ŀ�ǵ� ���۸� �����ϱ� ���� ť�� ����
	CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &vecCmdDraw[currentColorImage], NULL);

	// ������ �̹��� ǥ��
	VkPresentInfoKHR present = {};
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.swapchainCount = 1;
	present.pSwapchains = &swapChain;
	present.pImageIndices = &currentColorImage;

	// ǥ�ø� ���� �̹����� ť�� ����
	result = swapChainObj->fpQueuePresentKHR(deviceObj->queue, &present);
	assert(result == VK_SUCCESS);

	vkDestroySemaphore(deviceObj->device, presentCompleteSemaphore, NULL);
}

void VKDrawable::destroyVertexBuffer()
{
	VKDevice* deviceObj = rendererObj->getDevice();

	vkDestroyBuffer(deviceObj->device, VertexBuffer.buf, NULL);
	vkFreeMemory(deviceObj->device, VertexBuffer.mem, NULL);
}

void VKDrawable::recordCommandBuffer(int currentBuffer, VkCommandBuffer* cmdDraw)
{
	// ����� ���� ����
	VkClearValue clearValues[2];
	switch (currentBuffer)
	{
	case 0:
		clearValues[0].color = { 1.0f, 0.0f, 0.0f, 0.0f };
		break;

	case 1:
		clearValues[0].color = { 0.0f, 1.0f, 0.0f, 0.0f };
		break;

	case 2:
		clearValues[0].color = { 0.0f, 0.0f, 1.0f, 0.0f };
		break;

	default:
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		break;
	}

	// ����/���ٽ� ����� �� ����
	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	// VkRenderPassBeginInfo ���� ����ü�� ����
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

	// ���� �н� �ν��Ͻ��� ���ڵ� ����
	vkCmdBeginRenderPass(*cmdDraw, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

	// ���� �н� �ν��Ͻ��� ���ڵ� ����
	vkCmdEndRenderPass(*cmdDraw);
}
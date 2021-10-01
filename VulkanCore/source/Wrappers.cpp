#include "Wrappers.h"


/***************COMMAND BUFFER WRAPPERS***************/
void CommandBufferMgr::allocCommandBuffer(const VkDevice* device, const VkCommandPool cmdPool, VkCommandBuffer* cmdBuf, const VkCommandBufferAllocateInfo* commandBufferInfo)
{
	VkResult result;

	// Ŀ�ǵ� ������ ����� �� ������ �״�� ���
	if (commandBufferInfo) {
		result = vkAllocateCommandBuffers(*device, commandBufferInfo, cmdBuf);
		assert(!result);
		return;
	}

	// �⺻ ����, Ŀ�ǵ� ���� �Ҵ� ������ �����ϰ� ������ �Ķ���͸� �־��ش�.
	VkCommandBufferAllocateInfo cmdInfo = {};
	cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdInfo.pNext = NULL;
	cmdInfo.commandPool = cmdPool;
	cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdInfo.commandBufferCount = (uint32_t)sizeof(cmdBuf) / sizeof(VkCommandBuffer);;

	// �޸� �Ҵ�
	result = vkAllocateCommandBuffers(*device, &cmdInfo, cmdBuf);
	assert(!result);
}


void CommandBufferMgr::beginCommandBuffer(VkCommandBuffer cmdBuf, VkCommandBufferBeginInfo* inCmdBufInfo)
{
	VkResult  result;
	// ����ڰ� �ڽ��� Ŀ�ǵ� ���۸� ������ ��� �̸� ���
	if (inCmdBufInfo) {
		result = vkBeginCommandBuffer(cmdBuf, inCmdBufInfo);
		assert(result == VK_SUCCESS);
		return;
	}

	// �ƴϸ� �⺻ ���� ���
	VkCommandBufferInheritanceInfo cmdBufInheritInfo = {};
	cmdBufInheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	cmdBufInheritInfo.pNext = NULL;
	cmdBufInheritInfo.renderPass = VK_NULL_HANDLE;
	cmdBufInheritInfo.subpass = 0;
	cmdBufInheritInfo.framebuffer = VK_NULL_HANDLE;
	cmdBufInheritInfo.occlusionQueryEnable = VK_FALSE;
	cmdBufInheritInfo.queryFlags = 0;
	cmdBufInheritInfo.pipelineStatistics = 0;

	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.pNext = NULL;
	cmdBufInfo.flags = 0;
	cmdBufInfo.pInheritanceInfo = &cmdBufInheritInfo;

	result = vkBeginCommandBuffer(cmdBuf, &cmdBufInfo);

	assert(result == VK_SUCCESS);
}

void CommandBufferMgr::endCommandBuffer(VkCommandBuffer cmdBuf)
{
	VkResult  result;
	result = vkEndCommandBuffer(cmdBuf);
	assert(result == VK_SUCCESS);
}

void CommandBufferMgr::submitCommandBuffer(const VkQueue& queue, const VkCommandBuffer* cmdBufList, const VkSubmitInfo* inSubmitInfo, const VkFence& fence)
{
	VkResult result;

	// ����� ������ ����� �� ������ �̸� ����Ѵ�.
	// �̴� ����ü ���� Ŀ�ǵ尡 �̹� ������ �ִٰ� �����Ѵ�.
	// ���� Ŀ�ǵ� ���۸� �����Ѵ�.
	if (inSubmitInfo) {
		result = vkQueueSubmit(queue, 1, inSubmitInfo, fence);
		assert(!result);

		result = vkQueueWaitIdle(queue);
		assert(!result);
		return;
	}

	// �ƴϸ� ������ ������ ������ ���� ������� �����Ѵ�.
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = NULL;
	submitInfo.pWaitDstStageMask = NULL;
	submitInfo.commandBufferCount = (uint32_t)sizeof(cmdBufList) / sizeof(VkCommandBuffer);
	submitInfo.pCommandBuffers = cmdBufList;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = NULL;

	result = vkQueueSubmit(queue, 1, &submitInfo, fence);
	assert(!result);

	result = vkQueueWaitIdle(queue);
	assert(!result);
}
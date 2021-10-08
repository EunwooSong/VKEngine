#include "VKDrawable.h"
#include "VKApplication.h"
#include "VKDevice.h"
#include "Wrappers.h"
#include "VKRenderer.h"
#include "MeshData.h"

//********************VK_Drawable********************//
VKDrawable::VKDrawable(VKRenderer* parent)
{
	memset(&uniformData, 0, sizeof(UniformData));

	// Note: It's very important to initilize the member with 0 or respective value other wise it will break the system
	rendererObj = parent;

	// VkVertexInputBinding viIpBind�� ���ؽ� �Է����� ������ �������� ���ǵ� �ӵ��� ����
	viIpBind.binding = 0;
	viIpBind.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	viIpBind.stride = sizeof(squareData1[0]);

	// VkVertexInputAttribute(��ũ����) ����ü, ������ �ؼ��� ������ �ִ� ������ ����
	// ��ġ, �� Ư���� ���õ� ����.....
	viIpAttrb[0].binding = 0;
	viIpAttrb[0].location = 0;
	viIpAttrb[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	viIpAttrb[0].offset = 0;
	viIpAttrb[1].binding = 0;
	viIpAttrb[1].location = 1;
	viIpAttrb[1].format = false ? VK_FORMAT_R32G32_SFLOAT : VK_FORMAT_R32G32B32A32_SFLOAT;
	viIpAttrb[1].offset = 16; // After, 4 components - RGBA  each of 4 bytes(32bits)
}

VKDrawable::~VKDrawable()
{
}

void VKDrawable::createVertexBuffer(const void* vertexData, ModelObj* model, uint32_t dataSize, uint32_t dataStride, bool useTexture)
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
	result = vkCreateBuffer(deviceObj->device, &bufInfo, NULL, &model->vBuffer.buf);
	assert(result == VK_SUCCESS);

	// ���� ���ҽ� �䱸���� ��������
	VkMemoryRequirements memRqrmnt;
	vkGetBufferMemoryRequirements(deviceObj->device, model->vBuffer.buf, &memRqrmnt);

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
	result = vkAllocateMemory(deviceObj->device, &allocInfo, NULL, &(model->vBuffer.mem));
	assert(result == VK_SUCCESS);
	model->vBuffer.bufferInfo.range = memRqrmnt.size;
	model->vBuffer.bufferInfo.offset = 0;

	// ȣ��Ʈ�� ������ ��ġ �޸� ������ ����
	uint8_t* pData;
	result = vkMapMemory(deviceObj->device, model->vBuffer.mem, 0, memRqrmnt.size, 0, (void**)&pData);
	assert(result == VK_SUCCESS);

	// �����͸� ���ε� �޸𸮿� ����
	memcpy(pData, vertexData, dataSize);

	// ��ġ �޸𸮸� ���� ����
	vkUnmapMemory(deviceObj->device, model->vBuffer.mem);

	// �Ҵ�� ���� ���ҽ��� ��ġ �޸𸮿� ���ε�
	result = vkBindBufferMemory(deviceObj->device, model->vBuffer.buf, model->vBuffer.mem, 0);
	assert(result == VK_SUCCESS);
}

void VKDrawable::createVertexIndex(const void* indexData, ModelObj* model, uint32_t dataSize, uint32_t dataStride)
{
	VKApplication* appObj = VKApplication::GetInstance();
	VKDevice* deviceObj = appObj->deviceObj;

	VkResult	result;
	bool		pass;

	// Create the Buffer resourece metadata information
	VkBufferCreateInfo bufInfo = {};
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufInfo.pNext = NULL;
	bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufInfo.size = dataSize;
	bufInfo.queueFamilyIndexCount = 0;
	bufInfo.pQueueFamilyIndices = NULL;
	bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufInfo.flags = 0;

	// Create the Buffer resource
	result = vkCreateBuffer(deviceObj->device, &bufInfo, NULL, &model->vIndex.idx);
	assert(result == VK_SUCCESS);

	// Get the Buffer resource requirements
	VkMemoryRequirements memRqrmnt;
	vkGetBufferMemoryRequirements(deviceObj->device, model->vIndex.idx, &memRqrmnt);

	// Create memory allocation metadata information
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.memoryTypeIndex = 0;
	allocInfo.allocationSize = memRqrmnt.size;

	// Get the compatible type of memory
	pass = deviceObj->memoryTypeFromProperties(memRqrmnt.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex);
	assert(pass);

	// Allocate the physical backing for buffer resource
	result = vkAllocateMemory(deviceObj->device, &allocInfo, NULL, &(model->vIndex.mem));
	assert(result == VK_SUCCESS);
	model->vIndex.bufferInfo.range = memRqrmnt.size;
	model->vIndex.bufferInfo.offset = 0;

	// Map the physical device memory region to the host 
	uint8_t* pData;
	result = vkMapMemory(deviceObj->device, model->vIndex.mem, 0, memRqrmnt.size, 0, (void**)&pData);
	assert(result == VK_SUCCESS);

	// Copy the data in the mapped memory
	memcpy(pData, indexData, dataSize);

	// Unmap the device memory
	vkUnmapMemory(deviceObj->device, model->vIndex.mem);

	// Bind the allocated buffer resource to the device memory
	result = vkBindBufferMemory(deviceObj->device, model->vIndex.idx, model->vIndex.mem, 0);
	assert(result == VK_SUCCESS);
}

void VKDrawable::prepare()
{
	//����� Ŀ�ǵ� ���۸� �Ҵ��Ѵ�.
	VKDevice* deviceObj = rendererObj->getDevice();
	CommandBufferMgr::allocCommandBuffer(&deviceObj->device, *rendererObj->getCommandPool(), &cmdDraw);

	//���� ������Ʈ�� ����Ѵ�.
	createRenderObject(loadModel(3));
}

void VKDrawable::update()
{
	VKDevice* deviceObj = rendererObj->getDevice();
	Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	View = glm::lookAt(
		glm::vec3(0, 0, 5),		// Camera is in World Space
		glm::vec3(0, 0, 0),		// and looks at the origin
		glm::vec3(0, 1, 0)		// Head is up
	);
	Model = glm::mat4(1.0f);
	static float rot = 0;
	rot += .0005f;
	Model = glm::rotate(Model, rot, glm::vec3(0.0, 1.0, 0.0))
		* glm::rotate(Model, rot, glm::vec3(1.0, 1.0, 1.0));

	glm::mat4 MVP = Projection * View * Model;

	// Invalidate the range of mapped buffer in order to make it visible to the host.
	// If the memory property is set with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	// then the driver may take care of this, otherwise for non-coherent 
	// mapped memory vkInvalidateMappedMemoryRanges() needs to be called explicitly.
	VkResult res = vkInvalidateMappedMemoryRanges(deviceObj->device, 1, &uniformData.mappedRange[0]);
	assert(res == VK_SUCCESS);

	// Copy updated data into the mapped memory
	memcpy(uniformData.pData, &MVP, sizeof(MVP));

	// Flush the range of mapped buffer in order to make it visible to the device
	// If the memory is coherent (memory property must be beVK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	// then the driver may take care of this, otherwise for non-coherent 
	// mapped memory vkFlushMappedMemoryRanges() needs to be called explicitly to flush out 
	// the pending writes on the host side.
	res = vkFlushMappedMemoryRanges(deviceObj->device, 1, &uniformData.mappedRange[0]);
	assert(res == VK_SUCCESS);
}

void VKDrawable::render(VkSemaphore* presentCompleteSemaphore, VkSemaphore* drawingCompleteSemaphore)
{
	// 1. Ŀ�ǵ� ���۸� �ۼ��ϰ�
	CommandBufferMgr::beginCommandBuffer(cmdDraw);

	// 2. ���� �н��� �����Ѵ�.
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
	renderPassBegin.framebuffer = rendererObj->framebuffers[rendererObj->getSwapChain()->scPublicVars.currentColorBuffer];
	renderPassBegin.renderArea.offset.x = 0;
	renderPassBegin.renderArea.offset.y = 0;
	renderPassBegin.renderArea.extent.width = rendererObj->width;
	renderPassBegin.renderArea.extent.height = rendererObj->height;
	renderPassBegin.clearValueCount = 2;
	renderPassBegin.pClearValues = clearValues;


	vkCmdBeginRenderPass(cmdDraw, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

	// 3. ���� ����Ʈ�� �����Ѵ�
	setViewports();
	setScissors();

	// 4. ������������ ���ε� �Ѵ�.
	vkCmdBindPipeline(cmdDraw, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);

	vkCmdBindDescriptorSets(cmdDraw, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descriptorSet.data(), 0, NULL);
	
	// 5. ����� �� ��ü�� ���ؽ����� ���ε� �Ѵ�.
	const VkDeviceSize offsets[1] = { 0 };

	//��� ���� ������Ʈ ���ε�
	//pushCoonstants
	//vkCmdBineIndex
	//vkBindIndexBuffer
	for (RenderObj* obj : renderObjs) {
		ModelObj* model = models[obj->modelIndex];

		// ���ؽ� ���� ���ε�
		vkCmdBindVertexBuffers(cmdDraw, 0, 1, &model->vBuffer.buf, offsets);
		
		// �ε��� ���� ���ε�
		//vkCmdBindIndexBuffer(cmdDraw, model->vIndex.idx, 0, VK_INDEX_TYPE_UINT16);

		// �ε��� ���� �׷�
		//vkCmdDrawIndexed(cmdDraw, 6, 1, 0, 0, 0);
	}

	//�׷����~
	vkCmdDraw(cmdDraw, 3 * 2 * 6, 1 ,0 ,0);

	//���� �н��� �����Ѵ�.
	vkCmdEndRenderPass(cmdDraw);

	//Ŀ�ǵ� ���۸� �ۼ��� �����Ѵ�. 
	CommandBufferMgr::endCommandBuffer(cmdDraw);

	//Ŀ�ǵ� ���۸� �����Ѵ�.
	VKDevice* deviceObj = VKApplication::GetInstance()->deviceObj;

	VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = presentCompleteSemaphore;
	submitInfo.pWaitDstStageMask = &submitPipelineStages;
	submitInfo.commandBufferCount = (uint32_t)sizeof(cmdDraw) / sizeof(VkCommandBuffer);
	submitInfo.pCommandBuffers = &cmdDraw;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = drawingCompleteSemaphore;

	CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &cmdDraw, &submitInfo);
}

int VKDrawable::loadModel(int type)
{
	if (models.find(type) != models.end())
		return type;

	ModelObj* iter = new ModelObj();
	VKDevice* deviceObj = VKApplication::GetInstance()->deviceObj;

	rendererObj->getCommandPool();
	//Ŀ�ǵ� ���� �ۼ�,,,
	CommandBufferMgr::beginCommandBuffer(rendererObj->cmdVertexBuffer);

	// ����ü �ʱ�ȭ
	memset(&iter->vBuffer, 0, sizeof(VertexBuffer));

	switch (type)
	{
	case 0:
		createVertexBuffer(squareData1, iter, sizeof(squareData1), sizeof(squareData1[0]), false);
		createVertexIndex(squareIndices, iter, sizeof(squareIndices), sizeof(squareIndices[0]));
		break;
	case 1:
		createVertexBuffer(squareData2, iter, sizeof(squareData2), sizeof(squareData2[0]), false);
		createVertexIndex(squareIndices, iter, sizeof(squareIndices), sizeof(squareIndices[0]));
		break;
	case 2:
		createVertexBuffer(triangleData, iter, sizeof(triangleData), sizeof(triangleData[0]), false);
		createVertexIndex(squareIndices, iter, sizeof(squareIndices), sizeof(squareIndices[0]));
		break;
	case 3:
		createVertexBuffer(geometryData, iter, sizeof(geometryData), sizeof(geometryData[0]), false);
		createVertexIndex(squareIndices, iter, sizeof(squareIndices), sizeof(squareIndices[0]));

		break;
	default:
		break;
	}

	models[type] = iter;

	CommandBufferMgr::endCommandBuffer(rendererObj->cmdVertexBuffer);
	CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &rendererObj->cmdVertexBuffer);

	return type;
}

RenderObj* VKDrawable::createRenderObject(int modelIndex)
{
	RenderObj* iter = new RenderObj();
	iter->modelIndex = modelIndex;

	renderObjs.push_back(iter);

	return iter;
}

void VKDrawable::createUniformBuffer()
{
	VkResult  result;
	bool  pass;
	Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	View = glm::lookAt(
		glm::vec3(10, 3, 10),	// ī�޶�� ���� ��ǥ�迡 �ְ�
		glm::vec3(0, 0, 0),		// ������ ���� ����
		glm::vec3(0, -1, 0)		// ���� ����
	);
	Model = glm::mat4(1.0f);
	MVP = Projection * View * Model;

	// VkBufferCreateInfo�� ���� ���ҽ� ������Ʈ ����
	VkBufferCreateInfo bufInfo = {};
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufInfo.pNext = NULL;
	bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufInfo.size = sizeof(MVP);
	bufInfo.queueFamilyIndexCount = 0;
	bufInfo.pQueueFamilyIndices = NULL;
	bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufInfo.flags = 0;

	// ���� ������ ���� ��ü�� �����ϴ� �� ���
	result = vkCreateBuffer(deviceObj->device, &bufInfo, NULL, &uniformData.buffer);
	assert(result == VK_SUCCESS);

	// ���� �޸� �䱸���� ���
	VkMemoryRequirements memRqrmnt;
	vkGetBufferMemoryRequirements(deviceObj->device, uniformData.buffer, &memRqrmnt);

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext = NULL;
	memAllocInfo.memoryTypeIndex = 0;
	memAllocInfo.allocationSize = memRqrmnt.size;

	// �޸� �Ӽ����� �ʿ��� �޸� ���� ����
	pass = deviceObj->memoryTypeFromProperties(memRqrmnt.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memAllocInfo.memoryTypeIndex);
	assert(pass);

	// ���� ��ü�� �޸� �Ҵ�
	result = vkAllocateMemory(deviceObj->device, &memAllocInfo, NULL, &(uniformData.memory));
	assert(result == VK_SUCCESS);

	// ���ε� ����
	result = vkMapMemory(deviceObj->device, uniformData.memory, 0, memRqrmnt.size, 0, (void**)&uniformData.pData);
	assert(result == VK_SUCCESS);

	// ���ε� ���ۿ��� ���� ������ ����
	memcpy(uniformData.pData, &MVP, sizeof(MVP));

	// ������Ʈ�� ������ ���� ��ü�� 1��
	uniformData.mappedRange.resize(1);

	// VkMappedMemoryRange ������ ����ü ä���
	uniformData.mappedRange[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	uniformData.mappedRange[0].memory = uniformData.memory;
	uniformData.mappedRange[0].offset = 0;
	uniformData.mappedRange[0].size = sizeof(MVP);

	// ȣ��Ʈ���� �� �� �ְ� ���ε� ������ ������ ��ȿȭ
	vkInvalidateMappedMemoryRanges(deviceObj->device, 1, &uniformData.mappedRange[0]);

	//���� ��ġ �޸𸮸� ���ε� ��
	result = vkBindBufferMemory(deviceObj->device, uniformData.buffer, uniformData.memory, 0);
	assert(result == VK_SUCCESS);

	// ������ ���� ������ ���۷� ���� ������ ����ü�� ������Ʈ
	uniformData.bufferInfo.buffer = uniformData.buffer;
	uniformData.bufferInfo.offset = 0;
	uniformData.bufferInfo.range = sizeof(MVP);
	uniformData.memRqrmnt = memRqrmnt;
}

void VKDrawable::createDescriptorPool(bool useTexture)
{
	VkResult  result;
	// ����� ��ũ���� ��Ʈ ������ ������� ��ũ���� Ǯ�� ũ�� ����
	std::vector<VkDescriptorPoolSize> descriptorTypePool;

	// ù ��° ��ũ���� Ǯ ��ü�� ������ ���� ����
	descriptorTypePool.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 });

	// �ؽ�ó�� �����Ǹ� �� ��° ��ü�� ��ũ���� ���� �̹��� ���÷��� ����
	if (useTexture) {
		descriptorTypePool.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 });
	}

	// ������ ����ü�� ��ũ���� Ǯ ������Ʈ ���� ä�� �ֱ�
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = NULL;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.poolSizeCount = (uint32_t)descriptorTypePool.size();
	descriptorPoolCreateInfo.pPoolSizes = descriptorTypePool.data();

	// ��ũ���� Ǯ ���� ����
	result = vkCreateDescriptorPool(deviceObj->device, &descriptorPoolCreateInfo, NULL, &descriptorPool);
	assert(result == VK_SUCCESS);
}

void VKDrawable::createDescriptorResources()
{
	// ������ ���ҽ��� ���ο��� ����, ��ũ���� ��Ʈ�� ����
	// ��ũ���� ��Ʈ�� �����ϱ� ���� �ڿ��� ����
	createUniformBuffer();
	
}

void VKDrawable::createDescriptorSet(bool useTexture)
{
	VKPipeline* pipelineObj = rendererObj->getPipelineObject();
	VkResult  result;

	// ��ũ���� �Ҵ� ����ü�� �����ϰ� ��ũ���� Ǯ�� ��ũ���� ���̾ƿ��� ����
	VkDescriptorSetAllocateInfo dsAllocInfo[1];
	dsAllocInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dsAllocInfo[0].pNext = NULL;
	dsAllocInfo[0].descriptorPool = descriptorPool;
	dsAllocInfo[0].descriptorSetCount = 1;
	dsAllocInfo[0].pSetLayouts = descLayout.data();

	// �ʿ��� ������ ��ũ���� ��Ʈ�� �Ҵ�
	descriptorSet.resize(1);

	// ��ũ���� ��Ʈ �Ҵ�
	result = vkAllocateDescriptorSets(deviceObj->device, dsAllocInfo, descriptorSet.data());
	assert(result == VK_SUCCESS);

	// 2���� ���� ��ũ���͸� �Ҵ� - 1. MVP, 2. Texture
	VkWriteDescriptorSet writes[2];
	memset(&writes, 0, sizeof(writes));

	// ��ũ���Ϳ� ó�� ���� ������ ���õ� ������ ���۸� ����
	writes[0] = {};
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].pNext = NULL;
	writes[0].dstSet = descriptorSet[0];
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].pBufferInfo = &uniformData.bufferInfo;
	writes[0].dstArrayElement = 0;
	writes[0].dstBinding = 0; // DESCRIPTOR_SET_BINDING_INDEX

	// �ؽ�ó�� ���Ǵ� ��� �� ��° ���� ��ũ���� ����ü ������Ʈ
	if (useTexture)
	{
		writes[1] = {};
		writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[1].dstSet = descriptorSet[0];
		writes[1].dstBinding = 1; // DESCRIPTOR_SET_BINDING_INDEX
		writes[1].descriptorCount = 1;
		writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writes[1].pImageInfo = NULL;
		writes[1].dstArrayElement = 0;
	}

	// ������ ���۸� �Ҵ�� ��ũ���� ��Ʈ�� ������Ʈ
	vkUpdateDescriptorSets(deviceObj->device, useTexture ? 2 : 1, writes, 0, NULL);
}

void VKDrawable::createDescriptorSetLayout(bool useTexture)
{
	// ��ũ���� ��Ʈ�� ���� ���̾ƿ� ���ε� ������ ����
	// ���ε� ����Ʈ, ���̴� ����(���ؽ� ���̴�), ���� ��
	VkDescriptorSetLayoutBinding layoutBindings[2];
	layoutBindings[0].binding = 0; // DESCRIPTOR_SET_BINDING_INDEX
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindings[0].pImmutableSamplers = NULL;

	// ���� �ؽ�ó�� ���Ǹ�, �� ��° ���ε��� �����׸�Ʈ ���̴��� �־�� ��
	if (useTexture)
	{
		layoutBindings[1].binding = 1; // DESCRIPTOR_SET_BINDING_INDEX
		layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings[1].descriptorCount = 1;
		layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings[1].pImmutableSamplers = NULL;
	}

	// ���̾ƿ� ���ε� ����, ��ũ���� ��Ʈ ���̾ƿ��� �����ϱ� ���� ���
	VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pNext = NULL;
	descriptorLayout.bindingCount = useTexture ? 2 : 1;
	descriptorLayout.pBindings = layoutBindings;

	VkResult  result;
	// ��ũ���� ���̾ƿ� ��ü���� �ʿ��� ����ŭ �Ҵ��ϰ� �̸� ����
	descLayout.resize(1);
	result = vkCreateDescriptorSetLayout(deviceObj->device, &descriptorLayout, NULL, descLayout.data());
	assert(result == VK_SUCCESS);

}

void VKDrawable::createPipelineLayout()
{
	// Ǫ�� ��� ������ ����
	const unsigned pushConstantRangeCount = 1;
	VkPushConstantRange pushConstantRanges[pushConstantRangeCount] = {};
	pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRanges[0].offset = 0;
	pushConstantRanges[0].size = 8;

	// ��ũ���� ���̾ƿ��� ����� ���������� ���̾ƿ��� �����Ѵ�.
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = NULL;
	pPipelineLayoutCreateInfo.pushConstantRangeCount = pushConstantRangeCount;
	pPipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges;
	pPipelineLayoutCreateInfo.setLayoutCount = (uint32_t)descLayout.size();
	pPipelineLayoutCreateInfo.pSetLayouts = descLayout.data();

	VkResult  result;
	result = vkCreatePipelineLayout(deviceObj->device, &pPipelineLayoutCreateInfo, NULL, &pipelineLayout);
	assert(result == VK_SUCCESS);
}

void VKDrawable::setViewports()
{
	viewport.height = (float)rendererObj->height;
	viewport.width = (float)rendererObj->width;
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	viewport.x = 0;
	viewport.y = 0;
	vkCmdSetViewport(cmdDraw, 0, NUMBER_OF_VIEWPORTS, &viewport);
}

void VKDrawable::setScissors()
{
	scissor.extent.width = rendererObj->width;
	scissor.extent.height = rendererObj->height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(cmdDraw, 0, NUMBER_OF_SCISSORS, &scissor);
}

void VKDrawable::destroyModel()
{
	VKDevice* deviceObj = rendererObj->getDevice();

	for (std::pair<int, ModelObj*> m : models) {
		//VertexBuffer
		vkDestroyBuffer(deviceObj->device, m.second->vBuffer.buf, NULL);
		vkFreeMemory(deviceObj->device, m.second->vBuffer.mem, NULL);

		//VertexIndex
		vkDestroyBuffer(deviceObj->device, m.second->vIndex.idx, NULL);
		vkFreeMemory(deviceObj->device, m.second->vIndex.mem, NULL);

		free(m.second);
	}

	models.clear();
}

void VKDrawable::destroyRenderObj()
{
	for (RenderObj* r : renderObjs) {
		free(r);
	}
}

void VKDrawable::destroyUniformBuffer()
{
	vkUnmapMemory(deviceObj->device, uniformData.memory);
	vkDestroyBuffer(rendererObj->getDevice()->device, uniformData.buffer, NULL);
	vkFreeMemory(rendererObj->getDevice()->device, uniformData.memory, NULL);
}

void VKDrawable::destroyCommandBuffer()
{
	VKApplication* appObj = VKApplication::GetInstance();
	VKDevice* deviceObj = appObj->deviceObj;

	vkFreeCommandBuffers(deviceObj->device, rendererObj->cmdPool, 1, &cmdDraw);
}
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

	// VkVertexInputBinding viIpBind는 버텍스 입력으로 들어오는 정보에서 정의된 속도로 저장
	viIpBind.binding = 0;
	viIpBind.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	viIpBind.stride = sizeof(squareData1[0]);

	// VkVertexInputAttribute(디스크립션) 구조체, 데이터 해석에 도움을 주는 정보를 저장
	// 위치, 색 특성과 관련된 정보.....
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
	result = vkCreateBuffer(deviceObj->device, &bufInfo, NULL, &model->vBuffer.buf);
	assert(result == VK_SUCCESS);

	// 버퍼 리소스 요구사항 가져오기
	VkMemoryRequirements memRqrmnt;
	vkGetBufferMemoryRequirements(deviceObj->device, model->vBuffer.buf, &memRqrmnt);

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
	result = vkAllocateMemory(deviceObj->device, &allocInfo, NULL, &(model->vBuffer.mem));
	assert(result == VK_SUCCESS);
	model->vBuffer.bufferInfo.range = memRqrmnt.size;
	model->vBuffer.bufferInfo.offset = 0;

	// 호스트에 물리적 장치 메모리 공간을 매핑
	uint8_t* pData;
	result = vkMapMemory(deviceObj->device, model->vBuffer.mem, 0, memRqrmnt.size, 0, (void**)&pData);
	assert(result == VK_SUCCESS);

	// 데이터를 매핑된 메모리에 복사
	memcpy(pData, vertexData, dataSize);

	// 장치 메모리를 매핑 해제
	vkUnmapMemory(deviceObj->device, model->vBuffer.mem);

	// 할당된 버퍼 리소스를 장치 메모리에 바인딩
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
	//사용할 커맨드 버퍼를 할당한다.
	VKDevice* deviceObj = rendererObj->getDevice();
	CommandBufferMgr::allocCommandBuffer(&deviceObj->device, *rendererObj->getCommandPool(), &cmdDraw);

	//렌더 오브젝트를 등록한다.
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
	// 1. 커맨드 버퍼를 작성하고
	CommandBufferMgr::beginCommandBuffer(cmdDraw);

	// 2. 렌더 패스를 시작한다.
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

	// 3. 동적 뷰포트를 설정한다
	setViewports();
	setScissors();

	// 4. 파이프라인을 바인딩 한다.
	vkCmdBindPipeline(cmdDraw, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);

	vkCmdBindDescriptorSets(cmdDraw, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descriptorSet.data(), 0, NULL);
	
	// 5. 드로잉 할 개체의 버텍스들을 바인딩 한다.
	const VkDeviceSize offsets[1] = { 0 };

	//모든 렌더 오브젝트 바인딩
	//pushCoonstants
	//vkCmdBineIndex
	//vkBindIndexBuffer
	for (RenderObj* obj : renderObjs) {
		ModelObj* model = models[obj->modelIndex];

		// 버텍스 버퍼 바인딩
		vkCmdBindVertexBuffers(cmdDraw, 0, 1, &model->vBuffer.buf, offsets);
		
		// 인덱스 버퍼 바인딩
		//vkCmdBindIndexBuffer(cmdDraw, model->vIndex.idx, 0, VK_INDEX_TYPE_UINT16);

		// 인덱스 버퍼 그려
		//vkCmdDrawIndexed(cmdDraw, 6, 1, 0, 0, 0);
	}

	//그려줘어~
	vkCmdDraw(cmdDraw, 3 * 2 * 6, 1 ,0 ,0);

	//렌더 패스를 종료한다.
	vkCmdEndRenderPass(cmdDraw);

	//커맨드 버퍼를 작성을 종료한다. 
	CommandBufferMgr::endCommandBuffer(cmdDraw);

	//커맨드 버퍼를 제출한다.
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
	//커맨드 버퍼 작성,,,
	CommandBufferMgr::beginCommandBuffer(rendererObj->cmdVertexBuffer);

	// 구조체 초기화
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
		glm::vec3(10, 3, 10),	// 카메라는 월드 좌표계에 있고
		glm::vec3(0, 0, 0),		// 원점을 보고 있음
		glm::vec3(0, -1, 0)		// 상향 벡터
	);
	Model = glm::mat4(1.0f);
	MVP = Projection * View * Model;

	// VkBufferCreateInfo로 버퍼 리소스 스테이트 생성
	VkBufferCreateInfo bufInfo = {};
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufInfo.pNext = NULL;
	bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufInfo.size = sizeof(MVP);
	bufInfo.queueFamilyIndexCount = 0;
	bufInfo.pQueueFamilyIndices = NULL;
	bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufInfo.flags = 0;

	// 버퍼 정보와 버퍼 개체를 생성하는 데 사용
	result = vkCreateBuffer(deviceObj->device, &bufInfo, NULL, &uniformData.buffer);
	assert(result == VK_SUCCESS);

	// 버퍼 메모리 요구사항 얻기
	VkMemoryRequirements memRqrmnt;
	vkGetBufferMemoryRequirements(deviceObj->device, uniformData.buffer, &memRqrmnt);

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext = NULL;
	memAllocInfo.memoryTypeIndex = 0;
	memAllocInfo.allocationSize = memRqrmnt.size;

	// 메모리 속성으로 필요한 메모리 유형 결정
	pass = deviceObj->memoryTypeFromProperties(memRqrmnt.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memAllocInfo.memoryTypeIndex);
	assert(pass);

	// 버퍼 개체의 메모리 할당
	result = vkAllocateMemory(deviceObj->device, &memAllocInfo, NULL, &(uniformData.memory));
	assert(result == VK_SUCCESS);

	// 매핑된 버퍼
	result = vkMapMemory(deviceObj->device, uniformData.memory, 0, memRqrmnt.size, 0, (void**)&uniformData.pData);
	assert(result == VK_SUCCESS);

	// 매핑된 버퍼에서 계산된 데이터 복사
	memcpy(uniformData.pData, &MVP, sizeof(MVP));

	// 업데이트할 유니폼 버퍼 개체는 1개
	uniformData.mappedRange.resize(1);

	// VkMappedMemoryRange 데이터 구조체 채우기
	uniformData.mappedRange[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	uniformData.mappedRange[0].memory = uniformData.memory;
	uniformData.mappedRange[0].offset = 0;
	uniformData.mappedRange[0].size = sizeof(MVP);

	// 호스트에서 볼 수 있게 매핑된 버퍼의 범위를 무효화
	vkInvalidateMappedMemoryRanges(deviceObj->device, 1, &uniformData.mappedRange[0]);

	//버퍼 장치 메모리를 바인딩 함
	result = vkBindBufferMemory(deviceObj->device, uniformData.buffer, uniformData.memory, 0);
	assert(result == VK_SUCCESS);

	// 관리를 위한 유니폼 버퍼로 로컬 데이터 구조체를 업데이트
	uniformData.bufferInfo.buffer = uniformData.buffer;
	uniformData.bufferInfo.offset = 0;
	uniformData.bufferInfo.range = sizeof(MVP);
	uniformData.memRqrmnt = memRqrmnt;
}

void VKDrawable::createDescriptorPool(bool useTexture)
{
	VkResult  result;
	// 사용할 디스크립터 세트 유형을 기반으로 디스크립터 풀의 크기 정의
	std::vector<VkDescriptorPoolSize> descriptorTypePool;

	// 첫 번째 디스크립터 풀 개체는 유니폼 버퍼 유형
	descriptorTypePool.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 });

	// 텍스처가 지원되면 두 번째 개체를 디스크립터 유형 이미지 샘플러로 정의
	if (useTexture) {
		descriptorTypePool.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 });
	}

	// 생성된 구조체에 디스크립터 풀 스테이트 정보 채워 넣기
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = NULL;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.poolSizeCount = (uint32_t)descriptorTypePool.size();
	descriptorPoolCreateInfo.pPoolSizes = descriptorTypePool.data();

	// 디스크립터 풀 생성 정보
	result = vkCreateDescriptorPool(deviceObj->device, &descriptorPoolCreateInfo, NULL, &descriptorPool);
	assert(result == VK_SUCCESS);
}

void VKDrawable::createDescriptorResources()
{
	// 유니폼 리소스를 내부에서 생성, 디스크립터 세트를 생성
	// 디스크립터 세트를 생성하기 전에 자원과 연결
	createUniformBuffer();
	
}

void VKDrawable::createDescriptorSet(bool useTexture)
{
	VKPipeline* pipelineObj = rendererObj->getPipelineObject();
	VkResult  result;

	// 디스크립터 할당 구조체를 생성하고 디스크립터 풀과 디스크립터 레이아웃을 지정
	VkDescriptorSetAllocateInfo dsAllocInfo[1];
	dsAllocInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dsAllocInfo[0].pNext = NULL;
	dsAllocInfo[0].descriptorPool = descriptorPool;
	dsAllocInfo[0].descriptorSetCount = 1;
	dsAllocInfo[0].pSetLayouts = descLayout.data();

	// 필요한 수량의 디스크립터 세트를 할당
	descriptorSet.resize(1);

	// 디스크립터 세트 할당
	result = vkAllocateDescriptorSets(deviceObj->device, dsAllocInfo, descriptorSet.data());
	assert(result == VK_SUCCESS);

	// 2개의 쓰기 디스크립터를 할당 - 1. MVP, 2. Texture
	VkWriteDescriptorSet writes[2];
	memset(&writes, 0, sizeof(writes));

	// 디스크립터에 처음 쓰일 정보와 관련된 유니폼 버퍼를 지정
	writes[0] = {};
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].pNext = NULL;
	writes[0].dstSet = descriptorSet[0];
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].pBufferInfo = &uniformData.bufferInfo;
	writes[0].dstArrayElement = 0;
	writes[0].dstBinding = 0; // DESCRIPTOR_SET_BINDING_INDEX

	// 텍스처가 사용되는 경우 두 번째 쓰기 디스크립터 구조체 업데이트
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

	// 유니폼 버퍼를 할당된 디스크립터 세트로 업데이트
	vkUpdateDescriptorSets(deviceObj->device, useTexture ? 2 : 1, writes, 0, NULL);
}

void VKDrawable::createDescriptorSetLayout(bool useTexture)
{
	// 디스크립터 세트를 위한 레이아웃 바인딩 정보를 정의
	// 바인딩 포인트, 셰이더 유형(버텍스 셰이더), 개수 등
	VkDescriptorSetLayoutBinding layoutBindings[2];
	layoutBindings[0].binding = 0; // DESCRIPTOR_SET_BINDING_INDEX
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindings[0].pImmutableSamplers = NULL;

	// 만일 텍스처가 사용되면, 두 번째 바인딩이 프래그먼트 셰이더에 있어야 함
	if (useTexture)
	{
		layoutBindings[1].binding = 1; // DESCRIPTOR_SET_BINDING_INDEX
		layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings[1].descriptorCount = 1;
		layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings[1].pImmutableSamplers = NULL;
	}

	// 레이아웃 바인딩 지정, 디스크립터 세트 레이아웃을 생성하기 위해 사용
	VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pNext = NULL;
	descriptorLayout.bindingCount = useTexture ? 2 : 1;
	descriptorLayout.pBindings = layoutBindings;

	VkResult  result;
	// 디스크립터 레이아웃 개체에서 필요한 수만큼 할당하고 이를 생성
	descLayout.resize(1);
	result = vkCreateDescriptorSetLayout(deviceObj->device, &descriptorLayout, NULL, descLayout.data());
	assert(result == VK_SUCCESS);

}

void VKDrawable::createPipelineLayout()
{
	// 푸시 상수 범위를 설정
	const unsigned pushConstantRangeCount = 1;
	VkPushConstantRange pushConstantRanges[pushConstantRangeCount] = {};
	pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRanges[0].offset = 0;
	pushConstantRanges[0].size = 8;

	// 디스크립터 레이아웃을 사용해 파이프라인 레이아웃을 생성한다.
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
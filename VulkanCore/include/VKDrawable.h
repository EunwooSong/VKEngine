#pragma once
#include "Headers.h"

class VKRenderer;

// 원하는 기하 형상을 그리는 데 사용된다. (지금은 간단한 삼각형)
class VKDrawable
{
public:
	VKDrawable(VKRenderer* parent = 0);
	~VKDrawable();

	void createVertexBuffer(const void* vertexData, uint32_t dataSize, uint32_t dataStride, bool useTexture);
	void prepare();
	void render();
	void destroyVertexBuffer();

public:
	// 버텍스 버퍼 메타 데이터를 저장할 구조체
	struct {
		VkBuffer buf;
		VkDeviceMemory mem;
		VkDescriptorBufferInfo bufferInfo;
	} VertexBuffer;

	// 버텍스 입력 비율 저장
	VkVertexInputBindingDescription		viIpBind;

	// 데이터 해석에 도움을 주는 메타 데이터 저장
	VkVertexInputAttributeDescription	viIpAttrb[2];

private:
	std::vector<VkCommandBuffer> vecCmdDraw;			// Command buffer for drawing
	void recordCommandBuffer(int currentBuffer, VkCommandBuffer* cmdDraw);
	VKRenderer* rendererObj;
};
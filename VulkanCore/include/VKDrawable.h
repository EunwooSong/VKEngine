#pragma once
#include "Headers.h"

class VKRenderer;

// ���ϴ� ���� ������ �׸��� �� ���ȴ�. (������ ������ �ﰢ��)
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
	// ���ؽ� ���� ��Ÿ �����͸� ������ ����ü
	struct {
		VkBuffer buf;
		VkDeviceMemory mem;
		VkDescriptorBufferInfo bufferInfo;
	} VertexBuffer;

	// ���ؽ� �Է� ���� ����
	VkVertexInputBindingDescription		viIpBind;

	// ������ �ؼ��� ������ �ִ� ��Ÿ ������ ����
	VkVertexInputAttributeDescription	viIpAttrb[2];

private:
	std::vector<VkCommandBuffer> vecCmdDraw;			// Command buffer for drawing
	void recordCommandBuffer(int currentBuffer, VkCommandBuffer* cmdDraw);
	VKRenderer* rendererObj;
};
#pragma once
#include "Headers.h"

class VKRenderer;

// ���ϴ� ���� ������ �׸��� �� ���ȴ�. (������ ������ �ﰢ��)
class VKDrawable
{
public:
	VKDrawable(VKRenderer* parent = 0);
	virtual ~VKDrawable();

	void createVertexBuffer(const void* vertexData, uint32_t dataSize, uint32_t dataStride, bool useTexture);
	virtual void prepare() = 0;
	virtual void render() = 0;
	void setPipeline(VkPipeline* vulkanPipeline) { pipeline = vulkanPipeline; }
	VkPipeline* getPipeline() { return pipeline; }
	void destroyVertexBuffer();
	void destroyCommandBuffer();
	void destroySynchronizationObjects();

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

protected:
	std::vector<VkCommandBuffer> vecCmdDraw;			// Command buffer for drawing
	virtual void recordCommandBuffer(int currentBuffer, VkCommandBuffer* cmdDraw) = 0;

	VkSemaphore presentCompleteSemaphore;
	VkSemaphore drawingCompleteSemaphore;

	VKRenderer* rendererObj;
	VkPipeline* pipeline;


};


// �� ���, ���� �ʱ�ȭ
class VKBackground : public VKDrawable {
public:
	VKBackground(VKRenderer* parent);
	~VKBackground();

	virtual void prepare();
	virtual void render();

private:
	virtual void recordCommandBuffer(int currentBuffer, VkCommandBuffer* cmdDraw);
};

// ������ �ﰢ��
class VKTriangle : public VKDrawable {
public:
	VKTriangle(VKRenderer* parent);
	~VKTriangle();

	virtual void prepare();
	virtual void render();

	// ����Ʈ �Ķ���� �ʱ�ȭ
	void initViewports(VkCommandBuffer* cmd);

	// ������ �Ķ���� �ʱ�ȭ
	void initScissors(VkCommandBuffer* cmd);

private :
	virtual void recordCommandBuffer(int currentBuffer, VkCommandBuffer* cmdDraw);
	VkViewport viewport;
	VkRect2D   scissor;
};
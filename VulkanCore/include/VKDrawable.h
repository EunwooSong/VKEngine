#pragma once
#include "Headers.h"

class VKRenderer;

// 원하는 기하 형상을 그리는 데 사용된다. (지금은 간단한 삼각형)
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

protected:
	std::vector<VkCommandBuffer> vecCmdDraw;			// Command buffer for drawing
	virtual void recordCommandBuffer(int currentBuffer, VkCommandBuffer* cmdDraw) = 0;

	VkSemaphore presentCompleteSemaphore;
	VkSemaphore drawingCompleteSemaphore;

	VKRenderer* rendererObj;
	VkPipeline* pipeline;


};


// 뒷 배경, 버퍼 초기화
class VKBackground : public VKDrawable {
public:
	VKBackground(VKRenderer* parent);
	~VKBackground();

	virtual void prepare();
	virtual void render();

private:
	virtual void recordCommandBuffer(int currentBuffer, VkCommandBuffer* cmdDraw);
};

// 간단한 삼각형
class VKTriangle : public VKDrawable {
public:
	VKTriangle(VKRenderer* parent);
	~VKTriangle();

	virtual void prepare();
	virtual void render();

	// 뷰포트 파라미터 초기화
	void initViewports(VkCommandBuffer* cmd);

	// 시저링 파라미터 초기화
	void initScissors(VkCommandBuffer* cmd);

private :
	virtual void recordCommandBuffer(int currentBuffer, VkCommandBuffer* cmdDraw);
	VkViewport viewport;
	VkRect2D   scissor;
};
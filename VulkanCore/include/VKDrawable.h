#pragma once
#include "Headers.h"
#include "VKDescriptor.h"

class VKRenderer;

// ���ؽ� ���� ��Ÿ �����͸� ������ ����ü
struct VertexBuffer {
	VkBuffer buf;
	VkDeviceMemory mem;
	VkDescriptorBufferInfo bufferInfo;
};

struct VertexIndex {
	VkBuffer idx;
	VkDeviceMemory mem;
	VkDescriptorBufferInfo bufferInfo;
};

// ������ ���� ������ ���� ���� ������ ����ü
struct UniformData {
	VkBuffer						buffer;			// ���� ���ҽ� ��ü
	VkDeviceMemory					memory;			// ���� ���ҽ� ��ü�� �Ҵ�� ��ġ �޸�
	VkDescriptorBufferInfo			bufferInfo;		// ��ũ���� ��Ʈ�� �ۼ��ؼ� �����ؾ��ϴ� ���� ����
	VkMemoryRequirements			memRqrmnt;		// ������ ������ �䱸������ ������ �޸𸮿� ����
	std::vector<VkMappedMemoryRange>mappedRange;	// �޸��� ��Ÿ �����͸� ��ü�� ����
	uint8_t* pData;									// �����͸� ���� �� ����� ���ε� ��ġ �ּҰ� ���Ե� ȣ��Ʈ ������
};

// ������ ������Ʈ ��ü
struct ModelObj {
	VertexBuffer vBuffer;
	VertexIndex vIndex;
};

struct RenderObj {
	// ��ġ

	
	// �� �ε���
	int modelIndex;
};

// ����� ��ü
class VKDrawable : public VKDescriptor
{
public:
	VKDrawable(VKRenderer* parent = 0);
	virtual ~VKDrawable();

	void createVertexBuffer(const void* vertexData, ModelObj* model, uint32_t dataSize, uint32_t dataStride, bool useTexture);
	void createVertexIndex(const void* indexData, ModelObj* model, uint32_t dataSize, uint32_t dataStride);
	virtual void prepare();
	virtual void update();
	virtual void render(VkSemaphore* presentCompleteSemaphore, VkSemaphore* drawingCompleteSemaphore);

	void setPipeline(VkPipeline* vulkanPipeline) { pipeline = vulkanPipeline; }
	VkPipeline* getPipeline() { return pipeline; }

	// ������ �� �ҷ�����(������ 0�� ���� �ﰢ��, 1�� ū �簢��..!)
	// ���߿� VKResourceLoader�� �����ϰ� ���⼭ ���� ������... �� �ʿ� ����!
	int loadModel(int type);
	RenderObj* createRenderObject(int modelIndex);

	void createUniformBuffer();
	virtual void createDescriptorPool(bool useTexture);
	virtual void createDescriptorResources();
	virtual void createDescriptorSet(bool useTexture);
	virtual void createDescriptorSetLayout(bool useTexture);
	virtual void createPipelineLayout();

	// ����Ʈ
	void setViewports();
	void setScissors();

	void destroyModel();
	void destroyRenderObj();
	void destroyUniformBuffer();
	void destroyCommandBuffer();

public:
	std::map<int, ModelObj*> models;
	std::vector<RenderObj*> renderObjs;

	// ���ؽ� �Է� ���� ����
	VkVertexInputBindingDescription		viIpBind;

	// ������ �ؼ��� ������ �ִ� ��Ÿ ������ ����
	VkVertexInputAttributeDescription	viIpAttrb[2];

protected:
	std::vector<VertexBuffer*> vertexBuffer;

	// For Test
	UniformData uniformData;
	glm::mat4 Projection;
	glm::mat4 View;
	glm::mat4 Model;
	glm::mat4 MVP;

	VkCommandBuffer cmdDraw;			// Command buffer for drawing
	
	VkViewport viewport;
	VkRect2D   scissor;

	VKRenderer* rendererObj;
	VkPipeline* pipeline;
};
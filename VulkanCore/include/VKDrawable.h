#pragma once
#include "Headers.h"
#include "VKDescriptor.h"

class VKRenderer;

// 버텍스 버퍼 메타 데이터를 저장할 구조체
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

// 유니폼 버퍼 관리를 위한 로컬 데이터 구조체
struct UniformData {
	VkBuffer						buffer;			// 버퍼 리소스 개체
	VkDeviceMemory					memory;			// 버퍼 리소스 개체에 할당된 장치 메모리
	VkDescriptorBufferInfo			bufferInfo;		// 디스크립터 세트에 작성해서 공급해야하는 버퍼 정보
	VkMemoryRequirements			memRqrmnt;		// 유니폼 버퍼의 요구사항을 쿼리된 메모리에 저장
	std::vector<VkMappedMemoryRange>mappedRange;	// 메모리의 메타 데이터를 개체에 매핑
	uint8_t* pData;									// 데이터를 쓰는 데 사용한 매핑된 장치 주소가 포함된 호스트 포인터
};

// 렌더링 오브젝트 개체
struct ModelObj {
	VertexBuffer vBuffer;
	VertexIndex vIndex;
};

struct RenderObj {
	// 위치

	
	// 모델 인덱스
	int modelIndex;
};

// 드로잉 개체
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

	// 렌더링 모델 불러오기(지금은 0은 작은 삼각형, 1은 큰 사각형..!)
	// 나중에 VKResourceLoader를 생성하고 여기서 받은 데이터... 는 필요 없다!
	int loadModel(int type);
	RenderObj* createRenderObject(int modelIndex);

	void createUniformBuffer();
	virtual void createDescriptorPool(bool useTexture);
	virtual void createDescriptorResources();
	virtual void createDescriptorSet(bool useTexture);
	virtual void createDescriptorSetLayout(bool useTexture);
	virtual void createPipelineLayout();

	// 뷰포트
	void setViewports();
	void setScissors();

	void destroyModel();
	void destroyRenderObj();
	void destroyUniformBuffer();
	void destroyCommandBuffer();

public:
	std::map<int, ModelObj*> models;
	std::vector<RenderObj*> renderObjs;

	// 버텍스 입력 비율 저장
	VkVertexInputBindingDescription		viIpBind;

	// 데이터 해석에 도움을 주는 메타 데이터 저장
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
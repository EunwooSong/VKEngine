#pragma once
#include "Headers.h"
#include "VKSwapChain.h"
#include "VKDrawable.h"
#include "VKShader.h"
#include "VKPipeline.h"

// 샘플러의 수는 이미지 생성시 동일한 것이 필요하다.
// 렌더 패스와 파이프라인 생성시 사용된다.
#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

class VKRenderer
{
public:
	VKRenderer(VKApplication* app, VKDevice* deviceObject);
	~VKRenderer();

public:
	//Simple life cycle
	void initialize();
	void prepare();
	void update();
	bool render();

	// 실제 렌더 처리 진행 (스와프 이미지 관리)
	void process();

	// 빈 윈도 생성 (나중에 인자 값의 조정이 필요..)
	void createPresentationWindow(const int& windowWidth = 500, const int& windowHeight = 500);
	void setImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkAccessFlagBits srcAccessMask, const VkCommandBuffer& cmdBuf);

	// 윈도 시스템이 이벤트를 핸들링하기 위한 프로시져 (__stdcall : 프로그램 외부에서 코드 호출시...)
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// 윈도 제거
	void destroyPresentationWindow();

	// Getter functions for member variable specific to classes.
	inline VKApplication* getApplication() { return application; }
	inline VKDevice* getDevice() { return deviceObj; }
	inline VKSwapChain* getSwapChain() { return swapChainObj; }
	inline std::vector<VKDrawable*>* getDrawingItems() { return &drawableList; }
	inline VkCommandPool* getCommandPool() { return &cmdPool; }
	inline VKPipeline* getPipelineObject() { return &pipelineObj; }

	void createCommandPool();							// 커맨드 풀 생성
	void buildSwapChainAndDepthImage();					// 스와프 체인 색상 이미지 생성
	void createDepthImage();							// 깊이 이미지 생성
	void createVertexBuffer();
	void createRenderPass(bool includeDepth, bool clear = true); // 렌더 패스 생성
	void createFrameBuffer(bool includeDepth);
	void createShaders();
	void createDescriptors();							// 디스크립터 세트 레이아웃 생성
	void createPipelineStateManagement();
	void createPushConstants();			// 푸시 상수 리소스 업데이트

	void destroyCommandBuffer();
	void destroyCommandPool();
	void destroySynchronizationObjects();
	void destroyDepthBuffer();
	void destroyRenderpass();
	void destroyFrameBuffers();
	void destroyPipeline();
	void destoryDrawableObjects();
	void destroyShader();

public:
#ifdef _WIN32
#define APP_NAME_STR_LEN 80
	HINSTANCE					connection;				// hInstance -  Windows 인스턴스
	char						name[APP_NAME_STR_LEN]; // name - 윈도에 나타날 응용 프로그램 이름
	HWND						window;					// hWnd - 윈도 핸들
#else
	xcb_connection_t* connection;
	xcb_screen_t* screen;
	xcb_window_t				window;
	xcb_intern_atom_reply_t* reply;
#endif

	// 깊이 이미지에 사용할 데이터 구조체
	struct {
		VkFormat		format;	// 이름이 지어진 깊이 이미지 포맷을 참조
		VkImage			image;	// 깊이 이미지 개체인 VkImage
		VkDeviceMemory	mem;	// 깊이 이미지 개체와 관련해 할당된 메모리
		VkImageView		view;	// 깊이 이미지 개체의 깊이 이미지 뷰 개체
	}Depth;

	VkCommandBuffer		cmdDepthImage;	// 깊이 이미지 커맨드 버퍼
	VkCommandPool		cmdPool;		// 커맨드 풀
	VkCommandBuffer		cmdVertexBuffer;// 버텍스 버퍼 - Triangle geometry
	VkCommandBuffer		cmdPushConstant;// 푸시 상수 버퍼
	VkCommandBuffer		cmdUniformBuffer;

	int					width, height;	// 윈도의 폭과 높이

	VkRenderPass renderPass;
	std::vector<VkFramebuffer> framebuffers;	// 각 스와프 체인에 해당하는 프레임 버퍼 수
	std::vector<VkPipeline*> pipelineList;		// 각각의 드로잉 개체에 대한 파이프라인
	bool ready;

private:
	//클래스 관리
	VKApplication* application;

	// 이 프레젠테이션 레이어와 연결될 장치 개체.
	VKDevice* deviceObj;
	VKSwapChain* swapChainObj;

	std::vector<VKDrawable*> drawableList;
	VKShader 	   shaderObj;
	VKPipeline 	   pipelineObj;

	VkSemaphore presentCompleteSemaphore;
	VkSemaphore drawingCompleteSemaphore;
};
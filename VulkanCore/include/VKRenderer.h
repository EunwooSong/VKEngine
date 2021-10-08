#pragma once
#include "Headers.h"
#include "VKSwapChain.h"
#include "VKDrawable.h"
#include "VKShader.h"
#include "VKPipeline.h"

// ���÷��� ���� �̹��� ������ ������ ���� �ʿ��ϴ�.
// ���� �н��� ���������� ������ ���ȴ�.
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

	// ���� ���� ó�� ���� (������ �̹��� ����)
	void process();

	// �� ���� ���� (���߿� ���� ���� ������ �ʿ�..)
	void createPresentationWindow(const int& windowWidth = 500, const int& windowHeight = 500);
	void setImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkAccessFlagBits srcAccessMask, const VkCommandBuffer& cmdBuf);

	// ���� �ý����� �̺�Ʈ�� �ڵ鸵�ϱ� ���� ���ν��� (__stdcall : ���α׷� �ܺο��� �ڵ� ȣ���...)
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// ���� ����
	void destroyPresentationWindow();

	// Getter functions for member variable specific to classes.
	inline VKApplication* getApplication() { return application; }
	inline VKDevice* getDevice() { return deviceObj; }
	inline VKSwapChain* getSwapChain() { return swapChainObj; }
	inline std::vector<VKDrawable*>* getDrawingItems() { return &drawableList; }
	inline VkCommandPool* getCommandPool() { return &cmdPool; }
	inline VKPipeline* getPipelineObject() { return &pipelineObj; }

	void createCommandPool();							// Ŀ�ǵ� Ǯ ����
	void buildSwapChainAndDepthImage();					// ������ ü�� ���� �̹��� ����
	void createDepthImage();							// ���� �̹��� ����
	void createVertexBuffer();
	void createRenderPass(bool includeDepth, bool clear = true); // ���� �н� ����
	void createFrameBuffer(bool includeDepth);
	void createShaders();
	void createDescriptors();							// ��ũ���� ��Ʈ ���̾ƿ� ����
	void createPipelineStateManagement();
	void createPushConstants();			// Ǫ�� ��� ���ҽ� ������Ʈ

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
	HINSTANCE					connection;				// hInstance -  Windows �ν��Ͻ�
	char						name[APP_NAME_STR_LEN]; // name - ������ ��Ÿ�� ���� ���α׷� �̸�
	HWND						window;					// hWnd - ���� �ڵ�
#else
	xcb_connection_t* connection;
	xcb_screen_t* screen;
	xcb_window_t				window;
	xcb_intern_atom_reply_t* reply;
#endif

	// ���� �̹����� ����� ������ ����ü
	struct {
		VkFormat		format;	// �̸��� ������ ���� �̹��� ������ ����
		VkImage			image;	// ���� �̹��� ��ü�� VkImage
		VkDeviceMemory	mem;	// ���� �̹��� ��ü�� ������ �Ҵ�� �޸�
		VkImageView		view;	// ���� �̹��� ��ü�� ���� �̹��� �� ��ü
	}Depth;

	VkCommandBuffer		cmdDepthImage;	// ���� �̹��� Ŀ�ǵ� ����
	VkCommandPool		cmdPool;		// Ŀ�ǵ� Ǯ
	VkCommandBuffer		cmdVertexBuffer;// ���ؽ� ���� - Triangle geometry
	VkCommandBuffer		cmdPushConstant;// Ǫ�� ��� ����
	VkCommandBuffer		cmdUniformBuffer;

	int					width, height;	// ������ ���� ����

	VkRenderPass renderPass;
	std::vector<VkFramebuffer> framebuffers;	// �� ������ ü�ο� �ش��ϴ� ������ ���� ��
	std::vector<VkPipeline*> pipelineList;		// ������ ����� ��ü�� ���� ����������
	bool ready;

private:
	//Ŭ���� ����
	VKApplication* application;

	// �� ���������̼� ���̾�� ����� ��ġ ��ü.
	VKDevice* deviceObj;
	VKSwapChain* swapChainObj;

	std::vector<VKDrawable*> drawableList;
	VKShader 	   shaderObj;
	VKPipeline 	   pipelineObj;

	VkSemaphore presentCompleteSemaphore;
	VkSemaphore drawingCompleteSemaphore;
};
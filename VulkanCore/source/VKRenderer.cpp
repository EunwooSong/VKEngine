#include "VKRenderer.h"
#include "VKApplication.h"
#include "Wrappers.h"
#include "MeshData.h"
#include "VKPipeline.h"

VKRenderer::VKRenderer(VKApplication* app, VKDevice* deviceObject)
{
	assert(application != NULL);
	assert(deviceObj != NULL);

	// Note: It's very important to initilize the member with 0 or respective value other wise it will break the system
	memset(&Depth, 0, sizeof(Depth));
	memset(&connection, 0, sizeof(HINSTANCE));				// hInstance - Windows Instance

	application = app;
	deviceObj = deviceObject;

	swapChainObj = new VKSwapChain(this);

	VKDrawable* drawableObj = new VKBackground(this);
	drawableList.push_back(drawableObj);

	drawableObj = new VKTriangle(this);
	drawableList.push_back(drawableObj);

	ready = false;
}

VKRenderer::~VKRenderer()
{
	delete swapChainObj;
	swapChainObj = NULL;
}

//렌더러 초기화
// - 초기화에서는 입력으로 주어진 차원으로 프레젠테이션 윈도를 만들고
// 스와프 체인 구현을 위한 다양한 요구사항을 충족시킨다.
// 스와프 체인 확장판 API를 쿼리하고, 화면 개체를 만들고,
// 프레젠테이션 레이어를 가장 잘 지원하는 큐를 찾고,
// 드로잉을 위해 호환되는 이미지 포맷을 가져오는 등의 작업이 포함된다.
void VKRenderer::initialize()
{
	// 빈 윈도우 생성
	createPresentationWindow(1280, 720);

	// 스와프 체인 초기화
	swapChainObj->intializeSwapChain();

	// 커맨드 버퍼가 필요하므로 먼저 커맨드 버퍼 풀 생성
	createCommandPool();

	// 스와프 체인 색상 이미지 생성
	buildSwapChainAndDepthImage();

	// Build the vertex buffer 	
	createVertexBuffer();

	const bool includeDepth = true;
	// Create the render pass now..
	createRenderPass(includeDepth);

	// Use render pass and create frame buffer
	createFrameBuffer(includeDepth);

	// Create the vertex and fragment shader
	createShaders();

	// Manage the pipeline state objects
	createPipelineStateManagement();
	ready = true;
}

void VKRenderer::prepare() {
	for each (VKDrawable * drawableObj in drawableList)
	{
		drawableObj->prepare();
	}
}

// 5. 프레젠테이션 윈도 렌더링
// - 디스플레이에 프레젠테이션 윈도를 그리고 Windows 메시지를 처리한다.
// - 사용자가 닫기 버튼을 누르면, 프레젠테이션 윈도를 종료하고 렌더링 루프를 해제한다.
bool VKRenderer::render()
{
	MSG msg;   // 메시지
	PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
	if (msg.message == WM_QUIT) { // WM_QUIT 메시지가 오면 렌더링 루프를 종료함
		return false;
	}
	TranslateMessage(&msg);
	DispatchMessage(&msg);
	RedrawWindow(window, NULL, NULL, RDW_INTERNALPAINT);
	return true;
}

//1. 프레젠테이션 윈도 생성
// 디스플레이 윈도에 스와프 체인 색상 이미지를 표시하는 데 사용할 윈도 시스템 구현
// Windows, CreateWindowEX() API를 사용해 확장된 윈도 스타일로 겹쳐진 팝업 또는 자식 윈도우 생성
void VKRenderer::createPresentationWindow(const int& windowWidth, const int& windowHeight)
{
#ifdef _WIN32
	width = windowWidth;
	height = windowHeight;
	assert(width > 0 || height > 0);

	WNDCLASSEX  winInfo;

	sprintf(name, "Swapchain presentation window");
	memset(&winInfo, 0, sizeof(WNDCLASSEX));
	// 윈도 클래스 구조체 초기화
	winInfo.cbSize = sizeof(WNDCLASSEX);
	winInfo.style = CS_HREDRAW | CS_VREDRAW;
	winInfo.lpfnWndProc = WndProc;
	winInfo.cbClsExtra = 0;
	winInfo.cbWndExtra = 0;
	winInfo.hInstance = connection; // Windows 인스턴스
	winInfo.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winInfo.hCursor = LoadCursor(NULL, IDC_ARROW);
	winInfo.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	winInfo.lpszMenuName = NULL;
	winInfo.lpszClassName = name;
	winInfo.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	// Register window class:
	if (!RegisterClassEx(&winInfo)) {
		// It didn't work, so try to give a useful error:
		printf("Unexpected error trying to start the application!\n");
		fflush(stdout);
		exit(1);
	}

	// Create window with the registered class:
	RECT wr = { 0, 0, width, height };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	window = CreateWindowEx(0,
		name,					// class name
		name,					// app name
		WS_OVERLAPPEDWINDOW |	// window style
		WS_VISIBLE |
		WS_SYSMENU,
		100, 100,				// x/y coords
		wr.right - wr.left,     // width
		wr.bottom - wr.top,     // height
		NULL,					// handle to parent
		NULL,					// handle to menu
		connection,				// hInstance
		NULL);					// no extra parameters

	if (!window) {
		// It didn't work, so try to give a useful error:
		printf("Cannot create a window in which to draw!\n");
		fflush(stdout);
		exit(1);
	}

	SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)&application);
#else
	const xcb_setup_t* setup;
	xcb_screen_iterator_t iter;
	int scr;

	connection = xcb_connect(NULL, &scr);
	if (connection == NULL) {
		std::cout << "Cannot find a compatible Vulkan ICD.\n";
		exit(-1);
	}

	setup = xcb_get_setup(connection);
	iter = xcb_setup_roots_iterator(setup);
	while (scr-- > 0)
		xcb_screen_next(&iter);

	screen = iter.data;
#endif // _WIN32
}


// 기존 이미지 레이아웃 형식을 지정된 새 레이아웃 유형으로 변형한다.
// 이미지 개체가 처음 생성되고 미리 정의된 레이아웃이 적용되지 않아서 이전 이미지 레이아웃은 VK_IMAGE_LAYOUT_UNDEFINED로 지정된다. 
// 깊이/스텐실 테스트를 위한 이미지 레이아웃을 구현하므로 새로운 이미지 레이아웃은
// VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL 용도 유형으로 지정해야 한다.
void VKRenderer::setImageLayout(
	VkImage image,
	VkImageAspectFlags aspectMask,
	VkImageLayout oldImageLayout,
	VkImageLayout newImageLayout,
	VkAccessFlagBits srcAccessMask,
	const VkCommandBuffer& cmdBuf)
{
	// Command Buffer 종속성...
	assert(cmdBuf != VK_NULL_HANDLE);

	// deviceObj->queue를 초개화해야 함
	assert(deviceObj->queue != VK_NULL_HANDLE);

	VkImageMemoryBarrier imgMemoryBarrier = {};
	imgMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgMemoryBarrier.pNext = NULL;
	imgMemoryBarrier.srcAccessMask = srcAccessMask;
	imgMemoryBarrier.dstAccessMask = 0;
	imgMemoryBarrier.oldLayout = oldImageLayout;
	imgMemoryBarrier.newLayout = newImageLayout;
	imgMemoryBarrier.image = image;
	imgMemoryBarrier.subresourceRange.aspectMask = aspectMask;
	imgMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imgMemoryBarrier.subresourceRange.levelCount = 1;
	imgMemoryBarrier.subresourceRange.layerCount = 1;

	if (oldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		imgMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	switch (newImageLayout)
	{
		// 이 이미지로부터의 모든 복사가 완료된 것을 보장한다.
		// 이 레이아웃의 이미지는 명령의 대상(destination)으로만 사용할 수 있다.
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

		// 이 이미지로의 복사 또는 CPU 쓰기 작업이 완료된 것을 보장한다.
		// 이 레이아웃의 이미지는 읽기 전용 셰이더 리소스로만 사용할 수 있다.
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;

		// 이 레이아웃의 이미지는 프레임 버퍼의 색상 첨부로만 사용할 수 있다.
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		break;

		// 이 레이아웃의 이미지는 프레임 버퍼의 깊이/스텐실 첨부에만 사용할 수 있다.
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	}

	VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags destStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(cmdBuf, srcStages, destStages, 0, 0, NULL, 0, NULL, 1, &imgMemoryBarrier);
}

#ifdef _WIN32
// WM_PAINT : 윈도 렌더링, WM_SIZE : ??? , WM_CLOSE : 닫기
LRESULT VKRenderer::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	VKApplication* appObj = VKApplication::GetInstance();
	VkPresentInfoKHR present = {};
	VkResult result;

	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_PAINT :
		if (!appObj->rendererObj->ready) break;

		for (VKDrawable * drawableObj : appObj->rendererObj->drawableList)
		{
			drawableObj->render();

			// 윈도에 이미지 표시
			present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			present.swapchainCount = 1;
			present.pSwapchains = &appObj->rendererObj->swapChainObj->scPublicVars.swapChain;
			present.pImageIndices = &appObj->rendererObj->swapChainObj->scPublicVars.currentColorBuffer;

			// 표시를 위해 이미지를 큐에 제출
			result = appObj->rendererObj->swapChainObj->fpQueuePresentKHR(appObj->deviceObj->queue, &present);
			assert(result == VK_SUCCESS);
		}
		break;

	default:
		break;
	}
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}
#endif

void VKRenderer::destroyPresentationWindow()
{
#ifdef _WIN32
	DestroyWindow(window);
#endif
}

// 3. 커맨드 풀 생성
// - VKRenderer 클래스의 Command Pool은 커맨드 버퍼 할당과 깊이 이미지, 파이프라인 스테이트 설정, 드로잉 프리미티브 등의 다양한 작업에서 사용한다.
// - VKCommandPoolCreateInfo에는 커맨드 버퍼가 할당돼야 하는 그래픽스 큐의 인덱스가 들어 있다.
void VKRenderer::createCommandPool()
{
	VKDevice* deviceObj = application->deviceObj;
	/* Depends on intializeSwapChainExtension() */
	VkResult  res;

	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.pNext = NULL;
	cmdPoolInfo.queueFamilyIndex = deviceObj->graphicsQueueWithPresentIndex;	// 그래픽스 큐 인덱스
	cmdPoolInfo.flags = 0;

	res = vkCreateCommandPool(deviceObj->device, &cmdPoolInfo, NULL, &cmdPool); // 커맨드 풀 생성
	assert(res == VK_SUCCESS);
}

// 4. 스와프 체인과 깊이 이미지 생성
// - 스와프 체인과 깊이 이미지를 생성하기 위한 시작점.
void VKRenderer::buildSwapChainAndDepthImage()
{
	// 커맨드를 제출할 적절한 큐 가져오기
	deviceObj->getDeviceQueue();

	// 스와프 체인을 생성하고 색상 이미지 가져오기
	swapChainObj->createSwapChain(cmdDepthImage);

	// 깊이 이미지 생성
	createDepthImage();
}

// 깊이 버퍼 이미지 생성
void VKRenderer::createDepthImage()
{
	VkResult  result;
	bool  pass;

	VkImageCreateInfo imageInfo = {};

	// 깊이 포맷이 정의되지 않은 경우 16bit 값을 사용
	if (Depth.format == VK_FORMAT_UNDEFINED) {
		Depth.format = VK_FORMAT_D16_UNORM;
	}

	const VkFormat depthFormat = Depth.format;

	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(*deviceObj->gpu, depthFormat, &props);
	if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	}
	else if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
	}
	else {
		std::cout << "Unsupported Depth Format, try other Depth formats.\n";
		exit(-1);
	}

	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = NULL;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = depthFormat;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = NUM_SAMPLES;
	imageInfo.queueFamilyIndexCount = 0;
	imageInfo.pQueueFamilyIndices = NULL;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.flags = 0;

	// 사용자가 이미지 정보와 이미지 개체를 생성
	result = vkCreateImage(deviceObj->device, &imageInfo, NULL, &Depth.image);
	assert(result == VK_SUCCESS);

	// 이미지 메모리 요구사항 가져오기(깊이 이미지 개체의 물리적 메모리 저장을 할당하는 데 필요한 총 크기를 가져옴)
	VkMemoryRequirements memRqrmnt;
	vkGetImageMemoryRequirements(deviceObj->device, Depth.image, &memRqrmnt);

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.pNext = NULL;
	memAlloc.allocationSize = 0;
	memAlloc.memoryTypeIndex = 0;
	memAlloc.allocationSize = memRqrmnt.size;

	// 메모리 속성에 따라 메모리 유형을 결정
	pass = deviceObj->memoryTypeFromProperties(memRqrmnt.memoryTypeBits, 0, /* No requirements */ &memAlloc.memoryTypeIndex);
	assert(pass);

	// 물리적 메모리를 깊이 이미지로 할당
	result = vkAllocateMemory(deviceObj->device, &memAlloc, NULL, &Depth.mem);
	assert(result == VK_SUCCESS);

	// 깊이 이미지로 할당된 메모리를 바인딩
	result = vkBindImageMemory(deviceObj->device, Depth.image, Depth.mem, 0);
	assert(result == VK_SUCCESS);


	// 이미지 뷰 설정...
	VkImageViewCreateInfo imgViewInfo = {};
	imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imgViewInfo.pNext = NULL;
	imgViewInfo.image = VK_NULL_HANDLE;
	imgViewInfo.format = depthFormat;
	imgViewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
	imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imgViewInfo.subresourceRange.baseMipLevel = 0;
	imgViewInfo.subresourceRange.levelCount = 1;
	imgViewInfo.subresourceRange.baseArrayLayer = 0;
	imgViewInfo.subresourceRange.layerCount = 1;
	imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imgViewInfo.flags = 0;

	if (depthFormat == VK_FORMAT_D16_UNORM_S8_UINT ||
		depthFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
		depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT) {
		imgViewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	// 커맨드 버퍼를 이용해 깊이 이미지 생성
	// 여기에는 커맨드 버퍼 할당, 레코딩의 시작/종료 위치와 제출이 포함됨
	CommandBufferMgr::allocCommandBuffer(&deviceObj->device, cmdPool, &cmdDepthImage);
	CommandBufferMgr::beginCommandBuffer(cmdDepthImage);
	{
		// 깊이 스텐실 이미지 레이아웃을 optimal로 설정
		// 이 함수는 메모리 장벽을 레코딩하는 도우미 함수
		setImageLayout(Depth.image,
			imgViewInfo.subresourceRange.aspectMask,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, (VkAccessFlagBits)0, cmdDepthImage);
	}
	CommandBufferMgr::endCommandBuffer(cmdDepthImage);
	CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &cmdDepthImage);
	// 이 명령은 cmdDepthImage 커맨드 버퍼에 레코딩하며, 종속된 리소스가 리소스에 액세스하기 전에 적절한 이미지 레이아웃 요구사항을 충족하게 된다.
	
	// 이미지 뷰를 생성하고 응용 프로그램이 사용할 수 있도록 설정
	imgViewInfo.image = Depth.image;
	result = vkCreateImageView(deviceObj->device, &imgViewInfo, NULL, &Depth.view);
	assert(result == VK_SUCCESS);
}

void VKRenderer::createVertexBuffer()
{
	CommandBufferMgr::allocCommandBuffer(&deviceObj->device, cmdPool, &cmdVertexBuffer);
	CommandBufferMgr::beginCommandBuffer(cmdVertexBuffer);

	for each (VKDrawable * drawableObj in drawableList)
	{
		drawableObj->createVertexBuffer(triangleData, sizeof(triangleData), sizeof(triangleData[0]), false);
	}
	CommandBufferMgr::endCommandBuffer(cmdVertexBuffer);
	CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &cmdVertexBuffer);
}

void VKRenderer::createRenderPass(bool includeDepth, bool clear) {
	// 색상 이미지를 얻기 위한 createSwapChain()
	// 깊이 이미지를 얻기 위한 createDepthImage()
	// 이 둘에 대한 종속성이 있음
	
	VkResult  result;

	// 색상 버퍼와 깊이 버퍼를 첨부 형식으로 렌더 패스 인스턴스에 첨부함.
	VkAttachmentDescription attachments[2];
	attachments[0].format = swapChainObj->scPublicVars.format;
	attachments[0].samples = NUM_SAMPLES;
	attachments[0].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachments[0].flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;

	// 깊이 버퍼 첨부에 대해 첨부 속성이 깊이 정보를 제공하는지 점검
	if (includeDepth)
	{
		attachments[1].format = Depth.format;
		attachments[1].samples = NUM_SAMPLES;
		attachments[1].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
	}

	// 색상 버퍼 첨부의 바인딩 포인트와 레이아웃 정보를 정의
	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// 깊이 버퍼 첨부의 바인딩 포인트와 레이아웃 정보를 정의
	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// 첨부 지정 - 색상, 깊이, 리졸브, 보존 등
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorReference;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = includeDepth ? &depthReference : NULL;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	// 렌더 패스와 연결된 첨부와 서브패스를 지정
	VkRenderPassCreateInfo rpInfo = {};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpInfo.pNext = NULL;
	rpInfo.attachmentCount = includeDepth ? 2 : 1;
	rpInfo.pAttachments = attachments;
	rpInfo.subpassCount = 1;
	rpInfo.pSubpasses = &subpass;
	rpInfo.dependencyCount = 0;
	rpInfo.pDependencies = NULL;

	// 렌더 패스 개체 생성
	result = vkCreateRenderPass(deviceObj->device, &rpInfo, NULL, &renderPass);
	assert(result == VK_SUCCESS);
}


void VKRenderer::createFrameBuffer(bool includeDepth)
{
	// Dependency on createDepthBuffer(), createRenderPass() and recordSwapChain()
	VkResult  result;
	VkImageView attachments[2];
	attachments[1] = Depth.view;

	VkFramebufferCreateInfo fbInfo = {};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = NULL;
	fbInfo.renderPass = renderPass;
	fbInfo.attachmentCount = includeDepth ? 2 : 1;
	fbInfo.pAttachments = attachments;
	fbInfo.width = width;
	fbInfo.height = height;
	fbInfo.layers = 1;

	uint32_t i;

	//각 스와프 체인 색상 이미지에 해당하는 프레임 버퍼를 만든다.
	framebuffers.clear();
	framebuffers.resize(swapChainObj->scPublicVars.swapchainImageCount);
	for (i = 0; i < swapChainObj->scPublicVars.swapchainImageCount; i++) {
		attachments[0] = swapChainObj->scPublicVars.colorBuffer[i].view;
		result = vkCreateFramebuffer(deviceObj->device, &fbInfo, NULL, &framebuffers.at(i));
		assert(result == VK_SUCCESS);
	}
}

void VKRenderer::createShaders()
{
	void* vertShaderCode, *fragShaderCode;
	size_t sizeVert, sizeFrag;

#ifdef AUTO_COMPILE_GLSL_TO_SPV
	vertShaderCode = readFile("./../Draw.vert", &sizeVert);
	fragShaderCode = readFile("./../Draw.frag", &sizeFrag);
	
	shaderObj.buildShader((const char*)vertShaderCode, (const char*)fragShaderCode);
#else
	vertShaderCode = readFile("./../Draw-vert.spv", &sizeVert);
	fragShaderCode = readFile("./../Draw-frag.spv", &sizeFrag);

	shaderObj.buildShaderModuleWithSPV((uint32_t*)vertShaderCode, sizeVert, (uint32_t*)fragShaderCode, sizeFrag);
#endif
}

void VKRenderer::createPipelineStateManagement()
{
	pipelineObj.createPipelineCache();

	const bool depthPresent = true;
	for (VKDrawable * drawableObj : drawableList)
	{
		// 각각의 드로잉 개체에 대해 해당 파이프라인 생성
		VkPipeline* pipeline = (VkPipeline*)malloc(sizeof(VkPipeline));
		if (pipelineObj.createPipeline(drawableObj, pipeline, &shaderObj, depthPresent))
		{
			pipelineList.push_back(pipeline);
			drawableObj->setPipeline(pipeline);
		}
		else
		{
			free(pipeline);
			pipeline = NULL;
			std::cout << "Pipeline Error" << std::endl;
		}
	}
}

void VKRenderer::destroyCommandBuffer()
{
	VkCommandBuffer cmdBufs[] = { cmdDepthImage };
	vkFreeCommandBuffers(deviceObj->device, cmdPool, sizeof(cmdBufs) / sizeof(VkCommandBuffer), cmdBufs);
}

void VKRenderer::destroyCommandPool()
{
	VKDevice* deviceObj = application->deviceObj;

	vkDestroyCommandPool(deviceObj->device, cmdPool, NULL);
}

void VKRenderer::destroyDepthBuffer()
{
	vkDestroyImage(deviceObj->device, Depth.image, NULL);
	vkDestroyImageView(deviceObj->device, Depth.view, NULL);
	vkFreeMemory(deviceObj->device, Depth.mem, NULL);
}

void VKRenderer::destroyDrawableVertexBuffer()
{
	for (VKDrawable * drawableObj : drawableList)
	{
		drawableObj->destroyVertexBuffer();
	}
}

void VKRenderer::destroyRenderpass()
{
	vkDestroyRenderPass(deviceObj->device, renderPass, NULL);
}

void VKRenderer::destroyFrameBuffers()
{
	for (uint32_t i = 0; i < swapChainObj->scPublicVars.swapchainImageCount; i++) {
		vkDestroyFramebuffer(deviceObj->device, framebuffers.at(i), NULL);
	}
	framebuffers.clear();
}

void VKRenderer::destroyPipeline()
{
	for (VkPipeline * pipeline : pipelineList)
	{
		vkDestroyPipeline(deviceObj->device, *pipeline, NULL);
		free(pipeline);
	}
	pipelineList.clear();
}
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

//������ �ʱ�ȭ
// - �ʱ�ȭ������ �Է����� �־��� �������� ���������̼� ������ �����
// ������ ü�� ������ ���� �پ��� �䱸������ ������Ų��.
// ������ ü�� Ȯ���� API�� �����ϰ�, ȭ�� ��ü�� �����,
// ���������̼� ���̾ ���� �� �����ϴ� ť�� ã��,
// ������� ���� ȣȯ�Ǵ� �̹��� ������ �������� ���� �۾��� ���Եȴ�.
void VKRenderer::initialize()
{
	// �� ������ ����
	createPresentationWindow(1280, 720);

	// ������ ü�� �ʱ�ȭ
	swapChainObj->intializeSwapChain();

	// Ŀ�ǵ� ���۰� �ʿ��ϹǷ� ���� Ŀ�ǵ� ���� Ǯ ����
	createCommandPool();

	// ������ ü�� ���� �̹��� ����
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

// 5. ���������̼� ���� ������
// - ���÷��̿� ���������̼� ������ �׸��� Windows �޽����� ó���Ѵ�.
// - ����ڰ� �ݱ� ��ư�� ������, ���������̼� ������ �����ϰ� ������ ������ �����Ѵ�.
bool VKRenderer::render()
{
	MSG msg;   // �޽���
	PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
	if (msg.message == WM_QUIT) { // WM_QUIT �޽����� ���� ������ ������ ������
		return false;
	}
	TranslateMessage(&msg);
	DispatchMessage(&msg);
	RedrawWindow(window, NULL, NULL, RDW_INTERNALPAINT);
	return true;
}

//1. ���������̼� ���� ����
// ���÷��� ������ ������ ü�� ���� �̹����� ǥ���ϴ� �� ����� ���� �ý��� ����
// Windows, CreateWindowEX() API�� ����� Ȯ��� ���� ��Ÿ�Ϸ� ������ �˾� �Ǵ� �ڽ� ������ ����
void VKRenderer::createPresentationWindow(const int& windowWidth, const int& windowHeight)
{
#ifdef _WIN32
	width = windowWidth;
	height = windowHeight;
	assert(width > 0 || height > 0);

	WNDCLASSEX  winInfo;

	sprintf(name, "Swapchain presentation window");
	memset(&winInfo, 0, sizeof(WNDCLASSEX));
	// ���� Ŭ���� ����ü �ʱ�ȭ
	winInfo.cbSize = sizeof(WNDCLASSEX);
	winInfo.style = CS_HREDRAW | CS_VREDRAW;
	winInfo.lpfnWndProc = WndProc;
	winInfo.cbClsExtra = 0;
	winInfo.cbWndExtra = 0;
	winInfo.hInstance = connection; // Windows �ν��Ͻ�
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


// ���� �̹��� ���̾ƿ� ������ ������ �� ���̾ƿ� �������� �����Ѵ�.
// �̹��� ��ü�� ó�� �����ǰ� �̸� ���ǵ� ���̾ƿ��� ������� �ʾƼ� ���� �̹��� ���̾ƿ��� VK_IMAGE_LAYOUT_UNDEFINED�� �����ȴ�. 
// ����/���ٽ� �׽�Ʈ�� ���� �̹��� ���̾ƿ��� �����ϹǷ� ���ο� �̹��� ���̾ƿ���
// VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL �뵵 �������� �����ؾ� �Ѵ�.
void VKRenderer::setImageLayout(
	VkImage image,
	VkImageAspectFlags aspectMask,
	VkImageLayout oldImageLayout,
	VkImageLayout newImageLayout,
	VkAccessFlagBits srcAccessMask,
	const VkCommandBuffer& cmdBuf)
{
	// Command Buffer ���Ӽ�...
	assert(cmdBuf != VK_NULL_HANDLE);

	// deviceObj->queue�� �ʰ�ȭ�ؾ� ��
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
		// �� �̹����κ����� ��� ���簡 �Ϸ�� ���� �����Ѵ�.
		// �� ���̾ƿ��� �̹����� ����� ���(destination)���θ� ����� �� �ִ�.
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

		// �� �̹������� ���� �Ǵ� CPU ���� �۾��� �Ϸ�� ���� �����Ѵ�.
		// �� ���̾ƿ��� �̹����� �б� ���� ���̴� ���ҽ��θ� ����� �� �ִ�.
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;

		// �� ���̾ƿ��� �̹����� ������ ������ ���� ÷�ηθ� ����� �� �ִ�.
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		break;

		// �� ���̾ƿ��� �̹����� ������ ������ ����/���ٽ� ÷�ο��� ����� �� �ִ�.
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	}

	VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags destStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(cmdBuf, srcStages, destStages, 0, 0, NULL, 0, NULL, 1, &imgMemoryBarrier);
}

#ifdef _WIN32
// WM_PAINT : ���� ������, WM_SIZE : ??? , WM_CLOSE : �ݱ�
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

			// ������ �̹��� ǥ��
			present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			present.swapchainCount = 1;
			present.pSwapchains = &appObj->rendererObj->swapChainObj->scPublicVars.swapChain;
			present.pImageIndices = &appObj->rendererObj->swapChainObj->scPublicVars.currentColorBuffer;

			// ǥ�ø� ���� �̹����� ť�� ����
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

// 3. Ŀ�ǵ� Ǯ ����
// - VKRenderer Ŭ������ Command Pool�� Ŀ�ǵ� ���� �Ҵ�� ���� �̹���, ���������� ������Ʈ ����, ����� ������Ƽ�� ���� �پ��� �۾����� ����Ѵ�.
// - VKCommandPoolCreateInfo���� Ŀ�ǵ� ���۰� �Ҵ�ž� �ϴ� �׷��Ƚ� ť�� �ε����� ��� �ִ�.
void VKRenderer::createCommandPool()
{
	VKDevice* deviceObj = application->deviceObj;
	/* Depends on intializeSwapChainExtension() */
	VkResult  res;

	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.pNext = NULL;
	cmdPoolInfo.queueFamilyIndex = deviceObj->graphicsQueueWithPresentIndex;	// �׷��Ƚ� ť �ε���
	cmdPoolInfo.flags = 0;

	res = vkCreateCommandPool(deviceObj->device, &cmdPoolInfo, NULL, &cmdPool); // Ŀ�ǵ� Ǯ ����
	assert(res == VK_SUCCESS);
}

// 4. ������ ü�ΰ� ���� �̹��� ����
// - ������ ü�ΰ� ���� �̹����� �����ϱ� ���� ������.
void VKRenderer::buildSwapChainAndDepthImage()
{
	// Ŀ�ǵ带 ������ ������ ť ��������
	deviceObj->getDeviceQueue();

	// ������ ü���� �����ϰ� ���� �̹��� ��������
	swapChainObj->createSwapChain(cmdDepthImage);

	// ���� �̹��� ����
	createDepthImage();
}

// ���� ���� �̹��� ����
void VKRenderer::createDepthImage()
{
	VkResult  result;
	bool  pass;

	VkImageCreateInfo imageInfo = {};

	// ���� ������ ���ǵ��� ���� ��� 16bit ���� ���
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

	// ����ڰ� �̹��� ������ �̹��� ��ü�� ����
	result = vkCreateImage(deviceObj->device, &imageInfo, NULL, &Depth.image);
	assert(result == VK_SUCCESS);

	// �̹��� �޸� �䱸���� ��������(���� �̹��� ��ü�� ������ �޸� ������ �Ҵ��ϴ� �� �ʿ��� �� ũ�⸦ ������)
	VkMemoryRequirements memRqrmnt;
	vkGetImageMemoryRequirements(deviceObj->device, Depth.image, &memRqrmnt);

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.pNext = NULL;
	memAlloc.allocationSize = 0;
	memAlloc.memoryTypeIndex = 0;
	memAlloc.allocationSize = memRqrmnt.size;

	// �޸� �Ӽ��� ���� �޸� ������ ����
	pass = deviceObj->memoryTypeFromProperties(memRqrmnt.memoryTypeBits, 0, /* No requirements */ &memAlloc.memoryTypeIndex);
	assert(pass);

	// ������ �޸𸮸� ���� �̹����� �Ҵ�
	result = vkAllocateMemory(deviceObj->device, &memAlloc, NULL, &Depth.mem);
	assert(result == VK_SUCCESS);

	// ���� �̹����� �Ҵ�� �޸𸮸� ���ε�
	result = vkBindImageMemory(deviceObj->device, Depth.image, Depth.mem, 0);
	assert(result == VK_SUCCESS);


	// �̹��� �� ����...
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

	// Ŀ�ǵ� ���۸� �̿��� ���� �̹��� ����
	// ���⿡�� Ŀ�ǵ� ���� �Ҵ�, ���ڵ��� ����/���� ��ġ�� ������ ���Ե�
	CommandBufferMgr::allocCommandBuffer(&deviceObj->device, cmdPool, &cmdDepthImage);
	CommandBufferMgr::beginCommandBuffer(cmdDepthImage);
	{
		// ���� ���ٽ� �̹��� ���̾ƿ��� optimal�� ����
		// �� �Լ��� �޸� �庮�� ���ڵ��ϴ� ����� �Լ�
		setImageLayout(Depth.image,
			imgViewInfo.subresourceRange.aspectMask,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, (VkAccessFlagBits)0, cmdDepthImage);
	}
	CommandBufferMgr::endCommandBuffer(cmdDepthImage);
	CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &cmdDepthImage);
	// �� ����� cmdDepthImage Ŀ�ǵ� ���ۿ� ���ڵ��ϸ�, ���ӵ� ���ҽ��� ���ҽ��� �׼����ϱ� ���� ������ �̹��� ���̾ƿ� �䱸������ �����ϰ� �ȴ�.
	
	// �̹��� �並 �����ϰ� ���� ���α׷��� ����� �� �ֵ��� ����
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
	// ���� �̹����� ��� ���� createSwapChain()
	// ���� �̹����� ��� ���� createDepthImage()
	// �� �ѿ� ���� ���Ӽ��� ����
	
	VkResult  result;

	// ���� ���ۿ� ���� ���۸� ÷�� �������� ���� �н� �ν��Ͻ��� ÷����.
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

	// ���� ���� ÷�ο� ���� ÷�� �Ӽ��� ���� ������ �����ϴ��� ����
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

	// ���� ���� ÷���� ���ε� ����Ʈ�� ���̾ƿ� ������ ����
	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// ���� ���� ÷���� ���ε� ����Ʈ�� ���̾ƿ� ������ ����
	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// ÷�� ���� - ����, ����, ������, ���� ��
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

	// ���� �н��� ����� ÷�ο� �����н��� ����
	VkRenderPassCreateInfo rpInfo = {};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpInfo.pNext = NULL;
	rpInfo.attachmentCount = includeDepth ? 2 : 1;
	rpInfo.pAttachments = attachments;
	rpInfo.subpassCount = 1;
	rpInfo.pSubpasses = &subpass;
	rpInfo.dependencyCount = 0;
	rpInfo.pDependencies = NULL;

	// ���� �н� ��ü ����
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

	//�� ������ ü�� ���� �̹����� �ش��ϴ� ������ ���۸� �����.
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
		// ������ ����� ��ü�� ���� �ش� ���������� ����
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
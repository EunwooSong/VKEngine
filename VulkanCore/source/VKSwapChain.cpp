#include "VKSwapChain.h"

#include "VKDevice.h"
#include "VKInstance.h"
#include "VKRenderer.h"
#include "VKApplication.h"

// ************************ MACRO ************************ 
// INSTANCE_FUNC_PTR, DEVICE_FUNC_PTR
// - 인스턴스 지정이나 장치 확장판을 쿼리할 수 있다.
// - 이 매크로를 사용해 WSI(Window System Integration) 확장 API의 함수 포인터를 가져온다.
// - VKLayerAndExtenstion에서 디버깅 동적 쿼리할 때 사용한거 참고...
#define INSTANCE_FUNC_PTR(instance, entrypoint){											\
    fp##entrypoint = (PFN_vk##entrypoint) vkGetInstanceProcAddr(instance, "vk"#entrypoint); \
    if (fp##entrypoint == NULL) {															\
        std::cout << "Unable to locate the vkGetDeviceProcAddr: vk"#entrypoint;				\
        exit(-1);																			\
    }																						\
}

#define DEVICE_FUNC_PTR(dev, entrypoint){													\
    fp##entrypoint = (PFN_vk##entrypoint) vkGetDeviceProcAddr(dev, "vk"#entrypoint);		\
    if (fp##entrypoint == NULL) {															\
        std::cout << "Unable to locate the vkGetDeviceProcAddr: vk"#entrypoint;				\
        exit(-1);																			\
    }																						\
}

VKSwapChain::VKSwapChain(VKRenderer* renderer)
{
	rendererObj = renderer;
	appObj = VKApplication::GetInstance();
}

VKSwapChain::~VKSwapChain()
{
	scPrivateVars.swapchainImages.clear();
	scPrivateVars.surfFormats.clear();
	scPrivateVars.presentModes.clear();
}

void VKSwapChain::intializeSwapChain()
{
	// Querying swapchain extensions
	createSwapChainExtensions();

	// Create surface and associate with Windowing
	createSurface();

	// Getting a graphics queue with presentation support
	uint32_t index = getGraphicsQueueWithPresentationSupport();
	if (index == UINT32_MAX)
	{
		std::cout << "Could not find a graphics and a present queue\nCould not find a graphics and a present queue\n";
		exit(-1);
	}
	rendererObj->getDevice()->graphicsQueueWithPresentIndex = index;

	// Get the list of formats that are supported
	getSupportedFormats();
}

void VKSwapChain::createSwapChain(const VkCommandBuffer& cmd)
{
	/* This function retreive swapchain image and create those images- image view */

	// use extensions and get the surface capabilities, present mode
	getSurfaceCapabilitiesAndPresentMode();

	// Gather necessary information for present mode.
	managePresentMode();

	// Create the swap chain images
	createSwapChainColorImages();

	// Get the create color image drawing surfaces
	createColorImageView(cmd);
}

void VKSwapChain::destroySwapChain()
{
	VKDevice* deviceObj = appObj->deviceObj;

	for (uint32_t i = 0; i < scPublicVars.swapchainImageCount; i++) {
		vkDestroyImageView(deviceObj->device, scPublicVars.colorBuffer[i].view, NULL);
	}
	fpDestroySwapchainKHR(deviceObj->device, scPublicVars.swapChain, NULL);

	vkDestroySurfaceKHR(appObj->instanceObj.instance, scPublicVars.surface, NULL);
}

VkResult VKSwapChain::createSwapChainExtensions()
{
	// createPresentationWindow()의 의존성
	VkInstance& instance = appObj->instanceObj.instance;
	VkDevice& device = appObj->deviceObj->device;

	// 스와프 체인 확장판 함수 포인터를 기반으로 인스턴스 가져오기
	INSTANCE_FUNC_PTR(instance, GetPhysicalDeviceSurfaceSupportKHR);
	INSTANCE_FUNC_PTR(instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	INSTANCE_FUNC_PTR(instance, GetPhysicalDeviceSurfaceFormatsKHR);
	INSTANCE_FUNC_PTR(instance, GetPhysicalDeviceSurfacePresentModesKHR);
	INSTANCE_FUNC_PTR(instance, DestroySurfaceKHR);

	// 스와프 체인 확장판 함수 포인터를 기반으로 장치 가져오기
	DEVICE_FUNC_PTR(device, CreateSwapchainKHR);
	DEVICE_FUNC_PTR(device, DestroySwapchainKHR);
	DEVICE_FUNC_PTR(device, GetSwapchainImagesKHR);
	DEVICE_FUNC_PTR(device, AcquireNextImageKHR);
	DEVICE_FUNC_PTR(device, QueuePresentKHR);

	return VK_SUCCESS;
}

// 이미지 포맷의 개수를 가져온다.
void VKSwapChain::getSupportedFormats()
{
	VkPhysicalDevice gpu = *rendererObj->getDevice()->gpu;
	VkResult  result;

	// 지원되는 VkFormats 수 얻기
	uint32_t formatCount;
	result = fpGetPhysicalDeviceSurfaceFormatsKHR(gpu, scPublicVars.surface, &formatCount, NULL);
	assert(result == VK_SUCCESS);
	scPrivateVars.surfFormats.clear();
	scPrivateVars.surfFormats.resize(formatCount);

	// 할당된 개체에서 VkFormats 가져오기
	result = fpGetPhysicalDeviceSurfaceFormatsKHR(gpu, scPublicVars.surface, &formatCount, &scPrivateVars.surfFormats[0]);
	assert(result == VK_SUCCESS);

	// VK_FORMAT_UNDEFINED일 때 화면에 아무것도 없음
	// 32bit RGBA 형식을 사용함
	if (formatCount == 1 && scPrivateVars.surfFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		scPublicVars.format = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		assert(formatCount >= 1);
		scPublicVars.format = scPrivateVars.surfFormats[0].format;
	}
}

// createPresentationWindow()에 따라 윈도 핸들이 필요...
VkResult VKSwapChain::createSurface()
{
	VkResult  result;
	// Depends on createPresentationWindow(), need an empty window handle
	VkInstance& instance = appObj->instanceObj.instance;

	// 화면 설명을 설정
#ifdef _WIN32
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.hinstance = rendererObj->connection;
	createInfo.hwnd = rendererObj->window;

	// VkSurfaceKHR 설정
	result = vkCreateWin32SurfaceKHR(instance, &createInfo, NULL, &scPublicVars.surface);

#else  // _WIN32

	VkXcbSurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.connection = rendererObj->connection;
	createInfo.window = rendererObj->window;

	result = vkCreateXcbSurfaceKHR(instance, &createInfo, NULL, &surface);
#endif // _WIN32

	assert(result == VK_SUCCESS);
	return result;
}

uint32_t VKSwapChain::getGraphicsQueueWithPresentationSupport()
{
	VKDevice* device = appObj->deviceObj;
	uint32_t queueCount = device->queueFamilyCount;
	VkPhysicalDevice gpu = *device->gpu;
	std::vector<VkQueueFamilyProperties>& queueProps = device->queueFamilyProps;

	// 각각의 큐에 대해 프레젠테이션 상태 가져오기를 반복
	VkBool32* supportsPresent = (VkBool32*)malloc(queueCount * sizeof(VkBool32));
	for (uint32_t i = 0; i < queueCount; i++) {
		fpGetPhysicalDeviceSurfaceSupportKHR(gpu, i, scPublicVars.surface, &supportsPresent[i]);
	}

	// 큐 패밀리에서 그래픽스와 프레젠테이션를 동시에 지원하는 큐를 찾기
	uint32_t graphicsQueueNodeIndex = UINT32_MAX;
	uint32_t presentQueueNodeIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueCount; i++) {
		// 그래픽스 큐이고 
		if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
			if (graphicsQueueNodeIndex == UINT32_MAX) {
				graphicsQueueNodeIndex = i;
			}

			// 프레젠테이션을 지원하는지 확인
			if (supportsPresent[i] == VK_TRUE) {
				graphicsQueueNodeIndex = i;
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	if (presentQueueNodeIndex == UINT32_MAX) {
		// 그래픽스와 프레젠테이션을 지원하는 큐를 찾지 못한 경우
		// 독립적인 프레젠테이션 큐를 검색
		for (uint32_t i = 0; i < queueCount; ++i) {
			if (supportsPresent[i] == VK_TRUE) {
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	free(supportsPresent);

	// 프레젠테이션 큐 검색에 실패한 경우 오류 발생
	if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX) {
		return  UINT32_MAX;
	}

	return graphicsQueueNodeIndex;
}


void VKSwapChain::getSurfaceCapabilitiesAndPresentMode()
{
	VkResult  result;
	VkPhysicalDevice gpu = *appObj->deviceObj->gpu;
	result = fpGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, scPublicVars.surface, &scPrivateVars.surfCapabilities);
	assert(result == VK_SUCCESS);

	result = fpGetPhysicalDeviceSurfacePresentModesKHR(gpu, scPublicVars.surface, &scPrivateVars.presentModeCount, NULL);
	assert(result == VK_SUCCESS);

	scPrivateVars.presentModes.clear();
	scPrivateVars.presentModes.resize(scPrivateVars.presentModeCount);
	assert(scPrivateVars.presentModes.size() >= 1);

	result = fpGetPhysicalDeviceSurfacePresentModesKHR(gpu, scPublicVars.surface, &scPrivateVars.presentModeCount, &scPrivateVars.presentModes[0]);
	assert(result == VK_SUCCESS);

	if (scPrivateVars.surfCapabilities.currentExtent.width == (uint32_t)-1)
	{
		// 화면의 폭과 높이가 정의되지 않은 경우 이미지 크기와 동일하게 설정
		scPrivateVars.swapChainExtent.width = rendererObj->width;
		scPrivateVars.swapChainExtent.height = rendererObj->height;
	}
	else
	{
		// 화면의 크기가 정의된 경우 스와프 체인 크기를 맞추어야 한다.
		scPrivateVars.swapChainExtent = scPrivateVars.surfCapabilities.currentExtent;
	}
}

void VKSwapChain::managePresentMode()
{
	// MAILBOX - 가장 지연이 작고, 깨짐이 발생하지 않는 모드
	// 사용할 수 없는 경우 IMMEDIATE 시도 - 가장 빠르지만 (깨짐 발생)
	// 사용할 수 없는 경우 FIFO 사용
	scPrivateVars.swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (size_t i = 0; i < scPrivateVars.presentModeCount; i++) {
		if (scPrivateVars.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			scPrivateVars.swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((scPrivateVars.swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) &&
			(scPrivateVars.presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)) {
			scPrivateVars.swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	// 스와프 체인에서 사용할 vKiMAGE의 수 결정
	scPrivateVars.desiredNumberOfSwapChainImages = scPrivateVars.surfCapabilities.minImageCount + 1;
	if ((scPrivateVars.surfCapabilities.maxImageCount > 0) &&
		(scPrivateVars.desiredNumberOfSwapChainImages > scPrivateVars.surfCapabilities.maxImageCount))
	{
		// 응용 프로그램은 원하는 것보다 적은 수의 이미지를 처리해야함
		scPrivateVars.desiredNumberOfSwapChainImages = scPrivateVars.surfCapabilities.maxImageCount;
	}

	if (scPrivateVars.surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		scPrivateVars.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else {
		scPrivateVars.preTransform = scPrivateVars.surfCapabilities.currentTransform;
	}
}

void VKSwapChain::createSwapChainColorImages()
{
	VkResult  result;

	VkSwapchainCreateInfoKHR swapChainInfo = {};
	swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainInfo.pNext = NULL;
	swapChainInfo.surface = scPublicVars.surface;
	swapChainInfo.minImageCount = scPrivateVars.desiredNumberOfSwapChainImages;
	swapChainInfo.imageFormat = scPublicVars.format;
	swapChainInfo.imageExtent.width = scPrivateVars.swapChainExtent.width;
	swapChainInfo.imageExtent.height = scPrivateVars.swapChainExtent.height;
	swapChainInfo.preTransform = scPrivateVars.preTransform;
	swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainInfo.imageArrayLayers = 1;
	swapChainInfo.presentMode = scPrivateVars.swapchainPresentMode;
	swapChainInfo.oldSwapchain = VK_NULL_HANDLE;
	swapChainInfo.clipped = true;
	swapChainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainInfo.queueFamilyIndexCount = 0;
	swapChainInfo.pQueueFamilyIndices = NULL;

	// 스와프 체인 개체 생성
	result = fpCreateSwapchainKHR(rendererObj->getDevice()->device, &swapChainInfo, NULL, &scPublicVars.swapChain);
	assert(result == VK_SUCCESS);

	// 스와프 체인이 갖고 있는 이미지의 수를 가져온다. 
	result = fpGetSwapchainImagesKHR(rendererObj->getDevice()->device, scPublicVars.swapChain, &scPublicVars.swapchainImageCount, NULL);
	assert(result == VK_SUCCESS);

	scPrivateVars.swapchainImages.clear();
	
	// 이밉지들을 가져오기 위해 스와프 체인 이미지의 배열을 만든다.
	scPrivateVars.swapchainImages.resize(scPublicVars.swapchainImageCount);
	assert(scPrivateVars.swapchainImages.size() >= 1);

	// 스와프 체인 이미지 화면들을 가져온다.
	result = fpGetSwapchainImagesKHR(rendererObj->getDevice()->device, scPublicVars.swapChain, &scPublicVars.swapchainImageCount, &scPrivateVars.swapchainImages[0]);
	assert(result == VK_SUCCESS);
}

// 각 스와프 체인 이미지 개체에 대해 scPublicVars.swapchainImageCount에서 사용할 수 있는
// 이미지 개체 목록을 반복해 해당 이미지 뷰를 생성한다. 이 개수는 앞에서 fpGetSwapCchainImagesKHR()API로 검색한다.
// 생성한 이미지 뷰는 벡터 목록으로 다시 푸시되고. 정확한 프런트오 ㅏ백 버퍼 이미지를 참조하는 데 사용된다.
void VKSwapChain::createColorImageView(const VkCommandBuffer& cmd)
{
	VkResult  result;
	scPublicVars.colorBuffer.clear();
	for (uint32_t i = 0; i < scPublicVars.swapchainImageCount; i++) {
		SwapChainBuffer sc_buffer;

		VkImageViewCreateInfo imgViewInfo = {};
		imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imgViewInfo.pNext = NULL;
		imgViewInfo.format = scPublicVars.format;
		imgViewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
		imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imgViewInfo.subresourceRange.baseMipLevel = 0;
		imgViewInfo.subresourceRange.levelCount = 1;
		imgViewInfo.subresourceRange.baseArrayLayer = 0;
		imgViewInfo.subresourceRange.layerCount = 1;
		imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imgViewInfo.flags = 0;

		sc_buffer.image = scPrivateVars.swapchainImages[i];

		// 스와프 체인을 (우리가) 소유하고 있지 않으므로 이미지 레이아웃을 설정할 수 없음
		// 설정에 따라 프로그램에 오류가 발생할 수 있음
		// 이미지 레이아웃은 응용 프로그램이 아니라 WSI에 의해 생성됨
		imgViewInfo.image = sc_buffer.image;

		result = vkCreateImageView(rendererObj->getDevice()->device, &imgViewInfo, NULL, &sc_buffer.view);
		scPublicVars.colorBuffer.push_back(sc_buffer);
		assert(result == VK_SUCCESS);
	}
	scPublicVars.currentColorBuffer = 0;
}
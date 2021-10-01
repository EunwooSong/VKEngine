#include "VKSwapChain.h"

#include "VKDevice.h"
#include "VKInstance.h"
#include "VKRenderer.h"
#include "VKApplication.h"

// ************************ MACRO ************************ 
// INSTANCE_FUNC_PTR, DEVICE_FUNC_PTR
// - �ν��Ͻ� �����̳� ��ġ Ȯ������ ������ �� �ִ�.
// - �� ��ũ�θ� ����� WSI(Window System Integration) Ȯ�� API�� �Լ� �����͸� �����´�.
// - VKLayerAndExtenstion���� ����� ���� ������ �� ����Ѱ� ����...
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
	// createPresentationWindow()�� ������
	VkInstance& instance = appObj->instanceObj.instance;
	VkDevice& device = appObj->deviceObj->device;

	// ������ ü�� Ȯ���� �Լ� �����͸� ������� �ν��Ͻ� ��������
	INSTANCE_FUNC_PTR(instance, GetPhysicalDeviceSurfaceSupportKHR);
	INSTANCE_FUNC_PTR(instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	INSTANCE_FUNC_PTR(instance, GetPhysicalDeviceSurfaceFormatsKHR);
	INSTANCE_FUNC_PTR(instance, GetPhysicalDeviceSurfacePresentModesKHR);
	INSTANCE_FUNC_PTR(instance, DestroySurfaceKHR);

	// ������ ü�� Ȯ���� �Լ� �����͸� ������� ��ġ ��������
	DEVICE_FUNC_PTR(device, CreateSwapchainKHR);
	DEVICE_FUNC_PTR(device, DestroySwapchainKHR);
	DEVICE_FUNC_PTR(device, GetSwapchainImagesKHR);
	DEVICE_FUNC_PTR(device, AcquireNextImageKHR);
	DEVICE_FUNC_PTR(device, QueuePresentKHR);

	return VK_SUCCESS;
}

// �̹��� ������ ������ �����´�.
void VKSwapChain::getSupportedFormats()
{
	VkPhysicalDevice gpu = *rendererObj->getDevice()->gpu;
	VkResult  result;

	// �����Ǵ� VkFormats �� ���
	uint32_t formatCount;
	result = fpGetPhysicalDeviceSurfaceFormatsKHR(gpu, scPublicVars.surface, &formatCount, NULL);
	assert(result == VK_SUCCESS);
	scPrivateVars.surfFormats.clear();
	scPrivateVars.surfFormats.resize(formatCount);

	// �Ҵ�� ��ü���� VkFormats ��������
	result = fpGetPhysicalDeviceSurfaceFormatsKHR(gpu, scPublicVars.surface, &formatCount, &scPrivateVars.surfFormats[0]);
	assert(result == VK_SUCCESS);

	// VK_FORMAT_UNDEFINED�� �� ȭ�鿡 �ƹ��͵� ����
	// 32bit RGBA ������ �����
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

// createPresentationWindow()�� ���� ���� �ڵ��� �ʿ�...
VkResult VKSwapChain::createSurface()
{
	VkResult  result;
	// Depends on createPresentationWindow(), need an empty window handle
	VkInstance& instance = appObj->instanceObj.instance;

	// ȭ�� ������ ����
#ifdef _WIN32
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.hinstance = rendererObj->connection;
	createInfo.hwnd = rendererObj->window;

	// VkSurfaceKHR ����
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

	// ������ ť�� ���� ���������̼� ���� �������⸦ �ݺ�
	VkBool32* supportsPresent = (VkBool32*)malloc(queueCount * sizeof(VkBool32));
	for (uint32_t i = 0; i < queueCount; i++) {
		fpGetPhysicalDeviceSurfaceSupportKHR(gpu, i, scPublicVars.surface, &supportsPresent[i]);
	}

	// ť �йи����� �׷��Ƚ��� ���������̼Ǹ� ���ÿ� �����ϴ� ť�� ã��
	uint32_t graphicsQueueNodeIndex = UINT32_MAX;
	uint32_t presentQueueNodeIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueCount; i++) {
		// �׷��Ƚ� ť�̰� 
		if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
			if (graphicsQueueNodeIndex == UINT32_MAX) {
				graphicsQueueNodeIndex = i;
			}

			// ���������̼��� �����ϴ��� Ȯ��
			if (supportsPresent[i] == VK_TRUE) {
				graphicsQueueNodeIndex = i;
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	if (presentQueueNodeIndex == UINT32_MAX) {
		// �׷��Ƚ��� ���������̼��� �����ϴ� ť�� ã�� ���� ���
		// �������� ���������̼� ť�� �˻�
		for (uint32_t i = 0; i < queueCount; ++i) {
			if (supportsPresent[i] == VK_TRUE) {
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	free(supportsPresent);

	// ���������̼� ť �˻��� ������ ��� ���� �߻�
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
		// ȭ���� ���� ���̰� ���ǵ��� ���� ��� �̹��� ũ��� �����ϰ� ����
		scPrivateVars.swapChainExtent.width = rendererObj->width;
		scPrivateVars.swapChainExtent.height = rendererObj->height;
	}
	else
	{
		// ȭ���� ũ�Ⱑ ���ǵ� ��� ������ ü�� ũ�⸦ ���߾�� �Ѵ�.
		scPrivateVars.swapChainExtent = scPrivateVars.surfCapabilities.currentExtent;
	}
}

void VKSwapChain::managePresentMode()
{
	// MAILBOX - ���� ������ �۰�, ������ �߻����� �ʴ� ���
	// ����� �� ���� ��� IMMEDIATE �õ� - ���� �������� (���� �߻�)
	// ����� �� ���� ��� FIFO ���
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

	// ������ ü�ο��� ����� vKiMAGE�� �� ����
	scPrivateVars.desiredNumberOfSwapChainImages = scPrivateVars.surfCapabilities.minImageCount + 1;
	if ((scPrivateVars.surfCapabilities.maxImageCount > 0) &&
		(scPrivateVars.desiredNumberOfSwapChainImages > scPrivateVars.surfCapabilities.maxImageCount))
	{
		// ���� ���α׷��� ���ϴ� �ͺ��� ���� ���� �̹����� ó���ؾ���
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

	// ������ ü�� ��ü ����
	result = fpCreateSwapchainKHR(rendererObj->getDevice()->device, &swapChainInfo, NULL, &scPublicVars.swapChain);
	assert(result == VK_SUCCESS);

	// ������ ü���� ���� �ִ� �̹����� ���� �����´�. 
	result = fpGetSwapchainImagesKHR(rendererObj->getDevice()->device, scPublicVars.swapChain, &scPublicVars.swapchainImageCount, NULL);
	assert(result == VK_SUCCESS);

	scPrivateVars.swapchainImages.clear();
	
	// �̹������� �������� ���� ������ ü�� �̹����� �迭�� �����.
	scPrivateVars.swapchainImages.resize(scPublicVars.swapchainImageCount);
	assert(scPrivateVars.swapchainImages.size() >= 1);

	// ������ ü�� �̹��� ȭ����� �����´�.
	result = fpGetSwapchainImagesKHR(rendererObj->getDevice()->device, scPublicVars.swapChain, &scPublicVars.swapchainImageCount, &scPrivateVars.swapchainImages[0]);
	assert(result == VK_SUCCESS);
}

// �� ������ ü�� �̹��� ��ü�� ���� scPublicVars.swapchainImageCount���� ����� �� �ִ�
// �̹��� ��ü ����� �ݺ��� �ش� �̹��� �並 �����Ѵ�. �� ������ �տ��� fpGetSwapCchainImagesKHR()API�� �˻��Ѵ�.
// ������ �̹��� ��� ���� ������� �ٽ� Ǫ�õǰ�. ��Ȯ�� ����Ʈ�� ���� ���� �̹����� �����ϴ� �� ���ȴ�.
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

		// ������ ü���� (�츮��) �����ϰ� ���� �����Ƿ� �̹��� ���̾ƿ��� ������ �� ����
		// ������ ���� ���α׷��� ������ �߻��� �� ����
		// �̹��� ���̾ƿ��� ���� ���α׷��� �ƴ϶� WSI�� ���� ������
		imgViewInfo.image = sc_buffer.image;

		result = vkCreateImageView(rendererObj->getDevice()->device, &imgViewInfo, NULL, &sc_buffer.view);
		scPublicVars.colorBuffer.push_back(sc_buffer);
		assert(result == VK_SUCCESS);
	}
	scPublicVars.currentColorBuffer = 0;
}
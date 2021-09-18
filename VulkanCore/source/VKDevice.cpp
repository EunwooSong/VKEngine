#include "VKDevice.h"
#include "VKInstance.h"
#include "VKApplication.h"

VKDevice::VKDevice(VkPhysicalDevice* gpu)
{
	this->gpu = gpu;
}

VKDevice::~VKDevice()
{
}

//드로잉 기능이 있는 큐, 그래픽스 큐가 필요함...(단일 큐)
VkResult VKDevice::createDevice(std::vector<const char*>& layers, std::vector<const char*>& extensions)
{
	layerExtension.appRequestedLayerNames = layers;
	layerExtension.appRequestedExtensionNames = extensions;

	// 개체 정보의 생성
	VkResult result;
	float queuePriorities[1] = { 0.0 };
	VkDeviceQueueCreateInfo queueInfo = {}; //큐 정보

	queueInfo.queueFamilyIndex = graphicsQueueIndex;
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.pNext = NULL;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = queuePriorities;

	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = NULL;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.enabledLayerCount = 0;

	// 장치 레이어는 사용되지 않음
	deviceInfo.ppEnabledLayerNames = NULL;											
	deviceInfo.enabledExtensionCount = (uint32_t)extensions.size();
	deviceInfo.ppEnabledExtensionNames = extensions.size() ? extensions.data() : NULL;
	deviceInfo.pEnabledFeatures = NULL;

	result = vkCreateDevice(*gpu, &deviceInfo, NULL, &device);
	assert(result == VK_SUCCESS);

	return result;
}

void VKDevice::destroyDevice()
{
	vkDestroyDevice(device, NULL);
}

void VKDevice::getPhysicalDeviceQueuesAndProperties()
{
	//vkGetPhysicalDeviceQueueFamilyProperties?
	// 큐 패밀리 속성의 수를 알 수 있음...
	// - physicalDevice : 큐 속성들이 검색될 물리적 장치의 핸들
	// - pQueueFamilyPropertyCount : 이 장치에 의해 공개된 큐 패밀리의 개수를 참조한다.
	// - pQueueFamilyProperties : queueFamilyPropertyCount와 같은 크기의 배열로 된 이 필드에 큐 패밀리의 속성을 저장한다.

	// 두 번째 파라미터를 NULL로 해서 큐 패밀리의 수를 쿼리
	vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueFamilyCount, NULL);

	// 큐 속성을 저장할 공간 할당
	queueFamilyProps.resize(queueFamilyCount);

	// 큐 패밀리 속성을 가져옴
	vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueFamilyCount, queueFamilyProps.data());
}

uint32_t VKDevice::getGraphicsQueueHandle()
{
	//	1. Get the number of Queues supported by the Physical device
	//	2. Get the properties each Queue type or Queue Family
	//			There could be 4 Queue type or Queue families supported by physical device - 
	//			Graphics Queue	- VK_QUEUE_GRAPHICS_BIT 
	//			Compute Queue	- VK_QUEUE_COMPUTE_BIT
	//			DMA				- VK_QUEUE_TRANSFER_BIT
	//			Sparse memory	- VK_QUEUE_SPARSE_BINDING_BIT
	//	3. Get the index ID for the required Queue family, this ID will act like a handle index to queue.

	bool found = false;
	// 1. 물리적 장치에서 지원하는 큐의 수만큼 반복
	for (unsigned int i = 0; i < queueFamilyCount; i++) {
		// 2. 그래픽스 큐 유형을 가져옴
		//		There could be 4 Queue type or Queue families supported by physical device - 
		//		Graphics Queue		- VK_QUEUE_GRAPHICS_BIT 
		//		Compute Queue		- VK_QUEUE_COMPUTE_BIT
		//		DMA/Transfer Queue	- VK_QUEUE_TRANSFER_BIT
		//		Sparse memory		- VK_QUEUE_SPARSE_BINDING_BIT

		if (queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			// 3. 그래픽스 큐 패밀리의 핸들/인덱스 ID를 가져옴
			found = true;
			graphicsQueueIndex = i;
			break;
		}
	}

	return 0;
}
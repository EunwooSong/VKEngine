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

//����� ����� �ִ� ť, �׷��Ƚ� ť�� �ʿ���...(���� ť)
VkResult VKDevice::createDevice(std::vector<const char*>& layers, std::vector<const char*>& extensions)
{
	layerExtension.appRequestedLayerNames = layers;
	layerExtension.appRequestedExtensionNames = extensions;

	// ��ü ������ ����
	VkResult result;
	float queuePriorities[1] = { 0.0 };
	VkDeviceQueueCreateInfo queueInfo = {}; //ť ����

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

	// ��ġ ���̾�� ������ ����
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
	// ť �йи� �Ӽ��� ���� �� �� ����...
	// - physicalDevice : ť �Ӽ����� �˻��� ������ ��ġ�� �ڵ�
	// - pQueueFamilyPropertyCount : �� ��ġ�� ���� ������ ť �йи��� ������ �����Ѵ�.
	// - pQueueFamilyProperties : queueFamilyPropertyCount�� ���� ũ���� �迭�� �� �� �ʵ忡 ť �йи��� �Ӽ��� �����Ѵ�.

	// �� ��° �Ķ���͸� NULL�� �ؼ� ť �йи��� ���� ����
	vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueFamilyCount, NULL);

	// ť �Ӽ��� ������ ���� �Ҵ�
	queueFamilyProps.resize(queueFamilyCount);

	// ť �йи� �Ӽ��� ������
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
	// 1. ������ ��ġ���� �����ϴ� ť�� ����ŭ �ݺ�
	for (unsigned int i = 0; i < queueFamilyCount; i++) {
		// 2. �׷��Ƚ� ť ������ ������
		//		There could be 4 Queue type or Queue families supported by physical device - 
		//		Graphics Queue		- VK_QUEUE_GRAPHICS_BIT 
		//		Compute Queue		- VK_QUEUE_COMPUTE_BIT
		//		DMA/Transfer Queue	- VK_QUEUE_TRANSFER_BIT
		//		Sparse memory		- VK_QUEUE_SPARSE_BINDING_BIT

		if (queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			// 3. �׷��Ƚ� ť �йи��� �ڵ�/�ε��� ID�� ������
			found = true;
			graphicsQueueIndex = i;
			break;
		}
	}

	return 0;
}
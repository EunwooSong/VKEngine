#include "VKApplication.h"

extern std::vector<const char*> instanceExtensionNames;
extern std::vector<const char*> layerNames;
extern std::vector<const char*> deviceExtensionNames;

VKApplication::VKApplication()
{
	// ���� ���α׷����� ����, ���ŵ� �ν��Ͻ� ���̾��
	instanceObj.layerExtension.getInstanceLayerProperties();
}

VKApplication::~VKApplication()
{
}

VKApplication* VKApplication::GetInstance()
{
	static VKApplication vkApplication;
	return &vkApplication;
}

void VKApplication::initialize()
{
	char title[] = "Hello World!!";

	// Vulkan �ν��Ͻ��� ������ ���̾�� Ȯ���� �̸����� ����
	createVulkanInstance(layerNames, instanceExtensionNames, title);

	// �ý����� ������ ��ġ ����� ������ 
	std::vector<VkPhysicalDevice> gpuList;
	enumeratePhysicalDevices(gpuList);

	// ���� ��� ������ �ϳ��� ��ġ���� ����ϴ� ����
	if (gpuList.size() > 0) {
		handShakeWithDevice(&gpuList[0], layerNames, deviceExtensionNames);
	}
}

void VKApplication::deInitialize()
{
	deviceObj->destroyDevice();
	instanceObj.destroyInstance();
}

// Vulkan �ν��Ͻ��� �����ϱ� ���� ���� �Լ�
VkResult VKApplication::createVulkanInstance(std::vector<const char*>& layers, std::vector<const char*>& extensions, const char* applicationName)
{
	return instanceObj.createInstacne(layers, extensions, applicationName);
}

// ��ġ ��� Ȯ���� �Ӽ� ������ ����
VkResult VKApplication::handShakeWithDevice(VkPhysicalDevice* gpu, std::vector<const char*>& layers, std::vector<const char*>& extensions)
{
	// ����� ���� Vulkan ��ġ ��ü
	// �̸� ���� ������ ��ġ�� ���� ��ġ, ��ġ�� ť�� �Ӽ��� �����Ѵ�.
	deviceObj = new VKDevice(gpu);
	if (!deviceObj) {
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	// ��ġ���� ������ ���̾�� ���̾��� Ȯ������ ���
	deviceObj->layerExtension.getDeviceExtensionProperties(gpu);

	// ������ ��ġ �Ǵ� GPU �Ӽ��� ������
	vkGetPhysicalDeviceProperties(*gpu, &deviceObj->gpuProps);

	// ������ ��ġ �Ǵ� GPU���� �޸� �Ӽ��� ������
	vkGetPhysicalDeviceMemoryProperties(*gpu, &deviceObj->memoryProperties);

	// ������ ��ġ�� ������ ť�� �� �Ӽ��� ����
	deviceObj->getPhysicalDeviceQueuesAndProperties();

	// �׷��Ƚ� ������������ �����ϴ� ť�� ������
	deviceObj->getGraphicsQueueHandle();

	// ���� ��ġ�� �����ϰ� �̸� Ȯ���ϱ� ���� �׷��Ƚ� ť�� ����
	deviceObj->createDevice(layers, extensions);

	return VK_SUCCESS;
}

// �ý����� ��� ������ ������ ��ġ ��ü ���� ��ȯ�Ѵ�.
VkResult VKApplication::enumeratePhysicalDevices(std::vector<VkPhysicalDevice>& gpuList)
{
	// GPU�� �� ����
	uint32_t gpuDeviceCount;

	// GPU�� ���� ������
	VkResult result = vkEnumeratePhysicalDevices(instanceObj.instance, &gpuDeviceCount, NULL);
	
	// �������� �Ϳ� �����ϰų� ��� ������ ������ ��ġ�� ������ ���� �߻�
	assert(result == VK_SUCCESS);
	assert(gpuDeviceCount);

	// �������� ���� ���� Ȯ��
	gpuList.resize(gpuDeviceCount);

	// ������ ��ġ ��ü�� ������
	result = vkEnumeratePhysicalDevices(instanceObj.instance, &gpuDeviceCount, gpuList.data());
	assert(result == VK_SUCCESS);

	return result;
}
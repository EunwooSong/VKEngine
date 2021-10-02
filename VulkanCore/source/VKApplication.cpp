#include "VKApplication.h"
#include "VKRenderer.h"

extern std::vector<const char*> instanceExtensionNames;
extern std::vector<const char*> layerNames;
extern std::vector<const char*> deviceExtensionNames;

VKApplication::VKApplication()
{
	// ���� ���α׷����� ����, ���ŵ� �ν��Ͻ� ���̾��
	instanceObj.layerExtension.getInstanceLayerProperties();

	deviceObj = NULL;
	debugFlag = false;
	rendererObj = NULL;

#if defined(DEBUG) | defined(_DEBUG)
	debugFlag = true;
#endif
}

VKApplication::~VKApplication()
{
	delete rendererObj;
	rendererObj = NULL;
}

VKApplication* VKApplication::GetInstance()
{
	static VKApplication vkApplication;
	return &vkApplication;
}

//********************* CYCLE *********************//
void VKApplication::initialize()
{
	char title[] = "Hello World!!";

	// ���̾ �� ��⿡�� �����Ǵ��� Ȯ��
	if (debugFlag) {
		instanceObj.layerExtension.areLayersSupported(layerNames);
	}

	// Vulkan �ν��Ͻ��� ������ ���̾�� Ȯ���� �̸����� ����
	createVulkanInstance(layerNames, instanceExtensionNames, title);

	// ����� ���� ����
	if (debugFlag) {
		instanceObj.layerExtension.createDebugReportCallback();
	}

	// �ý����� ������ ��ġ ����� ������ 
	std::vector<VkPhysicalDevice> gpuList;
	enumeratePhysicalDevices(gpuList);

	// ���� ��� ������ �ϳ��� ��ġ���� ����ϴ� ����
	if (gpuList.size() > 0) {
		handShakeWithDevice(&gpuList[0], layerNames, deviceExtensionNames);
	}

	rendererObj = new VKRenderer(this, deviceObj);

	rendererObj->initialize();
}

void VKApplication::prepare()
{
	rendererObj->prepare();
}

bool VKApplication::render()
{
	return rendererObj->render();
}

void VKApplication::deInitialize()
{
	// Destroy all the pipeline objects
	rendererObj->destroyPipeline();

	// Destroy the associate pipeline cache
	rendererObj->getPipelineObject()->destroyPipelineCache();

	rendererObj->destroyFrameBuffers();
	rendererObj->destroyRenderpass();
	rendererObj->destroyDrawableVertexBuffer();
	rendererObj->destroyDepthBuffer();
	rendererObj->getSwapChain()->destroySwapChain();
	rendererObj->destroyCommandBuffer();
	rendererObj->destroyCommandPool();
	rendererObj->destroyPresentationWindow();

	deviceObj->destroyDevice();
	if (debugFlag) {
		instanceObj.layerExtension.destroyDebugReportCallback();
	}

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
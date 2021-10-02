#include "VKApplication.h"
#include "VKRenderer.h"

extern std::vector<const char*> instanceExtensionNames;
extern std::vector<const char*> layerNames;
extern std::vector<const char*> deviceExtensionNames;

VKApplication::VKApplication()
{
	// 응용 프로그램에서 시작, 열거된 인스턴스 레이어들
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

	// 레이어가 이 기기에서 지원되는지 확인
	if (debugFlag) {
		instanceObj.layerExtension.areLayersSupported(layerNames);
	}

	// Vulkan 인스턴스를 지정한 레이어와 확장판 이름으로 생성
	createVulkanInstance(layerNames, instanceExtensionNames, title);

	// 디버깅 보고서 생성
	if (debugFlag) {
		instanceObj.layerExtension.createDebugReportCallback();
	}

	// 시스템의 물리적 장치 목록을 가져옴 
	std::vector<VkPhysicalDevice> gpuList;
	enumeratePhysicalDevices(gpuList);

	// 먼저 사용 가능한 하나의 장치만을 사용하는 예제
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

// Vulkan 인스턴스를 생성하기 위한 래퍼 함수
VkResult VKApplication::createVulkanInstance(std::vector<const char*>& layers, std::vector<const char*>& extensions, const char* applicationName)
{
	return instanceObj.createInstacne(layers, extensions, applicationName);
}

// 장치 기반 확장판 속성 정보를 쿼리
VkResult VKApplication::handShakeWithDevice(VkPhysicalDevice* gpu, std::vector<const char*>& layers, std::vector<const char*>& extensions)
{
	// 사용자 정의 Vulkan 장치 개체
	// 이를 통해 물리적 장치와 논리적 장치, 장치의 큐와 속성을 관리한다.
	deviceObj = new VKDevice(gpu);
	if (!deviceObj) {
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	// 장치에서 가용한 레이어와 레이어의 확장판을 출력
	deviceObj->layerExtension.getDeviceExtensionProperties(gpu);

	// 물리적 장치 또는 GPU 속성을 가져옴
	vkGetPhysicalDeviceProperties(*gpu, &deviceObj->gpuProps);

	// 물리적 장치 또는 GPU에서 메모리 속성을 가져옴
	vkGetPhysicalDeviceMemoryProperties(*gpu, &deviceObj->memoryProperties);

	// 물리적 장치의 가용한 큐와 그 속성을 쿼리
	deviceObj->getPhysicalDeviceQueuesAndProperties();

	// 그래픽스 파이프라인을 지원하는 큐를 가져옴
	deviceObj->getGraphicsQueueHandle();

	// 논리적 장치를 생성하고 이를 확인하기 위해 그래픽스 큐와 연결
	deviceObj->createDevice(layers, extensions);

	return VK_SUCCESS;
}

// 시스템의 사용 가능한 물리적 장치 개체 수를 반환한다.
VkResult VKApplication::enumeratePhysicalDevices(std::vector<VkPhysicalDevice>& gpuList)
{
	// GPU의 수 보관
	uint32_t gpuDeviceCount;

	// GPU의 수를 가져옴
	VkResult result = vkEnumeratePhysicalDevices(instanceObj.instance, &gpuDeviceCount, NULL);
	
	// 가져오는 것에 실패하거나 사용 가능한 물리적 장치가 없으면 오류 발생
	assert(result == VK_SUCCESS);
	assert(gpuDeviceCount);

	// 가져오기 위한 공간 확보
	gpuList.resize(gpuDeviceCount);

	// 물리적 장치 개체를 가져옴
	result = vkEnumeratePhysicalDevices(instanceObj.instance, &gpuDeviceCount, gpuList.data());
	assert(result == VK_SUCCESS);

	return result;
}
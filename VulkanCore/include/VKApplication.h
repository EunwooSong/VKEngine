#pragma once
#include "VKInstance.h"
#include "VKDevice.h"
#include "VKLayerAndExtension.h"

class VKApplication
{
private:
	// CTOR: Application constructor responsible for layer enumeration.
	VKApplication();

public:
	// DTOR
	~VKApplication();

public:
	static VKApplication* GetInstance();

	// Simple program life cycle
	void initialize();				// Initialize and allocate resources
	//void prepare();					// Prepare resource
	//void update();					// Update data
	//bool render();					// Render primitives
	void deInitialize();			// Release resources

private:
	// VulkanInstance 생성
	VkResult createVulkanInstance(std::vector<const char*>& layers, std::vector<const char*>& extensions, const char* applicationName);
	
	// handShakeWithDevice?
	// - 논리적 장치 개체, 그 개체와 연관된 큐를 생성한다. 또한, 물리적 장치 속성과 메모리 속성 가져오기 같은 \
	// 추후 응용 프로그램 개발에 필요한 일부 초기화 작업을 수행한다.
	// 
	// gpu : 응용 프로그램이 연결하려는 물리적 장치 
	// layers : GPU에서 활성화해야 하는 레이어의 이름이다.
	// extensions : GPU에서 활성화해야 하는 확장판 이름을 참조한다.
	VkResult handShakeWithDevice(VkPhysicalDevice* gpu, std::vector<const char*>& layers, std::vector<const char*>& extensions);
	
	// 사용 가능한 물리적 장치와의 연결을 설정하려면 응용 프로그램에서 해당 물리적 장치를 열거해야 한다.
	// enumeratePhysicalDevices : 시스템의 사용 가능한 물리적 장치 개체 수를 반환한다.
	VkResult enumeratePhysicalDevices(std::vector<VkPhysicalDevice>& gpus);

public:
	VKInstance  instanceObj;

	VKDevice* deviceObj;
};
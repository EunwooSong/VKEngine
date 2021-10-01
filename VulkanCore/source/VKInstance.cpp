#include "VKInstance.h"

VkResult VKInstance::createInstacne(std::vector<const char*>& layers, std::vector<const char*>& extensions, const char* applicationName) {
	
	// 인스턴스 지정 레이어 및 확장판 정보를 설정
	layerExtension.appRequestedExtensionNames = extensions;
	layerExtension.appRequestedLayerNames = layers;

	// VkApplicationInfo?
	// - 이름, 버전, 엔진 등과 같은 응용 프로그램의 핵심 정보를 제공한다.
	// - Vulkan API 정보를 응용 프로그램에서 사용할 수 있도록 드라이버에 알려준다.
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;					// 제어 구조체의 자료형에 대한 정보를 갖고 있음
	appInfo.pNext = NULL;												// 확장판 지정 구조체에 대한 유효한 포인터를 지정하거나 Null이어야 한다
	appInfo.pApplicationName = applicationName;
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = applicationName;
	appInfo.engineVersion = 1;
	// VK_API_VERSION is now deprecated, use VK_MAKE_VERSION instead.
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	// Vulkan 인스턴스 생성 정보 구조체 정의
	VkInstanceCreateInfo instInfo = {};		
	instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;			// 제어 구조체의 자료형에 대한 정보를 갖고 있음 (VK_STRUCTURE_TYPE_INSTANCED_CREATE_INFO)
	instInfo.pNext = &layerExtension.dbgReportCreateInfo;				// 확장판 지정 구조체에 대한 유효한 포인터를 지정하거나 Null이어야 한다
	instInfo.flags = 0;													// ???
	instInfo.pApplicationInfo = &appInfo;								// 위에서 설정한 응용 프로그램의 지정 정보

	// 활성화할 레이어 이름의 목록 지정
	// - 지정할 목록이 없으면 NULL 포인터를 지정한다.
	instInfo.enabledLayerCount = (uint32_t)layers.size();				// 인스턴스 레벨에서 활성화할 레이어 의 수를 지정한다.
	instInfo.ppEnabledLayerNames = layers.size() ? layers.data() : NULL;// 레이어 이름의 목록을 갖고 있으며, 인스턴스 계층에서 활성화돼야 한다.

	// 활성화할 확장판의 목록 지정
	instInfo.enabledExtensionCount = (uint32_t)extensions.size();						// 인스턴스 계층에서 활성화된 확장판 수를 지정한다.
	instInfo.ppEnabledExtensionNames = extensions.size() ? extensions.data() : NULL;	// 인스턴스 계층에서 활성화된 확장판 이름들의 목록을 배열 형태로 갖고 있다.

	VkResult result = vkCreateInstance(&instInfo, NULL, &instance);						// 인스턴스를 생성한다
	assert(result == VK_SUCCESS);

	return result;
}

void VKInstance::destroyInstance() {
	vkDestroyInstance(instance, NULL);
}
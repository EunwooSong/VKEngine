#include "VKLayerAndExtension.h"
#include "VKApplication.h"

VkResult VulkanLayerAndExtension::getInstanceLayerProperties() { 
	uint32_t						instanceLayerCount;		// 인스턴스 레리어의 수 저장
	std::vector<VkLayerProperties>	layerProperties;		// 레이어 속성을 저장하기 위한 벡터
	VkResult						result;					// Vulkan API 명령의 결과 상태를 점검

	// 모든 레이어를 쿼리
	do {
		//vkEnumerateInstanceLayerProperties(pPropertyCount, pProperties)?
		// - (NULL인수 제공시) 레이어 수를 검색하는 데 사용한다
		// - (데이터 구조의 배열 제공) 정보가 포함된 레이어 배열을 가져오는 데 사용한다
		// pPropertyCount : 전달되는 값에 따리 입력/출력 변수로 작동한다.
		// pProperties : NULL -> 레이어 개수 반환 / 배열 -> 레이어 속성 정보를 반환
		result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, NULL);

		if (result)
			return result;

		if (instanceLayerCount == 0)
			return VK_INCOMPLETE; // 반환 실패

		layerProperties.resize(instanceLayerCount);
		// 레이어 상세한 속성 정보를 가져옴
		result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, layerProperties.data());
	} while (result == VK_INCOMPLETE);

	// 각 레이어에 대한 모든 확장판을 쿼리하고 이를 저장한다
	std::cout << "\nInstanced Layers" << std::endl;
	std::cout << "===================" << std::endl;
	for (auto globalLayerProp : layerProperties) {
		// 레이어 이름과 설명을 출력한다
		std::cout << "\n" << globalLayerProp.description << "\n\t|\n\t|---[Layer Name]--> " << globalLayerProp.layerName << "\n";

		LayerProperties layerProps;
		layerProps.properties = globalLayerProp;

		// 해당 레이어 속성을 위한 인스턴스 레벨 확장판을 가져옴
		result = getExtensionProperties(layerProps);

		if (result) {
			continue;
		}

		layerPropertyList.push_back(layerProps);
		// 각 인스턴스 레이어에 대한 확장판 이름을 출력
		for (auto j : layerProps.extensions) {
			std::cout << "\t\t|\n\t\t|---[Layer Extension]--> " << j.extensionName << "\n";
		}
	}

	return result;
}

// - 확장판과 확장판의 속성을 인스턴스와 장치 레벨에서 가져온다.
// - NULL을 사용해 인스턴스 레벨의 확장판을 특정해 가져온다.
// - gpu를 보대 장치 레벨의 확장을 가져온다.
VkResult VulkanLayerAndExtension::getExtensionProperties(LayerProperties& layerProps, VkPhysicalDevice* gpu) {
	uint32_t	extensionCount;								 // 레이어당 확장판의 수를 저장
	VkResult	result;
	char* layerName = layerProps.properties.layerName; // 레이어 이름

	do {
		// 이 계층의 확장판의 총 수를 가져온다
		if (gpu)	// 장치...
			result = vkEnumerateDeviceExtensionProperties(*gpu, layerName, &extensionCount, NULL);
		else		// 인스턴스...
			result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, NULL);

		if (result || extensionCount == 0)
			continue;

		layerProps.extensions.resize(extensionCount);

		// 모든 확장판의 속성 수집
		if (gpu)
			result = vkEnumerateDeviceExtensionProperties(*gpu, layerName, &extensionCount, layerProps.extensions.data());
		else
			result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, layerProps.extensions.data());
	} while (result == VK_INCOMPLETE);

	return result;
}


VkResult VulkanLayerAndExtension::getDeviceExtensionProperties(VkPhysicalDevice* gpu) {
	// Vulkan API의 결과 상태를 점검하기 위한 변수
	VkResult result;

	// 각 레이어의 모든 확장판을 쿼리하고 이를 저장
	std::cout << "Device extensions" << std::endl;
	std::cout << "===================" << std::endl;

	VKApplication* appObj = VKApplication::GetInstance();
	std::vector<LayerProperties>* instanceLayerProp = &appObj->GetInstance()->instanceObj.layerExtension.layerPropertyList;

	for (auto globalLayerProp : *instanceLayerProp) {
		LayerProperties layerProps;
		layerProps.properties = globalLayerProp.properties;

		if (result = getExtensionProperties(layerProps, gpu))
			continue;

		std::cout << "\n" << globalLayerProp.properties.description << "\n\t|\n\t|---[Layer Name]--> " << globalLayerProp.properties.layerName << "\n";
		layerPropertyList.push_back(layerProps);

		if (layerProps.extensions.size()) {
			for (auto j : layerProps.extensions) {
				std::cout << "\t\t|\n\t\t|---[Device Extesion]--> " << j.extensionName << "\n";
			}
		}
		else {
			std::cout << "\t\t|\n\t\t|---[Device Extesion]--> No extension found \n";
		}
	}
	return result;
}

VkBool32 VulkanLayerAndExtension::areLayersSupported(std::vector<const char*>& layerNames)
{
	uint32_t checkCount = (uint32_t)layerNames.size();
	uint32_t layerCount = (uint32_t)layerPropertyList.size();
		
	std::vector<const char*> unsupportLayerNames;
	for (uint32_t i = 0; i < checkCount; i++) {
		VkBool32 isSupported = 0;
		for (uint32_t j = 0; j < layerCount; j++) {
			if (!strcmp(layerNames[i], layerPropertyList[j].properties.layerName)) {
				isSupported = 1;
			}
		}

		if (!isSupported) {
			std::cout << "No Layer support found, removed from layer: " << layerNames[i] << std::endl;
			unsupportLayerNames.push_back(layerNames[i]);
		}
		else {
			std::cout << "Layer supported: " << layerNames[i] << std::endl;
		}
	}

	for (auto i : unsupportLayerNames) {
		auto it = std::find(layerNames.begin(), layerNames.end(), i);
		if (it != layerNames.end()) layerNames.erase(it);
	}

	return true;
}

VkResult VulkanLayerAndExtension::createDebugReportCallback()
{
	VkResult result;

	VKApplication* appObj = VKApplication::GetInstance();
	VkInstance* instance = &appObj->instanceObj.instance;

	// 디버깅 보고서는 vkCreateDebugReportCallbackEXT API를 사용해 작성한다.
	// 모든 디버깅 관련 API는 vkGetInstanceProcAddr()API로 쿼리하고 동적으로 링크해야 한다.
	// vkGetInstanceProcAddr(vkInstance, name)은 인스턴스 계층 확장판을 동적으로 가져온다. 
	// vkInstance : VKInstance 자료형의 변수.. NULL로 설정시 이름은 반드시 다음 중 하나로 설정해야함.
	// 	   vkEnumerateInstanceExtensionProperties, vkEnumerateInstanceLayerProperties, vkCreateInstance
	// 	name : 동적 링크를 위해 쿼리해야 할 API의 이름이다..
	// vkCreateDebugReportCallbackEXT API 얻기
	dbgCreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(*instance, "vkCreateDebugReportCallbackEXT");
	if (!dbgCreateDebugReportCallback) {
		std::cout << "Error: GetInstanceProcAddr unable to locate vkCreateDebugReportCallbackEXT function." << std::endl;
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	std::cout << "GetInstanceProcAddr loaded dbgCreateDebugReportCallback function\n";

	// vkDestroyDebugReportCallbackEXT API 얻기
	dbgDestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(*instance, "vkDestroyDebugReportCallbackEXT");
	if (!dbgDestroyDebugReportCallback) {
		std::cout << "Error: GetInstanceProcAddr unable to locate vkDestroyDebugReportCallbackEXT function." << std::endl;
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	std::cout << "GetInstanceProcAddr loaded dbgDestroyDebugReportCallback function\n";

	// dbgCreateDebugReportCallback()  함수 포인터로 디버깅 보고서 개체를 생성하고 debugReportCallback에 핸들을 저장한다.
	// VkDebugReportCallbackCreateInfoEXT(type, pNext, flags, fnCallback, pUserData)
	// 	   type : 제어 구조체의 자료형 정보로, 반드시 VK_STRUCTRE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT로 설정해야함
	// 	   flags : 디버깅할 때 가져올 디버깅 정보의 종류를 정의
	// 	   fnCallback 디버깅 메시지를 필터링하고 표시하기 위한 함수를 참조
	// 	   
	// 	   flags...?
	// 	    - VK_DEBUG_REPORT_INFORMATION_BIT_EXT			: 현재 실행중인 응용 프로그램의 백그라운드에서 일어나는 작업을 사용자 친화적인 정보(디버깅에 유용한 리소스의 상세 정보)로 표시
	// 	    - VK_DEBUG_REPORT_WARNING_BIT_EXIT				: 잠재적으로 부정확하거나 위험한 API 사용에 대한 경고 메시지를 제공한다.
	// 	    - VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT	: 잠재적으로 성능에 손실이 있을 것으로 예상되는 Vulkan의 비최적화된 사용을 나타낸다.
	// 	    - VK_DEBUG_REPORT_ERROR_BIT_EXT					: 정의되지 않은 결과(응용의 비정상적인 종료)를 일으킬수 있는 부정확한 API사용을 알려주는 오류 메시지를 참조한다.
	// 	    - VK_DEBUG_REPORT_DEBUG_BIT_EXT					: 로더와 레이어로부터 가져온 진단 정보이다.
	// 디버깅 보고서 제어 구조체를 정의하고, 'debugFunction'의 참조를 제공
	// 이 함수는 콘솔에 디버깅 정보 출력
	dbgReportCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	dbgReportCreateInfo.pfnCallback = debugFunction;
	dbgReportCreateInfo.pUserData = NULL;
	dbgReportCreateInfo.pNext = NULL;
	dbgReportCreateInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_DEBUG_BIT_EXT;

	// 디버깅 보고서의 콜백 함수를 생성하고 핸들을 'debugReportCallback'에 저장
	result = dbgCreateDebugReportCallback(*instance, &dbgReportCreateInfo, NULL, &debugReportCallback);
	if (result == VK_SUCCESS) {
		std::cout << "Debug report callback object created successfully\n";
	}
	return result;
}

void VulkanLayerAndExtension::destroyDebugReportCallback()
{
	VKApplication* appObj = VKApplication::GetInstance();
	VkInstance& instance = appObj->instanceObj.instance;
	dbgDestroyDebugReportCallback(instance, debugReportCallback, NULL);
}

// 보고된 메시지와 함께 디버깅 정보의 유형을 설명한다.
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanLayerAndExtension::debugFunction(VkFlags msgFlags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t srcObject, size_t location,
	int32_t msgCode, const char* layerPrefix,
	const char* msg, void* userData)
{
		if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
			std::cout << "[VK_DEBUG_REPORT] ERROR: [" << layerPrefix << "] Code" << msgCode << ":" << msg << std::endl;
		}
		else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
			std::cout << "[VK_DEBUG_REPORT] WARNING: [" << layerPrefix << "] Code" << msgCode << ":" << msg << std::endl;
		}
		else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
			std::cout << "[VK_DEBUG_REPORT] INFORMATION: [" << layerPrefix << "] Code" << msgCode << ":" << msg << std::endl;
		}
		else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
			std::cout << "[VK_DEBUG_REPORT] PERFORMANCE: [" << layerPrefix << "] Code" << msgCode << ":" << msg << std::endl;
		}
		else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
			std::cout << "[VK_DEBUG_REPORT] DEBUG: [" << layerPrefix << "] Code" << msgCode << ":" << msg << std::endl;
		}
		else {
			return VK_FALSE;
		}

		fflush(stdout);
		return VK_TRUE;
}
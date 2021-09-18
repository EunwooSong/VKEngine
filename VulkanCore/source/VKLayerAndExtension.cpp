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
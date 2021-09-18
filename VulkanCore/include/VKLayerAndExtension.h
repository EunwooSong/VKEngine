/*
1. VKLayerAndExtension
 - 인스턴스와 장치에 대해 레이어와 확장판 기능을 제공한다. 또한 디버깅 기능도 제공한다

 레이어?
 -   레이어는 기존 VulkanAPI에 연결되고, 지정된 레이어와 연결된 Vulkan 명령 체인에 삽입된다.
	레이어는 일반적으로 개발 프로세스를 검증하는 데 사용된다. 예를 들어, 드라이버는 Vulkan API에서 제공한 파라이터를
	확인할 필요가 없다. 들어오는 파라미터가 올바른지를 확인하는 것이 레이어의 역할이다.
		-> 주로 검증을 위해....
 확장판?
 -   확장판은 확장된 기능 또는 특징을 제공한다. 이 기능들은 추후 표준 사양에 포함되거나 포함되지 않을 수 있다.
	확장판은 인스턴스 또는 장치 일부로 구현될 수 있으며, 확장판 명령은 정적으로 링크할 수 없다. 확장판은 먼저 
	쿼리된 후 함수 포인터에 동적으로 링크된다. 등록 확장판인 경우 이 함수 포인터는 필요한 데이터 구조체, 열거형 데이터와 함께
	vulkan.h에 이미 정의돼 있을 수 있다.
		-> API에서 제공하는 확장된 기능... 인스턴스 또는 장치 일부로 구현... 함수 포인터에 동적으로 링크... 등록된 확장판은 이미 정의되어 있음

*/
#pragma once
#include "Headers.h"

struct LayerProperties {
	VkLayerProperties properties;
	std::vector<VkExtensionProperties> extensions;
};

class VulkanLayerAndExtension {
public:
	VulkanLayerAndExtension() {};
	~VulkanLayerAndExtension() {};

	/******* LAYER AND EXTENSION MEMBER FUNCTION AND VARAIBLES *******/

	// List of layer names requested by the application.
	std::vector<const char*>			appRequestedLayerNames;
	// List of extension names requested by the application.
	std::vector<const char*>			appRequestedExtensionNames;
	// Layers and corresponding extension list
	std::vector<LayerProperties>		layerPropertyList;

	// getInstanceLayerProperties?
	// - 인스턴스 또는 전역 레이어를 쿼리한다.
	// - 레이어의 총 개수를 가져와 모든 레이어 정보를 layerProperties라는 VkLayerProperties 벡터에 저장한다.
	VkResult getInstanceLayerProperties();

	// getExtensionProperties?
	// - 각 레이어는 하나 이상의 확장판을 지원할 수 있다...
	// - 먼저 제공되는 확장판의 수를 얻기 위해 호출된다.
	VkResult getExtensionProperties(LayerProperties& layerProps, VkPhysicalDevice* gpu = NULL);

	// 장치 기반 확장판
	VkResult getDeviceExtensionProperties(VkPhysicalDevice* gpu);
};
/*
2. VkInstance
 - 응용 프로그램을 빌드하는 데 필요한 기본 개체로, 모든 응용 프로그램의 스테이트를 저장한다.
 - Vulkan 인스턴스 개체의 생서와 제거를 담당한다.
*/

#pragma once
#include "VKLayerAndExtension.h"

class VKInstance {
public:
	VKInstance() {}
	~VKInstance() {}

	VkInstance instance; //Vulkan 인스턴스 개체 변수
	VulkanLayerAndExtension layerExtension; //Vulkan 인스턴스 지정 레이어 및 확장판
	
	// Vulkan 인스턴스의 생성 및 삭제를 위한 함수
	VkResult createInstacne(
		std::vector<const char*>& layers,
		std::vector<const char*>& extensions,
		const char* applicationName);

	// Vulkan 인스턴스 삭제
	void destroyInstance();
	
};
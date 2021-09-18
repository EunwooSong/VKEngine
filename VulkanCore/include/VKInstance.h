/*
2. VkInstance
 - ���� ���α׷��� �����ϴ� �� �ʿ��� �⺻ ��ü��, ��� ���� ���α׷��� ������Ʈ�� �����Ѵ�.
 - Vulkan �ν��Ͻ� ��ü�� ������ ���Ÿ� ����Ѵ�.
*/

#pragma once
#include "VKLayerAndExtension.h"

class VKInstance {
public:
	VKInstance() {}
	~VKInstance() {}

	VkInstance instance; //Vulkan �ν��Ͻ� ��ü ����
	VulkanLayerAndExtension layerExtension; //Vulkan �ν��Ͻ� ���� ���̾� �� Ȯ����
	
	// Vulkan �ν��Ͻ��� ���� �� ������ ���� �Լ�
	VkResult createInstacne(
		std::vector<const char*>& layers,
		std::vector<const char*>& extensions,
		const char* applicationName);

	// Vulkan �ν��Ͻ� ����
	void destroyInstance();
	
};
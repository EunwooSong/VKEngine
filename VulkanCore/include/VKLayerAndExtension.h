/*
1. VKLayerAndExtension
 - �ν��Ͻ��� ��ġ�� ���� ���̾�� Ȯ���� ����� �����Ѵ�. ���� ����� ��ɵ� �����Ѵ�

 ���̾�?
 -   ���̾�� ���� VulkanAPI�� ����ǰ�, ������ ���̾�� ����� Vulkan ��� ü�ο� ���Եȴ�.
	���̾�� �Ϲ������� ���� ���μ����� �����ϴ� �� ���ȴ�. ���� ���, ����̹��� Vulkan API���� ������ �Ķ����͸�
	Ȯ���� �ʿ䰡 ����. ������ �Ķ���Ͱ� �ùٸ����� Ȯ���ϴ� ���� ���̾��� �����̴�.
		-> �ַ� ������ ����....
 Ȯ����?
 -   Ȯ������ Ȯ��� ��� �Ǵ� Ư¡�� �����Ѵ�. �� ��ɵ��� ���� ǥ�� ��翡 ���Եǰų� ���Ե��� ���� �� �ִ�.
	Ȯ������ �ν��Ͻ� �Ǵ� ��ġ �Ϻη� ������ �� ������, Ȯ���� ����� �������� ��ũ�� �� ����. Ȯ������ ���� 
	������ �� �Լ� �����Ϳ� �������� ��ũ�ȴ�. ��� Ȯ������ ��� �� �Լ� �����ʹ� �ʿ��� ������ ����ü, ������ �����Ϳ� �Բ�
	vulkan.h�� �̹� ���ǵ� ���� �� �ִ�.
		-> API���� �����ϴ� Ȯ��� ���... �ν��Ͻ� �Ǵ� ��ġ �Ϻη� ����... �Լ� �����Ϳ� �������� ��ũ... ��ϵ� Ȯ������ �̹� ���ǵǾ� ����

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
	// - �ν��Ͻ� �Ǵ� ���� ���̾ �����Ѵ�.
	// - ���̾��� �� ������ ������ ��� ���̾� ������ layerProperties��� VkLayerProperties ���Ϳ� �����Ѵ�.
	VkResult getInstanceLayerProperties();

	// getExtensionProperties?
	// - �� ���̾�� �ϳ� �̻��� Ȯ������ ������ �� �ִ�...
	// - ���� �����Ǵ� Ȯ������ ���� ��� ���� ȣ��ȴ�.
	VkResult getExtensionProperties(LayerProperties& layerProps, VkPhysicalDevice* gpu = NULL);

	// ��ġ ��� Ȯ����
	VkResult getDeviceExtensionProperties(VkPhysicalDevice* gpu);
};
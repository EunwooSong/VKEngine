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

 �����?
 -	 ��ȿ�� ���� ������ ����� ���� ���������� ����� �� �ְ� �� -> ���� ���ӿ����� ������� ����, ��Ÿ�Ӷ� �ý��ۿ� ����
	Vulkan ������� ���� ���α׷� �������� �������� �ƴ϶� API�� ������ ����ߴ��� �����ϴ� �� ��ȿ�� ���� ��ɵ� ���� �ִ�.
	���޵� �� �Ķ���� ������ Ȯ���ϰ� ���������� ����Ȯ�ϰų� ���輺�� �ִ� API ����� �߰ߵǸ� ����ϰ� �������� ������ �ʾƵ� ��� �����ش� \
		-> Vulkan ������� API�� ����, ������, ���輺 ���� �����ϰ� ��� �� ������ �˷���...

 -	 ������� Ȱ��ȭ�Ǹ� ������ Vulkan ��ɿ� ���� ȣ�� ü�ο� �ڱ� �ڽ��� �����Ѵ�. �� ��ɿ� ���� ����� ���̾��
	Ȱ��ȭ�� ��� ���̾ �湮�ϰ�, ��ȿ���� ������ �������� ����, ���, ����� ���� ���� �����Ѵ�.
		-> Vulkan ��ɿ� ���� ȣ�� ü�ο� �ڱ� �ڽ��� ���� -> Ȱ��ȭ�� ��� ���̾� �湮 -> ��ȿ�� ����

 - Ȱ��ȭ �ܰ�
	1. �ν��Ͻ� �������� VK_EXT_DEBUG_REPORT_EXTENSION_NAME Ȯ���� �߰��� ����� ��� Ȱ��ȭ
	2. ������� ���� ��ȿ�� ���� ���̾ ����
		EX) VK_LAYTER_GOOGLE_unique_objects, VK_LAYER_LUNARG_api_dump etc...
	3. Vulkan ����� API�� �������� �δ��� ���� �ε��Ǵ� �ھ� ����� �ƴϴ�.
	  ����� API�� ��Ÿ�ӿ� �˻��� �̸� ���ǵ� �Լ� �����Ϳ� �������� ��ũ�� �� �ִ� Ȯ���� API ���·� ����� �� �ִ�.
	  ����� Ȯ���� API�� vkCreateDebugReportCallbackEXT �� vkDestroyDebugReportCallbackEXT�� �����ϰ� �������� �����Ѵ�.
		-> ����� API�� �������� �δ��� ���� �ε� X, �������� ��ũ�� �� �ִ� Ȯ���� API ���·� ���
	4. ����� ������ ���� �Լ� �����͸� ���������� �������� ���� API�� ����� ���� ��ü�� �����Ѵ�.
	  Vulkan�� �� API�� ����� ����� ���� �ݹ� �Լ��� ����� ������ ��ȯ�Ѵ�.
		-> ����� ���� �Լ� �����͸� �������� ���� ��ü ����, ��ȯ
	5. ������� ���� �ʿ����� ������ ����� ���� ��ü�� ������

 - ������� ����
	1. Ȯ������ �ν��Ͻ� ������ �߰��Ѵ�.
	2. ���̾� �ν��Ͻ� �������� ���� ���̾ �����ϰ� �� ���̾�� ������� �� �ִ�.
		+ LunarG SDK�� ����� �� �ִ� ���� ���̾�鿡 �߰��� VK_LAYER_LUNARG_standard_validation�̶�� Ư���� ���̾ �����Ѵ�.
		 �� ���̾�� ������ ������ ������ �⺻ ������ ����� �� �ְ� �Ѵ�.
		 ���� �� ��Ʈ�� ��Ÿ ������ ���̾�� ���� ���̾��� ǥ�� ��Ʈ�� ������ ������ �ε����ش�
		 ���̾ ����ϴ� ��Ȯ�� ������ ������ ���� �ʴٸ� ���� ���� ����� �����Ѵ�.
		a) "VK_LAYER_GOOGLE_threading",
		b) "VK_LAYER_LUNARG_parameter_validation",
		c) "VK_LAYER_LUNARG_object_tracker",
		d) "VK_LAYER_LUNARG_image",
		e) "VK_LAYER_LUNARG_core_validation",
		f) "VK_LAYER_LUNARG_swapchain",
		g) "VK_LAYER_GOOGLE_unique_objects"
			-> VK_LAYER_LUNARG_standard_validation ����, ������ ������ ���̾� �ε�, ��Ȯ�� ������ ���ٸ� ���� ���� ��� ����
			+ vkCreateInstance() API�� ������ Ȱ��ȭ ���� (VKInstance)
	3. VulkanLayerAndExtension Ŭ�������� �����Ѵ�.
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

	/******* VULKAN DEBUGGING MEMBER FUNCTION AND VARAIBLES *******/

	// ��ȿ�� ���� ���׾�� ���� ��ü, SDK ������ ���� �ſ� �ٸ�, ���̾ �⺻ �������� �����Ǵ��� ���� Ȯ���ϴ� ���� �ʿ�..
	// �ý��ۿ��� �����ϴ� ���̾����� ����!
	VkBool32 areLayersSupported(std::vector<const char*>& layerNames);

	
	// �� API�� Vulkan�� �⺻ ����� �ƴ϶� �������� ��ũ���� �ʴ´�...
	// ��� ����� ���� API�� vkGetInstanceProcAddr()API�� �����ϰ� �������� ��ũ�ؾ� �Ѵ�.
	VkResult createDebugReportCallback();
	void	destroyDebugReportCallback();
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugFunction(VkFlags msgFlags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t srcObject,
		size_t location,
		int32_t msgCode,
		const char* layerPrefix,
		const char* msg,
		void* userData);

private:
	//������ ���� �Լ� ������ ����
	PFN_vkCreateDebugReportCallbackEXT dbgCreateDebugReportCallback;
	PFN_vkDestroyDebugReportCallbackEXT dbgDestroyDebugReportCallback;
	
	// ����� ���� �ݹ� �Լ��� �ڵ�
	VkDebugReportCallbackEXT debugReportCallback;
	
public:
	// ����� ������ �ݹ� �Լ��� ���� ���� ����ü�� ������
	VkDebugReportCallbackCreateInfoEXT dbgReportCreateInfo = {};
};
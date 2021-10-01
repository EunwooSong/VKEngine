#include "VKInstance.h"

VkResult VKInstance::createInstacne(std::vector<const char*>& layers, std::vector<const char*>& extensions, const char* applicationName) {
	
	// �ν��Ͻ� ���� ���̾� �� Ȯ���� ������ ����
	layerExtension.appRequestedExtensionNames = extensions;
	layerExtension.appRequestedLayerNames = layers;

	// VkApplicationInfo?
	// - �̸�, ����, ���� ��� ���� ���� ���α׷��� �ٽ� ������ �����Ѵ�.
	// - Vulkan API ������ ���� ���α׷����� ����� �� �ֵ��� ����̹��� �˷��ش�.
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;					// ���� ����ü�� �ڷ����� ���� ������ ���� ����
	appInfo.pNext = NULL;												// Ȯ���� ���� ����ü�� ���� ��ȿ�� �����͸� �����ϰų� Null�̾�� �Ѵ�
	appInfo.pApplicationName = applicationName;
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = applicationName;
	appInfo.engineVersion = 1;
	// VK_API_VERSION is now deprecated, use VK_MAKE_VERSION instead.
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	// Vulkan �ν��Ͻ� ���� ���� ����ü ����
	VkInstanceCreateInfo instInfo = {};		
	instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;			// ���� ����ü�� �ڷ����� ���� ������ ���� ���� (VK_STRUCTURE_TYPE_INSTANCED_CREATE_INFO)
	instInfo.pNext = &layerExtension.dbgReportCreateInfo;				// Ȯ���� ���� ����ü�� ���� ��ȿ�� �����͸� �����ϰų� Null�̾�� �Ѵ�
	instInfo.flags = 0;													// ???
	instInfo.pApplicationInfo = &appInfo;								// ������ ������ ���� ���α׷��� ���� ����

	// Ȱ��ȭ�� ���̾� �̸��� ��� ����
	// - ������ ����� ������ NULL �����͸� �����Ѵ�.
	instInfo.enabledLayerCount = (uint32_t)layers.size();				// �ν��Ͻ� �������� Ȱ��ȭ�� ���̾� �� ���� �����Ѵ�.
	instInfo.ppEnabledLayerNames = layers.size() ? layers.data() : NULL;// ���̾� �̸��� ����� ���� ������, �ν��Ͻ� �������� Ȱ��ȭ�ž� �Ѵ�.

	// Ȱ��ȭ�� Ȯ������ ��� ����
	instInfo.enabledExtensionCount = (uint32_t)extensions.size();						// �ν��Ͻ� �������� Ȱ��ȭ�� Ȯ���� ���� �����Ѵ�.
	instInfo.ppEnabledExtensionNames = extensions.size() ? extensions.data() : NULL;	// �ν��Ͻ� �������� Ȱ��ȭ�� Ȯ���� �̸����� ����� �迭 ���·� ���� �ִ�.

	VkResult result = vkCreateInstance(&instInfo, NULL, &instance);						// �ν��Ͻ��� �����Ѵ�
	assert(result == VK_SUCCESS);

	return result;
}

void VKInstance::destroyInstance() {
	vkDestroyInstance(instance, NULL);
}
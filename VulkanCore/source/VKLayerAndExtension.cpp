#include "VKLayerAndExtension.h"
#include "VKApplication.h"

VkResult VulkanLayerAndExtension::getInstanceLayerProperties() { 
	uint32_t						instanceLayerCount;		// �ν��Ͻ� �������� �� ����
	std::vector<VkLayerProperties>	layerProperties;		// ���̾� �Ӽ��� �����ϱ� ���� ����
	VkResult						result;					// Vulkan API ����� ��� ���¸� ����

	// ��� ���̾ ����
	do {
		//vkEnumerateInstanceLayerProperties(pPropertyCount, pProperties)?
		// - (NULL�μ� ������) ���̾� ���� �˻��ϴ� �� ����Ѵ�
		// - (������ ������ �迭 ����) ������ ���Ե� ���̾� �迭�� �������� �� ����Ѵ�
		// pPropertyCount : ���޵Ǵ� ���� ���� �Է�/��� ������ �۵��Ѵ�.
		// pProperties : NULL -> ���̾� ���� ��ȯ / �迭 -> ���̾� �Ӽ� ������ ��ȯ
		result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, NULL);

		if (result)
			return result;

		if (instanceLayerCount == 0)
			return VK_INCOMPLETE; // ��ȯ ����

		layerProperties.resize(instanceLayerCount);
		// ���̾� ���� �Ӽ� ������ ������
		result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, layerProperties.data());
	} while (result == VK_INCOMPLETE);

	// �� ���̾ ���� ��� Ȯ������ �����ϰ� �̸� �����Ѵ�
	std::cout << "\nInstanced Layers" << std::endl;
	std::cout << "===================" << std::endl;
	for (auto globalLayerProp : layerProperties) {
		// ���̾� �̸��� ������ ����Ѵ�
		std::cout << "\n" << globalLayerProp.description << "\n\t|\n\t|---[Layer Name]--> " << globalLayerProp.layerName << "\n";

		LayerProperties layerProps;
		layerProps.properties = globalLayerProp;

		// �ش� ���̾� �Ӽ��� ���� �ν��Ͻ� ���� Ȯ������ ������
		result = getExtensionProperties(layerProps);

		if (result) {
			continue;
		}

		layerPropertyList.push_back(layerProps);
		// �� �ν��Ͻ� ���̾ ���� Ȯ���� �̸��� ���
		for (auto j : layerProps.extensions) {
			std::cout << "\t\t|\n\t\t|---[Layer Extension]--> " << j.extensionName << "\n";
		}
	}

	return result;
}

// - Ȯ���ǰ� Ȯ������ �Ӽ��� �ν��Ͻ��� ��ġ �������� �����´�.
// - NULL�� ����� �ν��Ͻ� ������ Ȯ������ Ư���� �����´�.
// - gpu�� ���� ��ġ ������ Ȯ���� �����´�.
VkResult VulkanLayerAndExtension::getExtensionProperties(LayerProperties& layerProps, VkPhysicalDevice* gpu) {
	uint32_t	extensionCount;								 // ���̾�� Ȯ������ ���� ����
	VkResult	result;
	char* layerName = layerProps.properties.layerName; // ���̾� �̸�

	do {
		// �� ������ Ȯ������ �� ���� �����´�
		if (gpu)	// ��ġ...
			result = vkEnumerateDeviceExtensionProperties(*gpu, layerName, &extensionCount, NULL);
		else		// �ν��Ͻ�...
			result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, NULL);

		if (result || extensionCount == 0)
			continue;

		layerProps.extensions.resize(extensionCount);

		// ��� Ȯ������ �Ӽ� ����
		if (gpu)
			result = vkEnumerateDeviceExtensionProperties(*gpu, layerName, &extensionCount, layerProps.extensions.data());
		else
			result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, layerProps.extensions.data());
	} while (result == VK_INCOMPLETE);

	return result;
}


VkResult VulkanLayerAndExtension::getDeviceExtensionProperties(VkPhysicalDevice* gpu) {
	// Vulkan API�� ��� ���¸� �����ϱ� ���� ����
	VkResult result;

	// �� ���̾��� ��� Ȯ������ �����ϰ� �̸� ����
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
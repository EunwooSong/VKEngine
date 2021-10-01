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

	// ����� ������ vkCreateDebugReportCallbackEXT API�� ����� �ۼ��Ѵ�.
	// ��� ����� ���� API�� vkGetInstanceProcAddr()API�� �����ϰ� �������� ��ũ�ؾ� �Ѵ�.
	// vkGetInstanceProcAddr(vkInstance, name)�� �ν��Ͻ� ���� Ȯ������ �������� �����´�. 
	// vkInstance : VKInstance �ڷ����� ����.. NULL�� ������ �̸��� �ݵ�� ���� �� �ϳ��� �����ؾ���.
	// 	   vkEnumerateInstanceExtensionProperties, vkEnumerateInstanceLayerProperties, vkCreateInstance
	// 	name : ���� ��ũ�� ���� �����ؾ� �� API�� �̸��̴�..
	// vkCreateDebugReportCallbackEXT API ���
	dbgCreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(*instance, "vkCreateDebugReportCallbackEXT");
	if (!dbgCreateDebugReportCallback) {
		std::cout << "Error: GetInstanceProcAddr unable to locate vkCreateDebugReportCallbackEXT function." << std::endl;
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	std::cout << "GetInstanceProcAddr loaded dbgCreateDebugReportCallback function\n";

	// vkDestroyDebugReportCallbackEXT API ���
	dbgDestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(*instance, "vkDestroyDebugReportCallbackEXT");
	if (!dbgDestroyDebugReportCallback) {
		std::cout << "Error: GetInstanceProcAddr unable to locate vkDestroyDebugReportCallbackEXT function." << std::endl;
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	std::cout << "GetInstanceProcAddr loaded dbgDestroyDebugReportCallback function\n";

	// dbgCreateDebugReportCallback()  �Լ� �����ͷ� ����� ���� ��ü�� �����ϰ� debugReportCallback�� �ڵ��� �����Ѵ�.
	// VkDebugReportCallbackCreateInfoEXT(type, pNext, flags, fnCallback, pUserData)
	// 	   type : ���� ����ü�� �ڷ��� ������, �ݵ�� VK_STRUCTRE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT�� �����ؾ���
	// 	   flags : ������� �� ������ ����� ������ ������ ����
	// 	   fnCallback ����� �޽����� ���͸��ϰ� ǥ���ϱ� ���� �Լ��� ����
	// 	   
	// 	   flags...?
	// 	    - VK_DEBUG_REPORT_INFORMATION_BIT_EXT			: ���� �������� ���� ���α׷��� ��׶��忡�� �Ͼ�� �۾��� ����� ģȭ���� ����(����뿡 ������ ���ҽ��� �� ����)�� ǥ��
	// 	    - VK_DEBUG_REPORT_WARNING_BIT_EXIT				: ���������� ����Ȯ�ϰų� ������ API ��뿡 ���� ��� �޽����� �����Ѵ�.
	// 	    - VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT	: ���������� ���ɿ� �ս��� ���� ������ ����Ǵ� Vulkan�� ������ȭ�� ����� ��Ÿ����.
	// 	    - VK_DEBUG_REPORT_ERROR_BIT_EXT					: ���ǵ��� ���� ���(������ ���������� ����)�� ����ų�� �ִ� ����Ȯ�� API����� �˷��ִ� ���� �޽����� �����Ѵ�.
	// 	    - VK_DEBUG_REPORT_DEBUG_BIT_EXT					: �δ��� ���̾�κ��� ������ ���� �����̴�.
	// ����� ���� ���� ����ü�� �����ϰ�, 'debugFunction'�� ������ ����
	// �� �Լ��� �ֿܼ� ����� ���� ���
	dbgReportCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	dbgReportCreateInfo.pfnCallback = debugFunction;
	dbgReportCreateInfo.pUserData = NULL;
	dbgReportCreateInfo.pNext = NULL;
	dbgReportCreateInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_DEBUG_BIT_EXT;

	// ����� ������ �ݹ� �Լ��� �����ϰ� �ڵ��� 'debugReportCallback'�� ����
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

// ����� �޽����� �Բ� ����� ������ ������ �����Ѵ�.
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
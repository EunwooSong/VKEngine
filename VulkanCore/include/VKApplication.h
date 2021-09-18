#pragma once
#include "VKInstance.h"
#include "VKDevice.h"
#include "VKLayerAndExtension.h"

class VKApplication
{
private:
	// CTOR: Application constructor responsible for layer enumeration.
	VKApplication();

public:
	// DTOR
	~VKApplication();

public:
	static VKApplication* GetInstance();

	// Simple program life cycle
	void initialize();				// Initialize and allocate resources
	//void prepare();					// Prepare resource
	//void update();					// Update data
	//bool render();					// Render primitives
	void deInitialize();			// Release resources

private:
	// VulkanInstance ����
	VkResult createVulkanInstance(std::vector<const char*>& layers, std::vector<const char*>& extensions, const char* applicationName);
	
	// handShakeWithDevice?
	// - ���� ��ġ ��ü, �� ��ü�� ������ ť�� �����Ѵ�. ����, ������ ��ġ �Ӽ��� �޸� �Ӽ� �������� ���� \
	// ���� ���� ���α׷� ���߿� �ʿ��� �Ϻ� �ʱ�ȭ �۾��� �����Ѵ�.
	// 
	// gpu : ���� ���α׷��� �����Ϸ��� ������ ��ġ 
	// layers : GPU���� Ȱ��ȭ�ؾ� �ϴ� ���̾��� �̸��̴�.
	// extensions : GPU���� Ȱ��ȭ�ؾ� �ϴ� Ȯ���� �̸��� �����Ѵ�.
	VkResult handShakeWithDevice(VkPhysicalDevice* gpu, std::vector<const char*>& layers, std::vector<const char*>& extensions);
	
	// ��� ������ ������ ��ġ���� ������ �����Ϸ��� ���� ���α׷����� �ش� ������ ��ġ�� �����ؾ� �Ѵ�.
	// enumeratePhysicalDevices : �ý����� ��� ������ ������ ��ġ ��ü ���� ��ȯ�Ѵ�.
	VkResult enumeratePhysicalDevices(std::vector<VkPhysicalDevice>& gpus);

public:
	VKInstance  instanceObj;

	VKDevice* deviceObj;
};
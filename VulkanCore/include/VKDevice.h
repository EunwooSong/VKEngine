/* 3. VulkanDevice
 - Vulkan�� �ý����� ���� ��ǻ�� ��ġ �Ǵ� GPU�� ���� ���α׷��� ���������� ������
 ���� ���α׷��� �ý��ۿ��� ��� ������ ���� ��ġ�� ������ �� �ִ�

 * Vulkan�� ��ġ�� ǥ���� ������ ��ġ�� ���� ��ġ�� �˷��� �� ���� �������� �����Ѵ�.
 1) ������ ��ġ
	- ������ ��ġ�� �ϳ��� �۾� �ɷ��� ��Ÿ����. �ý����� ����� �۾��� �����ϴ� ��
	������ �ִ� �ٸ� �ϵ���� ��ǰ�� �Բ� ���� GPU�� �����ȴ�. �ſ� ������ �ý��ۿ��� ������ ��ġ�� 
	���� �������� GPU ��ġ�� ������ �� �ִ�.
		-> ���� �������� GPU

 2) ���� ��ġ
	- ���� ��ġ�� ���� ���α׷� �並 ��Ÿ����.
	- ���� ���α׷� �������� ����ϰ� ������ ��ü�� ���� Ư���� �並 �����Ѵ�.
	EX) ������ ��ġ�� �׷��Ƚ��� ��ǻ��, ������ �� ���� ť�� ���� �� �� ����
		-> ���� ��ġ�� ���� ť�� ÷�ε� ������ �� ����
		-> �̷��� �ϸ� Ŀ�ǵ� ���۸� �ſ� ���� ������ �� �ְ� ��

 ������ ��ġ�� �Ӽ� �������� : vkGetPhysicalDeviceProperties();
 ������ ��ġ���� �޸� �Ӽ� Ȯ�� : vkGetPhyzicalDeviceMemoryProperties();

 * ȣ��Ʈ���� ����ϱ�
	- ť�� ��ġ�� ó���ؾ� �� �۾��� �ִ� ���� : Ȱ�� ����(Active) -> ������ ���� ����..
	- vkDeviceWaitIdle API�� ���� ��ġ�� ��� ť�� ���� ���°� �� ������ ȣ��Ʈ���� ����Ѵ�.
		-> �� API�� ���� ���¸� ������ ���� ��ġ ��ü�� �ڵ��� �Է� �ʵ�� ����Ѵ�.

 * ��ġ ����
	- ���� ��ġ�� ������ ��ġ�� �۾��ϴ� ���� �ϵ���� ���۵�, ��ġ ����, ���� �ð� �ʰ� ���� ������ ��ġ�� ���� �� �� �ִ�.
	
	+ ������ ��ġ�� ������ ���¿��� ���� ��ġ ��ü�� ������ �õ��ϸ� �����ϰԵǰ� 'VK_ERROR_DEVICE_LOST'�� ��ȯ�ȴ�.

 * ť�� ť �йи� �����ϱ�
	ť(Queue)?
		- ���� ���α׷��� ������ ��ġ�� ����ϴ� �����̴�. ���� ���α׷��� ť�� Ŀ�ǵ� ���۸� �����ϴ� �������� ���ϴ� �۾��� ��Ű��,
		�� �۾��� ������ ��ġ���о� �鿩 �񵿱������� ó���ȴ�. ������ ��ġ���� �� ���� ������ ť�� �����ϰ�, ���� ���α׷���
		�ʿ信 ���� �� ť�� ���� ť�� ������ ������ �� �ִ�.
			-> ť? ���� ���α׷��� ������ ��ġ�� ����ϴ� ����... �� ���� ������ ť ����... ���� ���α׷��� ť�� ���� ���� ���� ����\

		EX) ���� ���α׷��� ��ǻ�ð� �׷��Ƚ� �� ���� ť ������ ������ ù ��° ť�� �̹��� ������ǿ��꿡 �̿��ϰ�, �� ��° ť��
		���� �̹��� ������ ��꿡 ����� �� �ִ�.
		- �� ���� ������ ť : �׷��Ƚ� ť, ��ǻ�� ť, ���� ť, ��� ť
		- ������ ��ġ�� ������ ť �йи� ���ο� �����ϴ� ť ������ ������ �ϳ� �̻��� ť �йи��� �����ȴ�.
		����, �� ť �йи����� �ϳ� �̻��� ť ī��Ʈ�� ���� �� �ִ�.

 * ť ����
	- ť�� vkCreateDevice() API�� ����� ���� ��ġ ��ü�� ������ �� ���������� �����ȴ�.
	- vkGetDeviceQueue() API�� ������ ť�� �˻��� �� �ִ�.
*/

#pragma once

#include "Headers.h"
#include "VKLayerAndExtension.h"

class VKApplication;

class VKDevice {
public:
	VKDevice(VkPhysicalDevice* gpu);
	~VKDevice();

public:
	VkDevice							device;		// ���� ��ġ
	VkPhysicalDevice*					gpu;		// ������ ��ġ
	VkPhysicalDeviceProperties			gpuProps;	// ������ ��ġ �Ӽ�
	VkPhysicalDeviceMemoryProperties	memoryProperties;

public:
	// Queue
	VkQueue									queue;							// ť ��ü
	std::vector<VkQueueFamilyProperties>	queueFamilyProps;				// ������ ��ġ�� ���� ������ ��� ť �йи��� ����
	uint32_t								graphicsQueueIndex;				// �׷��Ƚ� ť�� �ε����� ����
	uint32_t								graphicsQueueWithPresentIndex;  // ��ġ�� ���� ������ ť �йи��� ��
	uint32_t								queueFamilyCount;				// ��ġ ���� Ȯ����

	// ��ġ ���� Ȯ����
	VulkanLayerAndExtension		layerExtension;

public:
	VkResult createDevice(std::vector<const char*>& layers, std::vector<const char*>& extensions);
	void destroyDevice();

	// �޸� ���� ����, �̹����� �޸𸮸� �Ҵ��ϴ� �� ������ �޸� ������ ������
	bool memoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex);

	// ������ ��ġ�� ť�� �Ӽ��� ������
	void getPhysicalDeviceQueuesAndProperties();

	// ���� ��ġ ��ü�� �����Ϸ��� ��ȿ�� ť �ڵ��� �ʿ��ϸ�, �̸� �̿��� ������ ť�� �����ϰ� �ȴ�.
	// VkQueueFamilyProperties::queueFlags ��Ʈ ������ Ȯ���� �ش� ť�� ã�´�.
	// �츮�� �׷��Ƚ� ť�� �����ð���
	uint32_t getGraphicsQueueHandle();

	// ť�� ���õ� ��� �Լ���
	void getDeviceQueue();
};
/* 3. VulkanDevice
 - Vulkan은 시스템의 실제 컴퓨팅 장치 또는 GPU를 응용 프로그램에 공개적으로 제공해
 응용 프로그램에 시스템에서 사용 가능한 실제 장치를 열거할 수 있다

 * Vulkan은 장치의 표현을 물리적 장치와 논리적 장치로 알려진 두 가지 형식으로 구분한다.
 1) 물리적 장치
	- 물리적 장치는 하나의 작업 능력을 나타낸다. 시스템은 제출된 작업을 수행하는 데
	도움을 주는 다른 하드웨어 부품과 함께 단일 GPU로 구성된다. 매우 간단한 시스템에서 물리적 장치는 
	실제 물리적인 GPU 장치로 간주할 수 있다.
		-> 실제 물리적인 GPU

 2) 논리적 장치
	- 실제 장치의 응용 프로그램 뷰를 나타낸다.
	- 응용 프로그램 공간에서 사용하고 물리적 장체에 대힌 특수한 뷰를 제공한다.
	EX) 물리적 장치는 그래픽스와 컴퓨팅, 전송의 세 가지 큐로 구성 될 수 있음
		-> 논리적 장치는 단일 큐가 첨부돼 생성될 수 있음
		-> 이렇게 하면 커맨드 버퍼를 매우 쉽게 제출할 수 있게 됨

 물리적 장치의 속성 가져오기 : vkGetPhysicalDeviceProperties();
 물리적 장치에서 메모리 속성 확인 : vkGetPhyzicalDeviceMemoryProperties();

 * 호스트에서 대기하기
	- 큐에 장치가 처리해야 할 작업이 있는 상태 : 활성 상태(Active) -> 없으면 유휴 상태..
	- vkDeviceWaitIdle API는 논리적 장치의 모든 큐가 유휴 상태가 될 때까지 호스트에서 대기한다.
		-> 이 API는 유휴 상태를 점검할 논리적 장치 개체의 핸들을 입력 필드로 사용한다.

 * 장치 끊김
	- 논리적 장치와 물리적 장치로 작업하는 도중 하드웨어 오작동, 장치 오류, 실행 시간 초과 등의 이유로 장치가 끊어 질 수 있다.
	
	+ 물리적 장치가 끊어진 상태에서 논리적 장치 개체의 생성을 시도하면 실패하게되고 'VK_ERROR_DEVICE_LOST'가 반환된다.

 * 큐와 큐 패밀리 이해하기
	큐(Queue)?
		- 응용 프로그램과 물리적 장치가 통신하는 수단이다. 응용 프로그램은 큐에 커맨드 버퍼를 제출하는 형식으로 원하는 작업을 시키고,
		이 작업을 물리적 장치가읽어 들여 비동기적으로 처리된다. 물리적 장치에는 네 가지 유형의 큐를 지원하고, 응용 프로그램은
		필요에 따라 이 큐의 수와 큐의 유형을 선택할 수 있다.
			-> 큐? 응용 프로그램과 물리적 장치가 통신하는 수단... 네 가지 유형의 큐 지원... 응용 프로그램은 큐의 수와 유형 선택 가능\

		EX) 응용 프로그램이 컴퓨팅과 그래픽스 두 개의 큐 유형을 선택해 첫 번째 큐는 이미지 컨볼루션연산에 이용하고, 두 번째 큐는
		블러링 이미지 렌더링 계산에 사용할 수 있다.
		- 네 가지 유형의 큐 : 그래픽스 큐, 컴퓨팅 큐, 전송 큐, 희소 큐
		- 물리적 장치는 각각의 큐 패밀리 내부에 존재하는 큐 유형이 공개된 하나 이상의 큐 패밀리로 구성된다.
		또한, 각 큐 패밀리에는 하나 이상의 큐 카운트가 있을 수 있다.

 * 큐 생성
	- 큐는 vkCreateDevice() API를 사용해 논리적 장치 개체가 생성될 때 묵시적으로 생성된다.
	- vkGetDeviceQueue() API로 생성된 큐를 검색할 수 있다.
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
	VkDevice							device;		// 논리적 장치
	VkPhysicalDevice*					gpu;		// 물리적 장치
	VkPhysicalDeviceProperties			gpuProps;	// 물리적 장치 속성
	VkPhysicalDeviceMemoryProperties	memoryProperties;

public:
	// Queue
	VkQueue									queue;							// 큐 개체
	std::vector<VkQueueFamilyProperties>	queueFamilyProps;				// 물리적 장치에 의해 공개된 모든 큐 패밀리를 저장
	uint32_t								graphicsQueueIndex;				// 그래픽스 큐의 인덱스를 저장
	uint32_t								graphicsQueueWithPresentIndex;  // 장치에 의해 공개된 큐 패밀리의 수
	uint32_t								queueFamilyCount;				// 장치 지정 확장판

	// 장치 지정 확장판
	VulkanLayerAndExtension		layerExtension;

public:
	VkResult createDevice(std::vector<const char*>& layers, std::vector<const char*>& extensions);
	void destroyDevice();

	// 메모리 유형 결정, 이미지의 메모리를 할당하는 데 적합한 메모리 유형을 결정함
	bool memoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex);

	// 물리적 장치의 큐와 속성을 가져옴
	void getPhysicalDeviceQueuesAndProperties();

	// 논리적 장치 개체를 생성하려면 유효한 큐 핸들이 필요하며, 이를 이용해 연관된 큐를 생성하게 된다.
	// VkQueueFamilyProperties::queueFlags 비트 정보를 확인해 해당 큐를 찾는다.
	// 우리는 그래픽스 큐를 가져올거임
	uint32_t getGraphicsQueueHandle();

	// 큐와 관련된 멤버 함수들
	void getDeviceQueue();
};
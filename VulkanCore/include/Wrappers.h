#pragma once
#include "Headers.h"

/***************COMMAND BUFFER WRAPPERS***************/
/* CommandBuffer...
 - 커맨드 버퍼는 커맨드의 모음으로, GPU 처리를 위해 해당 기능의 하드웨어 큐에 제줄한다.
 - 그 다음 단계에서 드라이버는 실제 GPU가 처리가 시작되기 전에 커맨드 버퍼를 가져와 유효성을 검증하고 컴파일한다.
 1. 커맨드버퍼 시작하기
	a) 커맨드 버퍼 : 하나로 구성된 명령의 버퍼 또는 모음
	  커맨드 버퍼는 응용 프로그램이 실행하기를 원하는 일련의 VulkanAPI 명령을 레코딩한다.
	 한번 만들어진 커맨드 버퍼는 몇 번이고 다시 사용할 수 있다. 이 명령들은 응용 프로그램에 의해 지정된 순서로 레코딩된다.
	 버텍스 버퍼 바인딩, 파이프라인 바인딩, 렌더 패스 커맨드 레코딩, 뷰포트와 시저링 설정, 드로잉 명령들의 지정, 이미지와 버퍼 내용의 복사 작업 제어 등...
	  * 1차 커맨드 버퍼 : 2차 커맨드 버퍼의 소유자로, 이들을 실행하는 책임이 있고 큐에 직접 제출
	  * 2차 커맨드 버퍼 : 1차 커맨드 버퍼를 통해 실행되며 큐에 직접 제출할 수 없음
	- 응용 프로그램의 커맨드 버퍼 수는 수백에서 수천까지 다양하다. 커맨드 버퍼는 커맨드 풀에서 할당되도록 설계되었다.
		-> 이를 통해 다중 커맨드 버퍼에서 리소스를 생성하는 비용을 최소화하게 된다.
		-> 커맨드 버퍼는 직접 생성하지 않고, 대신 커맨드 풀로부터 할당해 사용한다. (렌더링 풀 -> 할당 이런느낌인가...?)
	- 커맨드 버퍼는 지속적이다. 계속 재사용 가능. 커맨드 버퍼를 더는 사용하지 않을 경우 간단한 리셋 명령으로 업데이트해 다른 레코딩을 위해 사용할 수 있다.
		-> 같은 목적을 위해 사용할 버퍼를 삭제하고 다시 생성하는 것과 비교하면 매우 효율적이다!
	
	b) 명시적 동기화
	- 다중 스레드 환경에서 여러 개의 커맨드 버퍼가 생성되면 각 스레드에 별도의 커맨드 풀을 도입해 동기화 도메인을 분리하는 것이 좋다.
	 응용 프로그램이 다른 스레드에서는 명시적 동기화가 필요하지 않으므로 이것은 커맨드 버퍼 할당을 효율적으로 만든다.
		-> ??? : 스레드 하나당 커맨드 풀 하나(1대1) -> 다른 스레드에서 커맨드 버퍼의 동기화가 필요하지 않음!
	+ OpenGL은 암묵적 동기화 모델... 리소스 추적, 캐시 삭제, 종속성 체인 구축에 큰 부하 발생 -> CPU에 큰 부하, 모든 작업이 커튼 뒤에서 진행
		-> Vulkan은 명시적! 사용자를 당황하게 할 현상 발생 X
	+ 응용 프로그램은 자원이 어떻게 사용되고 어떤 종속성이 있는지 명확히 이해하고 있다. 반면 드라이버는 정확하게 종속성을 파악할 수 없다.
		-> OpenGL 프로그램은 예상치 못한 셰이더 재컴파일, 캐시 삭제 등을 경험
		-> Vulkan의 명시적 동기화는 이러한 한계 X, 하드웨어를 더 생산적으로 만들 수 있다.

	c) 커맨드 버퍼에 들어갈 커맨드 유형
	- 커맨드 버퍼는 하나 이상의 커맨드로 구성되며, 이 커맨드는 다음 세 가지 유형으로 분류할 수 있다.
		- 동작 명령 : 그리기, 전송, 지우기, 복사, 쿼리와 타임스탬프 작업, 서브패스의 시작과 종료와 같은 동작을 수행한다.
		- 스테이트 관리 명령 : 디스크립터 세트와 파이프라인 바인딩, 버퍼를 포함하며, 동적 스테이트, 상수의 푸시, 렌더 패스와 서브패스 스테이트를 설정하는 데 사용한다.
		- 동기화 명령 : 파이프라인 장벽, 이벤트 설정, 대기 이벤트, 렌더 패스와 서브패스의 종속성 등이 포함된다.
	d) 커맨드 버퍼와 큐
	- 커맨드 버퍼는 비동기적으로 처리되는 하드웨어 큐에 제출된다. 큐에 제출할 때는 커맨드 버퍼를 일괄처리하기 위한 배치를 만들고 한번에 실행하면 효율적이다.
	Vulkan은 지연 커맨드 모델을 갖고 있다. 이 방식은 커맨드 버퍼에 있는 드로잉 명령들의 모음과 제출이 따로 수행되고 두 개의 다른 처리로 간주될 수 있다.
	이는 응용 프로그램 관점에서 볼 때 매우 유용하다. 응용 프로그램은 장면 구성에 대한 사전 지식을 가지고 있기 때문에 명령을 제출하는 과정에서 적절한 최적화를 할 수 있다.
		-> 커맨드 버퍼는 비동기적으로 처리되는 하드웨어 큐에 제출된다. 큐는 일괄처리를 진행한다. 커맨드 버퍼에 있는 모음과 제출이 따로 수행, 다른 처리로 간주
	- 실행 순서 : 단일 큐에 제출 또는 복수의 큐에 제출될 수 있음...
		- 단일 큐에 제출 : 단일 큐에 제출된 여러 개의 커맨드 버퍼는 실행되거나 중첩될 수 있다. 단일 큐 제출에서 커맨드 버퍼는 각 명령의 순서와 표준에서 정의된 API
		 실행 순서에 따라 수행되야 한다. vkQueueSubmit을 사용해 제출된 명령만을 다루고, 희소 메모리 바인딩 커맨드 버퍼는 다루지 않는다.
		- 복수 큐에 제출 : 복수의 큐에 제출된 커맨드 버퍼들은 특별한 순서 없이 실행된다. 순서 지정은 세마포어나 펜스를 통한 동기화를 통해서만 가능하다
 
 2. 커맨드 풀과 커맨드 버퍼 API 이해하기(커맨드 버퍼 풀 생성, 리셋, 삭제하는 과정....)
	a) 커맨드 풀은 vkCreateCommandPool() 를 통해 생성한다. 입력으로 VkCommandPoolCreateInfo 제어 구조체를 사용한다.
	- VkCommandPoolCreateInfo : 이 풀로부터 할당될 커맨드 버퍼의 성질에 대한 정보를 제공, 큐가 포함돼야 하는 큐 패밀리를 지정
		-> 호환 가능한 커맨드 풀을 할당하는 데 유용
		-> 이러한 풀을 사용해 일반적인 큐 제출에 필요한 커맨드 버퍼 할당 프로세스 최적화

	vkCommandPoolCreateInfo {}
		- type : VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO로 지정
		- next : 확장판 지정 구조체를 가리키는 유효한 포인터이거나 NULL
		- flag : 열거형 비트 필드 플래그, 커맨드 풀의 사용 동작과 풀로부터 할당된 커맨드 버퍼의 동작을 지정하게 된다.
				VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT만 사용할 수 있다.
		- queueFamilyu Index : 이 필드는 커맨드 버퍼가 제출될 큐 패밀리를 지정한다.
		+ VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : 이 풀에서 할당된 커맨드 버퍼가 자주 변경되고 수명이 짧음을 나타낸다.
						커맨드 버퍼의 특성을 알리고 메모리 할당 동작을 제어하는 데 사용할 수 있음
		+ VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : 풀에서 할당된 커맨드 버퍼가 두 가지 방법으로 개별적으로 리셋될 수 있음을 나타냄
						vkResetCommandBuffer를 명시적으로 호출하거나 vkBeginCommandBuffer를 호출하는 암묵적인 방법이 있다.
						이 플래그가 설정돼 있지 않으면 풀에서 할당된 실행 가능 커맨드 버퍼에서 두 API를 호출하면 안됨 -> vkResetCommandPool을 호출해 전체만 리셋할 수 있음

	vkCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool)
		- device : 커맨드 풀을 생성할 장치의 핸들
		- pCreateInfo : VkCommandPoolCreateInfo 개체를 참조하며 커맨드 풀 내 커맨드 버퍼의 특성을 지정한다.
		- pAllocater : 호스트 메모리를 제어하는 함수이다.
		- pCommnadPool : API 처리를 통해 만들어진 VkCommandPool이 반환되는 포인터

	vkResetCommandPool(device, commandPool, flags)
		- device : 커맨드 풀을 소유한 장치의 핸들
		- commandPool : 리셋해야 하는 VkCommandPool 핸들을 참조한다.
		- flags : 풀을 리셋하는 동작을 제어한다.

	vkDestroyCommandPool(device, commandPool, allocator)
		- device : 커맨드 풀을 삭제할 장치의 핸들이다
		- commandPool : 삭제해야 하는 vkCommandPool 핸들을 참조한다.
		- allocater : 호스트 메모리 할당을 제어한다.

	b) 커맨드 버퍼 할당
	- vkCommandBufferAllocateInfo 제어 구조체 개체를 입력으로 사용한다.
	vkAllocateCommandBuffer(device, pAllocateInfo, pCommandBuffers)
		- device : 필드는 커맨드 풀을 소유한 논리적 장치 개체의 핸들 참조
		- AllocateInfo : 구조체 지정, 할당에 필요한 파라미터 지정
		- commandBuffers : 할당된 커맨드 버퍼 개체 배열을 참조
	vkCommandBufferAllocateInfo{}
		- pNext : 이 필드는 NULL이거나 확장판 지정 구조체를 참조해야 한다.
		- commandPool : 요청받은 커맨드 버퍼에 메모리를 할당해야 하는 커맨드 풀의 핸들이다.
		- level : 1차 커냄드 버퍼인지 2차 커맨드 버퍼인지를 지정하는 vkCommandBufferLevel의 비트 필드 플래그이다..
			- VK_COMMAND_BUFFER_LEVLE_PRIMARY = 0;
			- VK_COMMAND_BUFFER_LEVEL_SECONDARY = 1;
		- commandBufferCount : 할당이 필요한 커맨드 버퍼의 수를 나타낸다.

	c) 커맨드 버퍼 리셋
	- vkResetCommandBuffer() API로 리셋
	- vkResetCommandBuffer(리셋할 VkCommandBuffer 지정, VK_COMMAND_BUFFER_RESET_RELESASE_RESOURCES_BIT)

 3. 커맨드 버퍼 레코딩
 - vkBeginCommandBuffer()와 vkEndCommandBuffer() API를 사용해 커맨드 버퍼에 명령들을 레코딩한다. (Vulkan명령이 레코딩되는 범위를 지정)
	vkBeginCommandBuffer(commandBuffer, pBeginInfo)
		- commandBuffer : 호출이 레코딩될 커맨드 버퍼의 핸들
		- pBegineInfo : 커맨드 버퍼의 레코딩 프로세스 시작 방법을 알려주는 추가 정보 지정
 - vkEndCommandBuffer(commandBuffer()API를 사용해 레코딩을 중지한다.
 - 큐 제출
	- 일단 커맨드 버퍼가 레코딩되면 큐에 제출할 준비가 된 것이다. vkQueueSubmit()API는 해당 큐에서 작업을 실행할 때 사용 한다.
	- vkQueueSubmit(queue, submitCount, pSubmitInfo, fence)
		- queue : 커맨드 버퍼가 제출될 큐의 핸들
		- submitCount : 개체 배열의 크기
		- pSubmitInfo : VkSubmitInfo 구조체 배열의 포인터, 작업 제출에 대한 핵심 정보를 갖고 있으며 작업의 수는 submitCount로 전달한다.
		- fence : 커맨드 버퍼 실행이 완료됐는지를 알려주는 신호 메커니즘에서 사용한다. 모든 커맨드 버퍼의 실행이 끝났을 때 fence는 종료를 알리는 신호를 받게 된다.
	- VkSubmitInfo{type, pnext, waitSemaphoreCount, pWaitSemaphores, pWaitDstStageMask, commandBufferCount, pCommandBuffers, signalSemaphoreCount, pSignalSemaphores}
		- sType : VK_STRUCTRE_TYPE_SUBMIT_INFO로  지정
		- pNext : 확장판 지정 구조체를 가리키거나 null로 지정
		- waitSemaphoreCount : 세마포어의 개수를 참조, 이 세마포어는 커맨드 버퍼가 실행되기 전에 대기하는 데 사용한다.
		- pWaitSemaphores : 세마포어 배열에 대한 포인터, 커맨드 버퍼가 만들어져 배치에서 실행되기 전에 기다려야 하는 세마포어를 가리킨다.
		- pWaitDstStageMask : 이 필드는 파이프라인 스테이지들의 배열에 대한 포인터로, 스테이지에서 각 해당 세마포어의 대기가 발생한다.
		- commandBufferCount : 배치에서 실행할 커맨드 버퍼의 수를 참조한다.
		- pCommnadBuffers : 배치에서 실행할 커맨드 버퍼의 배열을 가리키는 포인터다.
		- signalSemaphoreCount : commandBuffers에서 지정된 명령의 실행이 완료됐음을 알리는 신호를 받을 세마포어의 개수다.
		- pSignalSemaphores : 세마포어의 배열을 가리키는 포인터, 배치에 제출된 커맨드 버버의 명령 실행이 완료되면 신호를 받는다.
 - 큐 대기
	- 일단 큐에 제출되면, 응용 프로그램은 제출된 작업을 완료하고 다음 배치를 처리할 준비가 될 때까지 큐를 기다려야 한다...
	 큐를 기다리는 프로세스는 vkQueueWaitIdle()API를 사용해 수행할 수 있다. 이 API는 큐에 들어있는 모든 커맨드 버퍼와 희소 바인딩 작업이 완료될 때까지 차단된다.
	 이 API는 대기를 수행할 큐의 핸들을 입력 파라미터로 사용한다.
		-> 다음 배치를 처리할 준비가 될 때까지 큐를 기다려야한다..!

 4. 커맨드 버퍼의 래퍼 클래스 구현
 5. Vulkan의 메모리 관리
 - Vulkan은 메모리를 크게 호스트 메모리와 장치 메모리의 두 가지 유형으로 구분한다, 또한, 각 유형의 메모리는 
  특성과 메모리 유형에 따라 고유하게 세분화할 수 있다.
	-> 응용 프로그램이 메모리 영역과 레이아웃을 명시적으로 제어할 수 있다!

 - 호스트 메모리 : 속도 낮음, 크기 큼
 - 장치 메모리 : 효율성 큼, 속도 빠름

 - 호스트 메모리 
	- Vulkan은 할당 함수를 제공해 이를 응용 프로그램이 호스트 메모리의 메모리 할당 제저에 사용.
	 만약 할당함수를 사용하지 않으면, Vulkan 구현은 자신의 데이터 구조체를 저장하기 위해 메모리 슬롯을 확보하는 디폴드 할당 스킴을 사용
	- 호스트 메모리는 VkAllocationCallbacks 제어 구조체로 관리
 
 6. 정훈이 혼내주기





*/
// CommandBufferMgr : 커맨드 버퍼에 대한 래퍼 클래스, 값을 수정하지 않으면 기본 함수의 인수가 알잘딱 처리함
// 커맨드 버퍼 레코딩 함수를 사용하는 방법
// 1. 커맨드 풀을 생성하고 allocCommandBuffer()로 커맨드 버퍼를 할당함
// 2. 지정된 레코딩 벙위 정의 함수를 통해 일련의 명령이 레코딩됨
// 3. 커맨드 버퍼는 submitCommandBuffer()로 제출...
class CommandBufferMgr
{
public:
	// 커맨드 버퍼 할당 프로세스 구현
	static void allocCommandBuffer(const VkDevice* device, const VkCommandPool cmdPool, VkCommandBuffer* cmdBuf, const VkCommandBufferAllocateInfo* commandBufferInfo = NULL);
	
	// 커맨드 버퍼 할당 프로세스 레코딩
	static void beginCommandBuffer(VkCommandBuffer cmdBuf, VkCommandBufferBeginInfo* inCmdBufInfo = NULL);
	
	// 커맨드 버퍼 레코딩의 끝
	static void endCommandBuffer(VkCommandBuffer cmdBuf);

	// 큐에 커맨드 버퍼 제출
	// queue : 큐 지정
	// cmdBufList : 커맨드 버퍼
	// submitInfo : 제출 프로세스 제어
	// fence : 제출된 커맨드 버퍼의 완료
	static void submitCommandBuffer(const VkQueue& queue, const VkCommandBuffer* cmdBufList, const VkSubmitInfo* submitInfo = NULL, const VkFence& fence = VK_NULL_HANDLE);
};

void* readFile(const char* spvFileName, size_t* fileSize);
/*
1. VKLayerAndExtension
 - 인스턴스와 장치에 대해 레이어와 확장판 기능을 제공한다. 또한 디버깅 기능도 제공한다

 레이어?
 -   레이어는 기존 VulkanAPI에 연결되고, 지정된 레이어와 연결된 Vulkan 명령 체인에 삽입된다.
	레이어는 일반적으로 개발 프로세스를 검증하는 데 사용된다. 예를 들어, 드라이버는 Vulkan API에서 제공한 파라이터를
	확인할 필요가 없다. 들어오는 파라미터가 올바른지를 확인하는 것이 레이어의 역할이다.
		-> 주로 검증을 위해....
 확장판?
 -   확장판은 확장된 기능 또는 특징을 제공한다. 이 기능들은 추후 표준 사양에 포함되거나 포함되지 않을 수 있다.
	확장판은 인스턴스 또는 장치 일부로 구현될 수 있으며, 확장판 명령은 정적으로 링크할 수 없다. 확장판은 먼저 
	쿼리된 후 함수 포인터에 동적으로 링크된다. 등록 확장판인 경우 이 함수 포인터는 필요한 데이터 구조체, 열거형 데이터와 함께
	vulkan.h에 이미 정의돼 있을 수 있다.
		-> API에서 제공하는 확장된 기능... 인스턴스 또는 장치 일부로 구현... 함수 포인터에 동적으로 링크... 등록된 확장판은 이미 정의되어 있음

 디버깅?
 -	 유효성 검증 과정과 디버깅 모델을 선택적으로 사용할 수 있게 함 -> 실제 게임에서의 오버헤드 감소, 런타임때 시스템에 주입
	Vulkan 디버깅은 응용 프로그램 구현에서 오류만이 아니라 API를 적절히 사용했는지 점검하는 등 유효성 검증 기능도 갖고 있다.
	전달된 각 파라미터 변수를 확인하고 잠재적으로 부정확하거나 위험성이 있는 API 사용이 발견되면 경고하고 최적으로 사용되지 않아도 경고를 보여준다 \
		-> Vulkan 디버깅은 API의 오류, 적절성, 위험성 등을 검증하고 경고 및 오류를 알려줌...

 -	 디버깅이 활성화되면 연관된 Vulkan 명령에 대한 호출 체인에 자기 자신을 삽입한다. 각 명령에 대해 디버깅 레이어는
	활성화된 모든 레이어를 방문하고, 유효성을 검증해 잠재적인 오류, 경고, 디버깅 정보 등을 점검한다.
		-> Vulkan 명령에 대한 호출 체인에 자기 자신을 삽입 -> 활성화된 모든 레이어 방문 -> 유효성 검증

 - 활성화 단계
	1. 인스턴스 계층에서 VK_EXT_DEBUG_REPORT_EXTENSION_NAME 확장팩 추가해 디버깅 기능 활성화
	2. 디버깅을 위한 유효성 검증 레이어를 정의
		EX) VK_LAYTER_GOOGLE_unique_objects, VK_LAYER_LUNARG_api_dump etc...
	3. Vulkan 디버깅 API는 정적으로 로더에 의해 로딩되는 코어 명령이 아니다.
	  디버깅 API는 런타임에 검색해 미리 정의된 함수 포인터에 동적으로 링크할 수 있는 확장판 API 형태로 사용할 수 있다.
	  디버깅 확장판 API인 vkCreateDebugReportCallbackEXT 와 vkDestroyDebugReportCallbackEXT를 쿼리하고 동적으로 연결한다.
		-> 디버깅 API는 정적으로 로더에 의해 로딩 X, 동적으로 링크할 수 있는 확장판 API 형태로 사용
	4. 디버깅 보고서에 대한 함수 포인터를 성공적으로 가져오면 이전 API가 디버깅 보고서 개체를 생성한다.
	  Vulkan은 이 API에 연결된 사용자 정의 콜백 함수에 디버깅 보고서를 반환한다.
		-> 디버깅 보고서 함수 포인터를 가져오고 보고서 개체 생성, 반환
	5. 디버깅이 더는 필요하지 않으면 디버깅 보고서 개체를 삭제함

 - 디버깅의 구현
	1. 확장판을 인스턴스 계층에 추가한다.
	2. 레이어 인스턴스 계층에서 다음 레이어를 정의하고 이 레이어에서 디버깅할 수 있다.
		+ LunarG SDK는 사용할 수 있는 검증 레이어들에 추가로 VK_LAYER_LUNARG_standard_validation이라는 특별한 레이어를 제공한다.
		 이 레이어는 다음에 설명한 순서로 기본 검증을 사용할 수 있게 한다.
		 또한 이 빌트인 메타 데이터 레이어는 검증 레이어의 표준 세트를 최적의 순서로 로딩해준다
		 레이어를 사용하는 정확한 순서가 지정돼 있지 않다면 다음 순서 사용을 권장한다.
		a) "VK_LAYER_GOOGLE_threading",
		b) "VK_LAYER_LUNARG_parameter_validation",
		c) "VK_LAYER_LUNARG_object_tracker",
		d) "VK_LAYER_LUNARG_image",
		e) "VK_LAYER_LUNARG_core_validation",
		f) "VK_LAYER_LUNARG_swapchain",
		g) "VK_LAYER_GOOGLE_unique_objects"
			-> VK_LAYER_LUNARG_standard_validation 제공, 최적의 순서로 레이어 로딩, 정확한 순서가 없다면 다음 순서 사용 권장
			+ vkCreateInstance() API에 제공해 활성화 진행 (VKInstance)
	3. VulkanLayerAndExtension 클래스에서 수행한다.
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
	// - 인스턴스 또는 전역 레이어를 쿼리한다.
	// - 레이어의 총 개수를 가져와 모든 레이어 정보를 layerProperties라는 VkLayerProperties 벡터에 저장한다.
	VkResult getInstanceLayerProperties();

	// getExtensionProperties?
	// - 각 레이어는 하나 이상의 확장판을 지원할 수 있다...
	// - 먼저 제공되는 확장판의 수를 얻기 위해 호출된다.
	VkResult getExtensionProperties(LayerProperties& layerProps, VkPhysicalDevice* gpu = NULL);

	// 장치 기반 확장판
	VkResult getDeviceExtensionProperties(VkPhysicalDevice* gpu);

	/******* VULKAN DEBUGGING MEMBER FUNCTION AND VARAIBLES *******/

	// 유효성 검증 레잉어는 공급 업체, SDK 버전에 따라 매우 다름, 레이어가 기본 구현에서 지원되는지 먼저 확인하는 것이 필요..
	// 시스템에서 지원하는 레이어인지 점검!
	VkBool32 areLayersSupported(std::vector<const char*>& layerNames);

	
	// 이 API는 Vulkan의 기본 명령이 아니라 정적으로 링크하지 않는다...
	// 모든 디버깅 관련 API는 vkGetInstanceProcAddr()API로 쿼리하고 동적으로 링크해야 한다.
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
	//생성과 삭제 함수 포인터 선언
	PFN_vkCreateDebugReportCallbackEXT dbgCreateDebugReportCallback;
	PFN_vkDestroyDebugReportCallbackEXT dbgDestroyDebugReportCallback;
	
	// 디버깅 보고서 콜백 함수의 핸들
	VkDebugReportCallbackEXT debugReportCallback;
	
public:
	// 디버깅 보고서의 콜백 함수는 정보 제어 구조체를 생성함
	VkDebugReportCallbackCreateInfoEXT dbgReportCreateInfo = {};
};
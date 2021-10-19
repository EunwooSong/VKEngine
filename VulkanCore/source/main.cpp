#include "Headers.h"
#include "VKApplication.h"

std::vector<const char*> instanceExtensionNames = {
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME
};

std::vector<const char*> layerNames = {
	"VK_LAYER_GOOGLE_threading"
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_image",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_LUNARG_swapchain",
	"VK_LAYER_GOOGLE_unique_objects",
	"VK_LAYER_LUNARG_standard_validation"
};

std::vector<const char*> deviceExtensionNames = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

int main(int argc, char** argv)
{
	VKApplication* appObj = VKApplication::GetInstance();
	appObj->initialize();
	appObj->prepare();
	bool isWindowOpen = true;
	while (isWindowOpen) {
		appObj->update();
		isWindowOpen = appObj->render();
	}
	appObj->deInitialize();
}



/* 개발하면서 어렵게 해결한 문제들...
* 순서 : 순번 + 문제 + 오류 발생 일 -> 분석 -> 해결 방안

 1. 잘 되던 프로젝트에서 갑자기 vkCreateDevice(), VK_ERROR_LOST_DEVICE... 등의 오류가 발생 (2021.10.1)
	- 설치된 그래픽 카드의 버전이 실행일자와 1년 이상 차이나면 실행이 안됨!
		-> 그래픽카드 업데이트.

 2. 디스크립터 생성후 Shdaer에 MVP를 Uniform Buffer를 통해 받을 때 생긴 오류 + 푸시 상수가 정상적으로 안들어옴 (2021.10.10)
	- 파이프라인에 정의가 안되어 있었음
		-> 파이프라인 레이아웃 수정
		-> vkCreateInfo에서 디스크립터 수정

*/
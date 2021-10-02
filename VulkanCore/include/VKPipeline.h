#pragma once
#include "Headers.h"

class VKShader;
class VKDrawable;
class VKDevice;
class VKApplication;

#define NUMBER_OF_VIEWPORTS 1
#define NUMBER_OF_SCISSORS NUMBER_OF_VIEWPORTS

// 입력 데이터가 흘러가는 일련의 고정된 스테이지를 의미한다.
// 그래픽스 파이프라인 : 커맨드 버퍼를 통해 Vulkan 명령 세트를 가져 와서 2D/3D 장면을 입력으로 한 2D 래스터 이미지를 그린다.
// 컴퓨팅 파이프라인 : 이 파이프라인은 커맨드 버퍼를 통해 Vulkan 명령을 가져와서 계산 작업을 처리한다.

// VKPipeline
// - 파이프라인 구현을 관리한다. 파이프라인 개체를 재사용하는 것이 효율적, PCO을 통해 파이프라인 캐싱 메커니즘을 제공하므로 오버헤드를 줄일 수 있다.
// - 파이프라인 재사용성을 높이기 위해 PCO에 쉽게 액세스 할 수 있는 레벨에 배치되야 한다.
class VKPipeline {
public:
	VKPipeline();
	~VKPipeline();

	// 파이프라인 캐시 개체를 생성하고 파이프라인 개체를 저장
	void createPipelineCache();

	// 생성된 파이프라인 개체를 반환, 개체에는 드로잉 개체의 버텍스 입력 비율
	// 데이터 해석 방법, 셰이더 파일, 깊이 지원 여부를 나타내는 이진 플래그와
	// 버텍스 입력이 사용 가능한지를 나타내는 플래그 등이 포함된다.
	bool createPipeline(VKDrawable* drawableObj, VkPipeline* pipeline, VKShader* shaderObj, VkBool32 includeDepth, VkBool32 includeVi = true);
	
	// 파이프 라인 캐시 개체 삭제
	void destroyPipelineCache();

public:
	VkPipelineCache		pipelineCache;
	VkPipelineLayout	pipelineLayout;

	VKApplication*		appObj;
	VKDevice*			deviceObj;
};
#pragma once
#include "Headers.h"

class VKDevice;
class VKApplication;

// 렌더링할 기하 개체에 대한 3D 변환을 구현....
// 유니폼은 셰이더가 액세슬할 수 있는 읽기 전용 데이터 블록
// 전체 드로잉 호출에 대해 상수값으로 작동한다.
// 디스크립터 세트는 주어진 리소스(유니폼 버퍼, 샘플 이미지, 저장된 이미지 등)을 셰이더에 연결하며,
// 디스크립터 세튼 레이아웃을 사용해 정의된 레이아웃 바인딩으로 들어오는 리소스 데이터를 읽고 해석하는 것을 돕는다.
// DescriptorSetLayout : 셰이더가 지정된 위치의 리소스를 읽을 수 있게 하는 인터페이스를 저장
// PipelineLayout : 파이프라인이 디스크립터 세트를 액세스 할 수 있다. 파이프라인 레이아웃은 디스크립터 세트 레이아웃과 푸시 상수 범위로 구성되며, 기본 파이프라인이 액세스 할 수 있는 전체 리소스 세트를 나타낸다.
// DescriptorPool : 디스크립터 세트를 만들기 위해 할당되는 디스크립터의 모음
// Descriptor
class VKDescriptor
{
public:
	VKDescriptor();
	~VKDescriptor();

	// 디스크립터 풀을 생성하고 이로부터 디스크립터 세트를 할당
	void createDescriptor(bool useTexture);
	// 생성된 디스크립터 세트 개체를 삭제
	void destroyDescriptor();

	// 디스크립터 세트 레이아웃 바인딩을 정의하고 디스크립터 레이아웃을 생성
	virtual void createDescriptorSetLayout(bool useTexture) = 0;

	// 유효한 디스크립터 레이아웃 개체를 삭제
	void destroyDescriptorLayout();

	// 디스크립터 세트의 할당에 사용할 디스크립터 풀 생성
	virtual void createDescriptorPool(bool useTexture) = 0;
	
	// 디스크립터 풀 삭제
	void destroyDescriptorPool();

	// 파이프라인에 주입하기 위한 파이프라인 레이아웃 생성
	virtual void createDescriptorResources() = 0;

	// 디스크립터 풀에 할당된 메모리로부터 디스크립터 세트를 생성하고 디스크립터 세트 정보를 업데이트
	virtual void createDescriptorSet(bool useTexture) = 0;
	void destroyDescriptorSet();

	// 파이프라인에 주입하기 위한 파이프라인 레이아웃 생성
	virtual void createPipelineLayout() = 0;
	// 생성된 파이프라인 레이아웃 삭제
	void destroyPipelineLayouts();

public:
	// 파이프라인 레이아웃 개체
	VkPipelineLayout pipelineLayout;

	// 모든 VkDescriptorSetLayouts의 목록
	std::vector<VkDescriptorSetLayout> descLayout;

	// VkDescriptorSet 개체를 할당하는 데 사용하는 디스크립터 풀 개체
	VkDescriptorPool descriptorPool;

	// 생성된 모든 VkDescriptorSet의 목록
	std::vector<VkDescriptorSet> descriptorSet;

	// 디스크립터 풀과 디스크립터 풀을 생성하는 데 사용하는 논리적 장치
	VKDevice* deviceObj;
};
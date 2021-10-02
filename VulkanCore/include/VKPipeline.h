#pragma once
#include "Headers.h"

class VKShader;
class VKDrawable;
class VKDevice;
class VKApplication;

#define NUMBER_OF_VIEWPORTS 1
#define NUMBER_OF_SCISSORS NUMBER_OF_VIEWPORTS

// �Է� �����Ͱ� �귯���� �Ϸ��� ������ ���������� �ǹ��Ѵ�.
// �׷��Ƚ� ���������� : Ŀ�ǵ� ���۸� ���� Vulkan ��� ��Ʈ�� ���� �ͼ� 2D/3D ����� �Է����� �� 2D ������ �̹����� �׸���.
// ��ǻ�� ���������� : �� ������������ Ŀ�ǵ� ���۸� ���� Vulkan ����� �����ͼ� ��� �۾��� ó���Ѵ�.

// VKPipeline
// - ���������� ������ �����Ѵ�. ���������� ��ü�� �����ϴ� ���� ȿ����, PCO�� ���� ���������� ĳ�� ��Ŀ������ �����ϹǷ� ������带 ���� �� �ִ�.
// - ���������� ���뼺�� ���̱� ���� PCO�� ���� �׼��� �� �� �ִ� ������ ��ġ�Ǿ� �Ѵ�.
class VKPipeline {
public:
	VKPipeline();
	~VKPipeline();

	// ���������� ĳ�� ��ü�� �����ϰ� ���������� ��ü�� ����
	void createPipelineCache();

	// ������ ���������� ��ü�� ��ȯ, ��ü���� ����� ��ü�� ���ؽ� �Է� ����
	// ������ �ؼ� ���, ���̴� ����, ���� ���� ���θ� ��Ÿ���� ���� �÷��׿�
	// ���ؽ� �Է��� ��� ���������� ��Ÿ���� �÷��� ���� ���Եȴ�.
	bool createPipeline(VKDrawable* drawableObj, VkPipeline* pipeline, VKShader* shaderObj, VkBool32 includeDepth, VkBool32 includeVi = true);
	
	// ������ ���� ĳ�� ��ü ����
	void destroyPipelineCache();

public:
	VkPipelineCache		pipelineCache;
	VkPipelineLayout	pipelineLayout;

	VKApplication*		appObj;
	VKDevice*			deviceObj;
};
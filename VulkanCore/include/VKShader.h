#pragma once
#include "Headers.h"

// Shader class managing the shader conversion, compilation, linking
class VKShader
{
public:
	VKShader() {}
	~VKShader() {}
	
	void buildShaderModuleWithSPV(uint32_t* vertShaderText, size_t vertexSPVSize, uint32_t* fragShaderText, size_t fragmentSPVSize);

	// �ʿ����� ���� ��� ���̴� ����
	void destroyShaders();

#ifdef AUTO_COMPILE_GLSL_TO_SPV
	// GLSL ���̴��� SPIR-V�� ��ȯ
	bool GLSLtoSPV(const VkShaderStageFlagBits shaderType, const char* pshader, std::vector<unsigned int>& spirv);

	// ���̴��� �����ϱ� ���� ������
	void buildShader(const char* vertShaderText, const char* fragShaderText);

	// ���̴� ����� ����.. ����� �� �ִ� �������� - EShLangVertex,Tessellation Control, 
	// Tessellation Evaluation, Geometry, Fragment, Compute�� �ִ�.
	EShLanguage getLanguage(const VkShaderStageFlagBits shader_type);

	// TbuitInResource �ʱ�ȭ
	void initializeResources(TBuiltInResource& Resources);
#endif

	// ���ؽ� ���̴��� �����׸�Ʈ ���̴� ������ ������ ����ü
	VkPipelineShaderStageCreateInfo shaderStages[2];
};
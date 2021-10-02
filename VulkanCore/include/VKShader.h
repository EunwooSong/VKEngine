#pragma once
#include "Headers.h"

// Shader class managing the shader conversion, compilation, linking
class VKShader
{
public:
	VKShader() {}
	~VKShader() {}
	
	void buildShaderModuleWithSPV(uint32_t* vertShaderText, size_t vertexSPVSize, uint32_t* fragShaderText, size_t fragmentSPVSize);

	// 필요하지 않은 경우 셰이더 종료
	void destroyShaders();

#ifdef AUTO_COMPILE_GLSL_TO_SPV
	// GLSL 셰이더를 SPIR-V로 변환
	bool GLSLtoSPV(const VkShaderStageFlagBits shaderType, const char* pshader, std::vector<unsigned int>& spirv);

	// 셰이더를 빌드하기 위한 시작점
	void buildShader(const char* vertShaderText, const char* fragShaderText);

	// 셰이더 언어의 유형.. 사용할 수 있는 유형에는 - EShLangVertex,Tessellation Control, 
	// Tessellation Evaluation, Geometry, Fragment, Compute가 있다.
	EShLanguage getLanguage(const VkShaderStageFlagBits shader_type);

	// TbuitInResource 초기화
	void initializeResources(TBuiltInResource& Resources);
#endif

	// 버텍스 셰이더와 프래그먼트 셰이더 정보를 저장할 구조체
	VkPipelineShaderStageCreateInfo shaderStages[2];
};
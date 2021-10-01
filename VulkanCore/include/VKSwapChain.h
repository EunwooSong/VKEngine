#pragma once

#include "Headers.h"

class VKInstance;
class VKDevice;
class VKRenderer;
class VKApplication;
/*
* Keep each of our swap chain buffers' image, command buffer and view in one spot
*/
struct SwapChainBuffer {
	VkImage image;
	VkImageView view;
};

struct SwapChainPrivateVariables
{
	// 이미지 화면의 기능(Capability) 저장
	VkSurfaceCapabilitiesKHR	surfCapabilities;

	// 프레젠테이션 모드(수직 동기화, 바로바로, etc...)의 수 저장
	uint32_t					presentModeCount;

	// 프레젠 테이션 모드를 가져올 배열
	std::vector<VkPresentModeKHR> presentModes;

	// 스와프 테인 색상 이미지의 크기
	VkExtent2D					swapChainExtent;

	// 지원되는 색상 이미지의 크기
	uint32_t					desiredNumberOfSwapChainImages;
	VkSurfaceTransformFlagBitsKHR preTransform;

	// 스와프 체인 생성을 위한 프레젠테이션 모드의 비트 플래그 저장
	VkPresentModeKHR			swapchainPresentMode;

	// 가져온 드로잉 색상 스와프 체인 이미지들
	std::vector<VkImage>		swapchainImages;

	std::vector<VkSurfaceFormatKHR> surfFormats;
};

struct SwapChainPublicVariables
{
	// 플랫폼에 따른 화면 개체
	VkSurfaceKHR surface;

	// 스와프 체인에 사용될 버퍼 이미지의 수
	uint32_t swapchainImageCount;

	// 스와프 체인 개체
	VkSwapchainKHR swapChain;

	// 색상 스와프 체인 이미지의 목록
	std::vector<SwapChainBuffer> colorBuffer;

	// 싱크를 위한 Semaphore(동기화)
	VkSemaphore presentCompleteSemaphore;

	// 현재 사용중인 드로잉 화면의 인덱스
	uint32_t currentColorBuffer;

	// 이미지 포맷
	VkFormat format;
};

// API 확장판 쿼리(동적...) -> 적절한 이미지 포맷 획득 -> 화면 기능과 프레젠테이션 모드 가져옴...
class VKSwapChain {

	// Public member function
public:
	VKSwapChain(VKRenderer* renderer);
	~VKSwapChain();
	void intializeSwapChain();
	void createSwapChain(const VkCommandBuffer& cmd);
	void destroySwapChain();


	// Private member variables
private:
	VkResult createSwapChainExtensions();
	void getSupportedFormats();
	VkResult createSurface();
	uint32_t getGraphicsQueueWithPresentationSupport();
	void getSurfaceCapabilitiesAndPresentMode();
	void managePresentMode();
	void createSwapChainColorImages();
	void createColorImageView(const VkCommandBuffer& cmd);

	// Public member variables
public:
	// 사용자 정의 구조체에는 스와프 체인의 private와
	// public 함수들이 사용할 private 변수가 들어있음
	SwapChainPublicVariables	scPublicVars;
	PFN_vkQueuePresentKHR		fpQueuePresentKHR;
	PFN_vkAcquireNextImageKHR	fpAcquireNextImageKHR;

	// Private member variables
private:
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR		fpGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR	fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR		fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR	fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkDestroySurfaceKHR							fpDestroySurfaceKHR;

	// Layer Extensions Debugging
	PFN_vkCreateSwapchainKHR	fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR	fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;

	SwapChainPrivateVariables	scPrivateVars;
	VKRenderer* rendererObj;	// parent
	VKApplication* appObj;

	
};
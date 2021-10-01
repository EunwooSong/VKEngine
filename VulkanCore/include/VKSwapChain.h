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
	// �̹��� ȭ���� ���(Capability) ����
	VkSurfaceCapabilitiesKHR	surfCapabilities;

	// ���������̼� ���(���� ����ȭ, �ٷιٷ�, etc...)�� �� ����
	uint32_t					presentModeCount;

	// ������ ���̼� ��带 ������ �迭
	std::vector<VkPresentModeKHR> presentModes;

	// ������ ���� ���� �̹����� ũ��
	VkExtent2D					swapChainExtent;

	// �����Ǵ� ���� �̹����� ũ��
	uint32_t					desiredNumberOfSwapChainImages;
	VkSurfaceTransformFlagBitsKHR preTransform;

	// ������ ü�� ������ ���� ���������̼� ����� ��Ʈ �÷��� ����
	VkPresentModeKHR			swapchainPresentMode;

	// ������ ����� ���� ������ ü�� �̹�����
	std::vector<VkImage>		swapchainImages;

	std::vector<VkSurfaceFormatKHR> surfFormats;
};

struct SwapChainPublicVariables
{
	// �÷����� ���� ȭ�� ��ü
	VkSurfaceKHR surface;

	// ������ ü�ο� ���� ���� �̹����� ��
	uint32_t swapchainImageCount;

	// ������ ü�� ��ü
	VkSwapchainKHR swapChain;

	// ���� ������ ü�� �̹����� ���
	std::vector<SwapChainBuffer> colorBuffer;

	// ��ũ�� ���� Semaphore(����ȭ)
	VkSemaphore presentCompleteSemaphore;

	// ���� ������� ����� ȭ���� �ε���
	uint32_t currentColorBuffer;

	// �̹��� ����
	VkFormat format;
};

// API Ȯ���� ����(����...) -> ������ �̹��� ���� ȹ�� -> ȭ�� ��ɰ� ���������̼� ��� ������...
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
	// ����� ���� ����ü���� ������ ü���� private��
	// public �Լ����� ����� private ������ �������
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
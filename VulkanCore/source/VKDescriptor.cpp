#include "VKDescriptor.h"
#include "VKApplication.h"
#include "VKDevice.h"

VKDescriptor::VKDescriptor()
{
	deviceObj =VKApplication::GetInstance()->deviceObj;
}

VKDescriptor::~VKDescriptor()
{
}

void VKDescriptor::createDescriptor(bool useTexture)
{
	// Create the uniform buffer resource 
	createDescriptorResources();

	// Create the descriptor pool and 
	// use it for descriptor set allocation
	createDescriptorPool(useTexture);

	// Create descriptor set with uniform buffer data in it
	createDescriptorSet(useTexture);
}

void VKDescriptor::destroyDescriptor()
{
	destroyDescriptorLayout();
	destroyPipelineLayouts();
	destroyDescriptorSet();
	destroyDescriptorPool();
}

void VKDescriptor::destroyDescriptorLayout()
{
	for (int i = 0; i < descLayout.size(); i++) {
		vkDestroyDescriptorSetLayout(deviceObj->device, descLayout[i], NULL);
	}
	descLayout.clear();
}

void VKDescriptor::destroyDescriptorPool()
{
	vkDestroyPipelineLayout(deviceObj->device, pipelineLayout, NULL);
}

void VKDescriptor::destroyDescriptorSet()
{
	vkDestroyDescriptorPool(deviceObj->device, descriptorPool, NULL);
}

void VKDescriptor::destroyPipelineLayouts()
{
	vkFreeDescriptorSets(deviceObj->device, descriptorPool, (uint32_t)descriptorSet.size(), &descriptorSet[0]);
}

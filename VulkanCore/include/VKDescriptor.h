#pragma once
#include "Headers.h"

class VKDevice;
class VKApplication;

// �������� ���� ��ü�� ���� 3D ��ȯ�� ����....
// �������� ���̴��� �׼����� �� �ִ� �б� ���� ������ ���
// ��ü ����� ȣ�⿡ ���� ��������� �۵��Ѵ�.
// ��ũ���� ��Ʈ�� �־��� ���ҽ�(������ ����, ���� �̹���, ����� �̹��� ��)�� ���̴��� �����ϸ�,
// ��ũ���� ��ư ���̾ƿ��� ����� ���ǵ� ���̾ƿ� ���ε����� ������ ���ҽ� �����͸� �а� �ؼ��ϴ� ���� ���´�.
// DescriptorSetLayout : ���̴��� ������ ��ġ�� ���ҽ��� ���� �� �ְ� �ϴ� �������̽��� ����
// PipelineLayout : ������������ ��ũ���� ��Ʈ�� �׼��� �� �� �ִ�. ���������� ���̾ƿ��� ��ũ���� ��Ʈ ���̾ƿ��� Ǫ�� ��� ������ �����Ǹ�, �⺻ ������������ �׼��� �� �� �ִ� ��ü ���ҽ� ��Ʈ�� ��Ÿ����.
// DescriptorPool : ��ũ���� ��Ʈ�� ����� ���� �Ҵ�Ǵ� ��ũ������ ����
// Descriptor
class VKDescriptor
{
public:
	VKDescriptor();
	~VKDescriptor();

	// ��ũ���� Ǯ�� �����ϰ� �̷κ��� ��ũ���� ��Ʈ�� �Ҵ�
	void createDescriptor(bool useTexture);
	// ������ ��ũ���� ��Ʈ ��ü�� ����
	void destroyDescriptor();

	// ��ũ���� ��Ʈ ���̾ƿ� ���ε��� �����ϰ� ��ũ���� ���̾ƿ��� ����
	virtual void createDescriptorSetLayout(bool useTexture) = 0;

	// ��ȿ�� ��ũ���� ���̾ƿ� ��ü�� ����
	void destroyDescriptorLayout();

	// ��ũ���� ��Ʈ�� �Ҵ翡 ����� ��ũ���� Ǯ ����
	virtual void createDescriptorPool(bool useTexture) = 0;
	
	// ��ũ���� Ǯ ����
	void destroyDescriptorPool();

	// ���������ο� �����ϱ� ���� ���������� ���̾ƿ� ����
	virtual void createDescriptorResources() = 0;

	// ��ũ���� Ǯ�� �Ҵ�� �޸𸮷κ��� ��ũ���� ��Ʈ�� �����ϰ� ��ũ���� ��Ʈ ������ ������Ʈ
	virtual void createDescriptorSet(bool useTexture) = 0;
	void destroyDescriptorSet();

	// ���������ο� �����ϱ� ���� ���������� ���̾ƿ� ����
	virtual void createPipelineLayout() = 0;
	// ������ ���������� ���̾ƿ� ����
	void destroyPipelineLayouts();

public:
	// ���������� ���̾ƿ� ��ü
	VkPipelineLayout pipelineLayout;

	// ��� VkDescriptorSetLayouts�� ���
	std::vector<VkDescriptorSetLayout> descLayout;

	// VkDescriptorSet ��ü�� �Ҵ��ϴ� �� ����ϴ� ��ũ���� Ǯ ��ü
	VkDescriptorPool descriptorPool;

	// ������ ��� VkDescriptorSet�� ���
	std::vector<VkDescriptorSet> descriptorSet;

	// ��ũ���� Ǯ�� ��ũ���� Ǯ�� �����ϴ� �� ����ϴ� ���� ��ġ
	VKDevice* deviceObj;
};
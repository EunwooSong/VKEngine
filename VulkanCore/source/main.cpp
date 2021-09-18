#include "Headers.h"
#include "VKApplication.h"

std::vector<const char*> instanceExtensionNames = {
	VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME
};

std::vector<const char*> layerNames = {
	"VK_LAYER_LUNARG_api_dump"
};

std::vector<const char*> deviceExtensionNames = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

int main()
{
	VKApplication* appObj = VKApplication::GetInstance();
	appObj->initialize();
	appObj->deInitialize();

	return 0;
}
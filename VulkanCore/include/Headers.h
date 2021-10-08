#pragma once

/*********** COMPILER SPECIFIC PREPROCESSORS ***********/
#ifdef _WIN32
#pragma comment(linker, "/subsystem:console")
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define APP_NAME_STR_LEN 80
#define _CRT_SECURE_NO_WARNINGS
#else  // _WIN32
#define VK_USE_PLATFORM_XCB_KHR
#include <unistd.h>
#endif // _WIN32

/*********** C/C++ HEADER FILES ***********/
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <iomanip>
#include <fstream>
#include <assert.h>

// Header files for Singleton
#include <memory>
#include <mutex>

/*********** GLM HEADER FILES ***********/
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

/*********** VULKAN HEADER FILES ***********/
#include <vulkan/vulkan.h>
#ifdef AUTO_COMPILE_GLSL_TO_SPV
#include "SPIRV/GlslangToSpv.h"
#endif
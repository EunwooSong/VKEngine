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
#include <iomanip>
#include <assert.h>

// Header files for Singleton
#include <memory>
#include <mutex>

/*********** VULKAN HEADER FILES ***********/
#include <vulkan/vulkan.h>
# CMakeList.txt: VulkanCore에 대한 CMake 프로젝트, 여기에 소스를 포함하고
# 프로젝트 특정 논리를 정의합니다.
#
cmake_minimum_required (VERSION 3.8)

# AUTO_LOCATE_VULKAN - accepted value ON or OFF
# ON  - Use CMake to auto locate the Vulkan SDK.
# OFF - Vulkan SDK path can be specified manually. This is helpful to test the build on various Vulkan version.
option(AUTO_LOCATE_VULKAN "AUTO_LOCATE_VULKAN" ON)

if(AUTO_LOCATE_VULKAN)
	message(STATUS "Attempting auto locate Vulkan using CMake......")
	
	# Find Vulkan Path using CMake's Vulkan Module
	# This will return Boolean 'Vulkan_FOUND' indicating the status of find as success(ON) or fail(OFF).
	# Include directory path - 'Vulkan_INCLUDE_DIRS' and 'Vulkan_LIBRARY' with required libraries.
	find_package(Vulkan)
	
	# Try extracting VulkanSDK path from ${Vulkan_INCLUDE_DIRS}
	if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
		set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
		STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
	endif()
		 
	if(NOT Vulkan_FOUND)
		# CMake may fail to locate the libraries but could be able to 
		# provide some path in Vulkan SDK include directory variable
		# 'Vulkan_INCLUDE_DIRS', try to extract path from this.
		message(STATUS "Failed to locate Vulkan SDK, retrying again...")
		if(EXISTS "${VULKAN_PATH}")
			message(STATUS "Successfully located the Vulkan SDK: ${VULKAN_PATH}")
		else()
			message("Error: Unable to locate Vulkan SDK. Please turn off auto locate option by specifying 'AUTO_LOCATE_VULKAN' as 'OFF'")
			message("and specify manually path using 'VULKAN_SDK' and 'VULKAN_VERSION' variables in the CMakeLists.txt.")
			return()
		endif()
	endif()
else()
	message(STATUS "Attempting to locate Vulkan SDK using manual path......")
	# Vulkan SDK가 설치되어있는 경로 설정
	set(VULKAN_SDK "C:/VulkanSDK")
	# 로드할 불칸의 버전 설정
	set(VULKAN_VERSION "1.0.33.0")
	set(VULKAN_PATH "${VULKAN_SDK}/${VULKAN_VERSION}")
	message(STATUS "Using manual specified path: ${VULKAN_PATH}")

	# Check if manual set path exists
	if(NOT EXISTS "${VULKAN_PATH}")
		message("Error: Unable to locate this Vulkan SDK path VULKAN_PATH: ${VULKAN_PATH}, please specify correct path.
		For more information on correct installation process, please refer to subsection 'Getting started with Lunar-G SDK'
		and 'Setting up first project with CMake' in Chapter 3, 'Shaking hands with the device' in this book 'Learning Vulkan', ISBN - 9781786469809.")
	   return()
	endif()
endif()

# Specify a suitable project name
project(VulkanCore)

# Add any required preprocessor definitions here
# Windows에서는 WSI에 VK_KHR_WIN32_SURFAACE_EXTENSION_NAME 확장자가 필요하므로 add_definitions()를 사용해 전처리기 지시문을 정의함
if(WIN32)
	add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
endif(WIN32)

# Vulkan  응용 프로그램을 빌드하기 위해 vulkan-1 라이브러리 추가
set(VULKAN_LIB_LIST "vulkan-1")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	# Vulkan SDK로 부터 헤더 파일을 포함
	include_directories(AFTER ${VULKAN_PATH}/Include)

	# vulkan-1을 위한 링크 디렉터리
	link_directories(${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;)
endif()

# 헤더 파일과 소스 파일들을 함께 묶음
if(WIN32)
    source_group("include" REGULAR_EXPRESSION "include/*")
    source_group("source" REGULAR_EXPRESSION "source/*")
endif(WIN32)

# include 경로 정의
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 컴파일을 위한 헤더와 소스 파일 목록 모으기
file(GLOB_RECURSE CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
file(GLOB_RECURSE HPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.*)

# 프로젝트 빌드, 컴파일될 파일명과 include 파일
add_executable(${PROJECT_NAME} ${CPP_FILES} ${HPP_FILES})

# 프로젝트에 디버그와 릴리스 라이브러리 링크
target_link_libraries( ${PROJECT_NAME} ${VULKAN_LIB_LIST} )

# 프로젝트 컴파일에 사용할 프로젝트 등록 정보와 올바른 C, C++ 표준 버전을 지정한다...
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/binaries)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 99)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD_REQUIRED ON)
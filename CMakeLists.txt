cmake_minimum_required(VERSION 3.21)
include(ExternalProject)

project(ARTest)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../Rayne/CMake/Rayne.cmake)
set(CMAKE_XCODE_GENERATE_TOP_LEVEL_PROJECT_ONLY ON)

option(RN_BUILD_BULLET_MODULE "Build the Bullet physics module" OFF)
option(RN_BUILD_PHYSX_MODULE "Build the PhysX physics module" OFF)
option(RN_BUILD_JOLT_MODULE "Build the Jolt physics module" ON)
option(RN_BUILD_ODE_MODULE "Build the ODE physics module" OFF)
option(RN_BUILD_SPLASH_MODULE "Build the Splash physics module" OFF)
option(RN_BUILD_ASSIMP_MODULE "Build the Assimp module" OFF)
option(RN_BUILD_STEAMAUDIO_MODULE "Build the Steam Audio module" OFF)
option(RN_BUILD_OCULUSAUDIO_MODULE "Build the Oculus Audio module" OFF)
option(RN_BUILD_RESONANCEAUDIO_MODULE "Build the Resonance Audio module" OFF)
option(RN_BUILD_ENET_MODULE "Build the ENet module" OFF)
option(RN_BUILD_SAMPLES "Enable building the sample projects" OFF)
option(RN_BUILD_RECAST_MODULE "Build the Recast pathfinding module" OFF)
option(RN_BUILD_OGG_MODULE "Build the ogg audio file module" OFF)
option(RN_BUILD_OPENAL_MODULE "Build the OpenAL module" OFF)
option(RN_BUILD_UI_MODULE "Build the UI module" ON)
option(RN_BUILD_OPENXR_MODULE "Build the OpenXR module" ON)
option(RN_BUILD_APPLEXR_MODULE "Build the AppleXR module" ON)
option(RN_BUILD_VRSTUB_MODULE "Build the VRStub module" ON)

set(
	RN_BUILD_CONFIGURATION
	"independent"
	CACHE STRING
	"The build configuration."
)
set_property(
	CACHE RN_BUILD_CONFIGURATION
	PROPERTY STRINGS independent oculus steam pico
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Build)

if(IOS OR VISIONOS)
	enable_language(OBJCXX)
	enable_language(Swift)
endif()

if(IOS)
	set(CMAKE_OSX_DEPLOYMENT_TARGET 15.0)
elseif(VISIONOS)
	set(CMAKE_OSX_DEPLOYMENT_TARGET 1.0)
else()
	set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
endif()

add_subdirectory(
	"${CMAKE_CURRENT_SOURCE_DIR}/../../Rayne"
	"${CMAKE_CURRENT_BINARY_DIR}/Rayne"
)

if(WIN32)
	set(CMAKE_SYSTEM_VERSION "10.0.10240.0")
endif()

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
if(APPLE)
	if(IOS OR VISIONOS)
		set(CMAKE_INSTALL_RPATH @executable_path;@executable_path/Frameworks)
	else()
		set(
			CMAKE_INSTALL_RPATH
			@executable_path;@executable_path/../Resources/Modules/RayneJolt;@executable_path/../Resources/Modules/RayneMetal;@executable_path/../Resources/Modules/RayneOpenXR;@executable_path/../Resources/Modules/RayneVR
		)
	endif()
elseif(UNIX)
	set(
		CMAKE_INSTALL_RPATH
		\$ORIGIN;\$ORIGIN/Resources/Modules/RayneJolt;\$ORIGIN/Resources/Modules/RayneVulkan;\$ORIGIN/Resources/Modules/RayneOpenXR;\$ORIGIN/Resources/Modules/RayneVR
	)
endif()

set(
	SOURCES
	Sources/main.cpp
	Sources/ARTApplication.cpp
	Sources/ARTCameraManager.cpp
	Sources/ARTWorld.cpp
)

set(
	HEADERS
	Sources/ARTApplication.h
	Sources/ARTCameraManager.h
	Sources/ARTWorld.h
	Sources/ARTestLib.h
)

set(RESOURCES manifest.json Resources)

if(IOS OR VISIONOS)
	set(SOURCES_SWIFT Sources/main-apple.swift)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

if(ANDROID)
	add_library(ARTest SHARED "${SOURCES}" "${HEADERS}")
elseif(IOS OR VISIONOS)
	add_library(ARTestLib STATIC "${SOURCES}" "${HEADERS}")
	set_target_properties(ARTestLib PROPERTIES FRAMEWORK TRUE)
	set_target_properties(
		ARTestLib
		PROPERTIES MACOSX_FRAMEWORK_IDENTIFIER com.index.ARTestLib
	)
	set_property(TARGET ARTestLib PROPERTY PUBLIC_HEADER ${HEADERS})
	set_target_properties(ARTestLib PROPERTIES XCODE_ATTRIBUTE_DEFINES_MODULE YES)
	target_compile_options(ARTestLib PRIVATE -xobjective-c++)

	add_executable(ARTest "${SOURCES_SWIFT}")
	set_target_properties(
		ARTest
		PROPERTIES XCODE_ATTRIBUTE_SWIFT_OBJC_INTEROP_MODE objcxx
	)
	target_link_libraries(ARTest PUBLIC ARTestLib)
else()
	add_executable(ARTest "${SOURCES}" "${HEADERS}")
endif()

target_include_directories(ARTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Sources)

set(
	ADDITIONAL_RESOURCE_COPY_PARAMS
	"--resourcespec=${CMAKE_CURRENT_SOURCE_DIR}/resources.json"
)
rayne_copy_resources(ARTest "${RESOURCES}" "${ADDITIONAL_RESOURCE_COPY_PARAMS}")

rayne_link_with(ARTest)
rayne_use_modules(ARTest RayneJolt)
rayne_use_modules(ARTest RayneVR)

if(NOT APPLE)
	rayne_use_modules(ARTest RayneOpenXR)
endif()

if(VISIONOS)
	rayne_use_modules(ARTest RayneAppleXR)
endif()

if(NOT APPLE AND ${RN_BUILD_VULKAN_MODULE})
	rayne_use_modules(ARTest RayneVulkan)
endif()

if(APPLE AND ${RN_BUILD_METAL_MODULE})
	rayne_use_modules(ARTest RayneMetal)
endif()

if(${RN_BUILD_CONFIGURATION} STREQUAL "oculus")
	set(ART_COMPILE_DEFINITIONS ${ART_COMPILE_DEFINITIONS} -DBUILD_FOR_OCULUS)
elseif(${RN_BUILD_CONFIGURATION} STREQUAL "steam")
	set(ART_COMPILE_DEFINITIONS ${ART_COMPILE_DEFINITIONS} -DBUILD_FOR_STEAM)
elseif(${RN_BUILD_CONFIGURATION} STREQUAL "pico")
	set(ART_COMPILE_DEFINITIONS ${ART_COMPILE_DEFINITIONS} -DBUILD_FOR_PICO)
elseif(WIN32)
	add_custom_command(
		TARGET ARTest
		POST_BUILD
		COMMAND
			${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_SOURCE_DIR}/Other/RunPancake-windows.bat
			$<TARGET_FILE_DIR:ARTest>/RunPancake-windows.bat
	)
	add_custom_command(
		TARGET GRAB
		POST_BUILD
		COMMAND
			${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:GRAB>
			$<TARGET_RUNTIME_DLLS:GRAB>
		COMMAND_EXPAND_LISTS
	)
endif()

target_compile_definitions(ARTest PRIVATE ${ART_COMPILE_DEFINITIONS})

if(APPLE)
	if(IOS OR VISIONOS)
		set_target_properties(
			ARTest
			PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING 0.0.1
		)
		set_target_properties(ARTest PROPERTIES MACOSX_BUNDLE_BUNDLE_VERSION 1)
	endif()

	set_target_properties(ARTest PROPERTIES MACOSX_BUNDLE TRUE)
	set_target_properties(
		ARTest
		PROPERTIES
			MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Other/Info.plist.in
	)
	set_target_properties(
		ARTest
		PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER com.index.artest
	)

	set_target_properties(
		ARTest
		PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
	)
	set_target_properties(
		ARTest
		PROPERTIES XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "LDF4CM9KT3"
	)
endif()

if(MSVC)
	set_property(
		DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		PROPERTY VS_STARTUP_PROJECT ARTst
	)
endif()

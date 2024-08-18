cmake_minimum_required(VERSION 3.0...3.30.2)
project(
	LearnWebGPU # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)

# Add glfw
if (NOT EMSCRIPTEN)
	add_subdirectory(glfw)
else()
	# Emscripten has built-in support for GLFW but requires the `-sUSE_GLFW=3` link option:
	add_library(glfw INTERFACE)
	target_link_options(glfw INTERFACE -sUSE_GLFW=3)
endif()

# Include webgpu directory, to define the 'webgpu' target
add_subdirectory(WebGPU-distribution-dawn-6512)

# Add a little extension of glfw that connects it to WebGPU
add_subdirectory(glfw3webgpu)

# 设置要查找的文件夹路径
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# cpp 文件列表
set(CPP_FILES_LIST)
# h 文件列表
set(H_FILES_LIST)

file(GLOB_RECURSE NEW_CPP_FILES "${SOURCE_DIR}/engine/*.cpp")
list(APPEND CPP_FILES_LIST ${NEW_CPP_FILES})
file(GLOB_RECURSE NEW_CPP_FILES "${SOURCE_DIR}/utils/*.cpp")
list(APPEND CPP_FILES_LIST ${NEW_CPP_FILES})

# file(GLOB_RECURSE NEW_H_FILES "${SOURCE_DIR}/engine/*.h")
# list(APPEND H_FILES_LIST ${NEW_H_FILES})
# file(GLOB_RECURSE NEW_H_FILES "${SOURCE_DIR}/utils/*.h")
# list(APPEND H_FILES_LIST ${NEW_H_FILES})

# We specify that we want to create a target of type executable, called "App"
add_executable(App ${SOURCE_DIR}/main.cpp ${CPP_FILES_LIST} ${H_FILES_LIST})

# Add the 'webgpu' target as a dependency of our App
target_link_libraries(App PRIVATE glfw webgpu glfw3webgpu)

# The application's binary must find wgpu.dll or libwgpu.so at runtime,
# so we automatically copy it (it's called WGPU_RUNTIME_LIB in general)
# next to the binary.
target_copy_webgpu_binaries(App)

# Set some recommended properties to configure the version of C++ and treat warnings as errors
set_target_properties(App PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	COMPILE_WARNING_AS_ERROR OFF
)

# Also add more warnings, to minimize mistakes
if (MSVC)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
	target_compile_options(App PRIVATE /W4)
else()
	target_compile_options(App PRIVATE -Wall)
endif()

# Generate "schemes" for XCode (macOS) and enable shader debugging (will become handy later on)
if (XCODE)
	set_target_properties(App PROPERTIES
		XCODE_GENERATE_SCHEME ON
		XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal"
	)
endif()

# Options that are specific to Emscripten
if (EMSCRIPTEN)
	# Generate a full web page rather than a simple WebAssembly module
	set_target_properties(App PROPERTIES SUFFIX ".html")
endif()
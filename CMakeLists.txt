cmake_minimum_required(VERSION 3.0...3.30)
project(
	LearnWebGPU # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)

# Include webgpu directory, to define the 'webgpu' target
add_subdirectory(WebGPU-distribution-dawn-6512)

# We specify that we want to create a target of type executable, called "App"
add_executable(App src/main.cpp)

# Add the 'webgpu' target as a dependency of our App
target_link_libraries(App PRIVATE webgpu)

# The application's binary must find wgpu.dll or libwgpu.so at runtime,
# so we automatically copy it (it's called WGPU_RUNTIME_LIB in general)
# next to the binary.
target_copy_webgpu_binaries(App)

# Set some recommended properties to configure the version of C++ and treat warnings as errors
set_target_properties(App PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	COMPILE_WARNING_AS_ERROR ON
)

# Also add more warnings, to minimize mistakes
if (MSVC)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
	target_compile_options(App PRIVATE /W4)
else()
	target_compile_options(App PRIVATE -Wall -Wextra -pedantic)
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
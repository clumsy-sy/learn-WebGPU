.PHONY: all clean run emscripten check-input

all:
	@echo "No input parameters detected. Compiling with CMake..."
	# if [ ! -d "build" ]; then \
	# 		mkdir build; \
	# 		echo "Created build directory."; \
	# fi
	cmake -S . -B build -G Ninja
	cmake --build build --parallel 16

run:
	if [ -f "./build/App" ]; then \
			./build/App; \
	else \
			echo "Error: ./build/Debug/App not found."; exit 1; \
	fi

emscripten:
	@echo "Input parameter detected: EMSCRIPTEN. Compiling with EMSCRIPTEN support..."
	if [ ! -d "build_emscripten" ]; then \
			mkdir build_emscripten; \
			echo "Created build_emscripten directory."; \
	fi

	cmake -S . -B build_emscripten -DCMAKE_TOOLCHAIN_FILE=./emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
	cmake --build build_emscripten --parallel 16

check-input:
	@echo "Input parameters detected. Proceeding with other actions..."
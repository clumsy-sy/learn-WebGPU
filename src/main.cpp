// 下面的三个宏由 cmake 控制
// #define WEBGPU_BACKEND_DAWN
// #define WEBGPU_BACKEND_WGPU
// #define WEBGPU_BACKEND_EMSCRIPTEN

#ifdef WEBGPU_BACKEND_WGPU
#  include <webgpu/wgpu.h>
#endif // WEBGPU_BACKEND_WGPU

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__

#include "engine/application.h"

int main() {
	auto& app = webgpu::Application::GetInstance();

	try {
		app->Initialize();
	} catch (const std::runtime_error& e) {
		std::cerr << "Exception caught: " << e.what() << '\n';
	}

#ifdef __EMSCRIPTEN__
	// Equivalent of the main loop when using Emscripten:
	auto callback = [](void *arg) {
		//                   ^^^ 2. We get the address of the app in the callback.
		Application* pApp = reinterpret_cast<Application*>(arg);
		//                  ^^^^^^^^^^^^^^^^ 3. We force this address to be interpreted
		//                                      as a pointer to an Application object.
		pApp->MainLoop(); // 4. We can use the application object
	};
	emscripten_set_main_loop_arg(callback, &app, 0, true);
	//                                     ^^^^ 1. We pass the address of our application object.
#else // __EMSCRIPTEN__
	while (app->IsRunning()) {
		try {
			app->MainLoop();
    } catch (const std::runtime_error& e) {
			std::cerr << "Exception caught: " << e.what() << '\n';
			return -1;
    }
	}
#endif // __EMSCRIPTEN__

	return 0;
}

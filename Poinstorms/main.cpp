#include <iostream>
#include <functional>

#include "gl-cpp/gl-cpp.hpp"

#include "framework/drawable.hpp"
#include "framework/pointstorm.hpp"
#include "framework/pointmorph.hpp"
#include "demo.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"


const std::vector<kali::demo> DEMOS{
	kali::demo{ "Fibonacci Sphere (Point Storm)", []() { return kali::pointstorm::fib_sphere_demo(); }},
	kali::demo{ "Torus (Point Storm)", []() { return kali::pointstorm::torus_demo(); }},
	kali::demo{ "Mother's Day (Point Morph)", []() { return std::make_unique<kali::pointmorph>(kali::pointmorph{kali::generate_hmd_points()}); }},
	kali::demo{ "the_Points_Manifold", []() { return std::make_unique<kali::pointstorm>(); }, true},
};

int run_proglet(int argc, char** argv, std::unique_ptr<kali::drawable> proglet, kali::window& window) {

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window.handle, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	proglet->start(window);

	clock_t start_time = clock();
	size_t frame = 0;
	double fps_show_threshold = 0;
	double fps_show_interval = 1;

	int last_options_keystate = GLFW_RELEASE;
	int options_keystate = GLFW_RELEASE;
	bool show_options{ false };

	int last_demos_keystate = GLFW_RELEASE;
	int demos_keystate = GLFW_RELEASE;
	bool show_demos{ false };

	size_t fps_start = 0;

	glm::vec3 camera_position = proglet->default_camera_position();

	while (!glfwWindowShouldClose(window.handle)) {
		glfwPollEvents();
		window.update();

		glm::mat4 view = glm::lookAt(camera_position, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::mat4 projection = glm::perspective((float)std::numbers::pi / 2.0f, (float)window.width / (float)window.height, 0.01f, 100.0f);

		proglet->draw((clock() - start_time) / (float)CLOCKS_PER_SEC, window, frame, projection * view);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		last_options_keystate = options_keystate;
		options_keystate = glfwGetKey(window.handle, GLFW_KEY_O);
		if (last_options_keystate == GLFW_RELEASE && options_keystate == GLFW_PRESS) {
			if (show_options) {
				if (!ImGui::IsAnyItemActive()) {
					show_options = !show_options;
				}
			}
			else {
				show_options = !show_options;
			}
		}
		if (show_options) {
			ImGui::Begin("Options", &show_options);
			proglet->populate_options();
			ImGui::End();
		}

		last_demos_keystate = demos_keystate;
		demos_keystate = glfwGetKey(window.handle, GLFW_KEY_D);
		if (last_demos_keystate == GLFW_RELEASE && demos_keystate == GLFW_PRESS) {
			if (show_demos) {
				if (!ImGui::IsAnyItemActive()) {
					show_demos = !show_demos;
				}
			}
			else {
				show_demos = !show_demos;
			}
		}
		if (show_demos) {
			bool show_secrets = glfwGetKey(window.handle, GLFW_KEY_LEFT_SHIFT);
			ImGui::Begin("Demos", &show_demos, ImGuiWindowFlags_AlwaysAutoResize);
			size_t i = 0;
			for (const kali::demo& demo : DEMOS) {
				if (!demo.is_secret || show_secrets) {
					if (demo.demo_button(i)) {
						proglet = demo();
						start_time = clock();
						camera_position = proglet->default_camera_position();
						show_demos = false;
					}
				}

				i++;
			}
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window.handle);
		if (fps_show_threshold < glfwGetTime()) {
			fps_show_threshold += fps_show_interval;
			std::cout << (frame - fps_start) / fps_show_interval << "FPS\n";
			fps_start = frame;
		}
		frame++;
	}

	glfwTerminate();
	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	try {
		if (glfwInit() != GLFW_TRUE) throw std::runtime_error("glfw failed to intiialize");
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		kali::window window{ 1600, 900 };
		if (glewInit() != GLEW_OK) throw std::runtime_error("glew failed to initialize");
		glViewport(0, 0, 1600, 900);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		return run_proglet(argc, argv, DEMOS[0](), window);
	}
	catch (std::exception& ex) {
		std::cerr << "[X_X]: " << ex.what();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

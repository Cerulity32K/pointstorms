#include <iostream>

#include "gl-cpp/gl-cpp.hpp"

#include "framework/drawable.hpp"
#include "framework/pointstorm.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

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
	int last_key = GLFW_RELEASE;
	int key = GLFW_RELEASE;
	bool show_options{ false };
	size_t fps_start = 0;
	while (!glfwWindowShouldClose(window.handle)) {
		glfwPollEvents();
		window.update();

		glm::mat4 view = glm::lookAt(glm::vec3(1.5, 1.5, 1.5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::mat4 projection = glm::perspective((float)std::numbers::pi / 2.0f, 1600.0f / 900.0f, 0.01f, 100.0f);

		proglet->draw((clock() - start_time) / (float)CLOCKS_PER_SEC, window, frame, projection * view);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		last_key = key;
		key = glfwGetKey(window.handle, GLFW_KEY_O);
		if (last_key == GLFW_RELEASE && key == GLFW_PRESS) {
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

		return run_proglet(argc, argv, kali::pointstorm::demo(), window);
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

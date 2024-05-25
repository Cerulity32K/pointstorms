#include "gl-cpp.hpp"
#include "imgui/imgui.h"
#include <sstream>
#include <format>
#include <string>

namespace kali {
	const char* const uniform::glsl_type() const {
		return uniform_typename(data);
	}
	const char* const uniform_typename(const uniform::variant_type& variant) {
		return std::visit(uniform_visitor{
			[](glm::vec2) { return "vec2"; },
			[](glm::vec3) { return "vec3"; },
			[](glm::vec4) { return "vec4"; },
			[](glm::mat4) { return "mat4"; },
			[](glm::float32) { return "float"; },
			[](glm::uint) { return "uint"; },
		}, variant);
	}
	bool uniform::show_gui(size_t i) {
		std::stringstream namebuf;
		namebuf << name;
		std::string suffix = (std::stringstream() << "##uniform" << i).str();
		std::string tmp = namebuf.str() + suffix;
		const char* name_nomod = tmp.c_str();

		if (edit_allowed) {
			std::visit(uniform_visitor{
				[this, name_nomod](glm::vec2& v) { ImGui::DragFloat2(name_nomod, (float*)&v, 0.01); },
				[this, name_nomod](glm::vec3& v) { ImGui::DragFloat3(name_nomod, (float*)&v, 0.01); },
				[this, name_nomod](glm::vec4& v) { ImGui::DragFloat4(name_nomod, (float*)&v, 0.01); },
				[this, suffix, name_nomod](glm::mat4& v) {
					ImGui::DragFloat4((name + std::string("[0]") + suffix).c_str(), (float*)&v[0], 0.01);
					ImGui::DragFloat4((name + std::string("[1]") + suffix).c_str(), (float*)&v[1], 0.01);
					ImGui::DragFloat4((name + std::string("[2]") + suffix).c_str(), (float*)&v[2], 0.01);
					ImGui::DragFloat4((name + std::string("[3]") + suffix).c_str(), (float*)&v[3], 0.01);
				},
				[this, name_nomod](glm::float32& v) { ImGui::DragFloat(name_nomod, &v, 0.01); },
				[this, name_nomod](glm::uint& v) { ImGui::DragScalar(name_nomod, ImGuiDataType_U32, &v, 0.1); },
			}, data);
		}
		ImGui::SameLine();
		return ImGui::Button(std::format("X##uniform_delete{}", i).c_str());
	}

	window::window(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share)
		: handle(glfwCreateWindow(width, height, title, monitor, share)),
		width(width),
		height(height) {
		if (!handle) throw std::runtime_error("glfw window failed to open");
		glfwMakeContextCurrent(handle);
	}
	void window::update() {
		glfwGetFramebufferSize(handle, &width, &height);
		glViewport(0, 0, width, height);
	}
	window::~window() {
		if (handle) glfwDestroyWindow(handle);
	}

	shader::shader(const char* src, GLenum type) : binding(glCreateShader(type)) {
		glShaderSource(binding, 1, &src, nullptr);
		glCompileShader(binding);

		int success;
		glGetShaderiv(binding, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLint length;
			glGetShaderiv(binding, GL_INFO_LOG_LENGTH, &length);
			char* log = new char[length];
			glGetShaderInfoLog(binding, 512, nullptr, log);
			throw std::runtime_error(log);
		}
	}
	result<shader, std::runtime_error> try_new(const char* src, GLenum type) {
		try {
			return result<shader, std::runtime_error>::ok(std::move(shader{ src, type }));
		}
		catch (std::runtime_error& ex) {
			return result<shader, std::runtime_error>::err(ex);
		}
	}
	shader::shader(shader&& rhs) noexcept : binding(rhs.binding) {
		rhs.binding = 0;
	}
	shader& shader::operator=(shader&& rhs) noexcept {
		binding = rhs.binding;
		rhs.binding = 0;
		return *this;
	}
	shader::~shader() {
		if (binding) glDeleteShader(binding);
	}

	program::program() {}
	program::program(std::initializer_list<std::pair<const char*, GLenum>> shaders) : binding(glCreateProgram()) {
		std::vector<shader> cached_shaders;
		for (auto& [src, type] : shaders) {
			shader shader{ src, type };
			glAttachShader(binding, shader.binding);
			cached_shaders.push_back(std::move(shader));
		}
		glLinkProgram(binding);
	}
	program::program(std::initializer_list<std::pair<std::string, GLenum>> shaders) : binding(glCreateProgram()) {
		std::vector<shader> cached_shaders;
		for (auto& [src, type] : shaders) {
			shader shader{ src.c_str(), type};
			glAttachShader(binding, shader.binding);
			cached_shaders.push_back(std::move(shader));
		}
		glLinkProgram(binding);
	}
	program::program(shader* shaders, size_t count) : binding(glCreateProgram()) {
		for (size_t i = 0; i < count; i++) {
			glAttachShader(binding, shaders[i].binding);
		}
		glLinkProgram(binding);
	}

	program::~program() {
		if (binding) glDeleteProgram(binding);
	}
	program::program(program&& rhs) noexcept : binding(rhs.binding) {
		rhs.binding = 0;
	}
	program& program::operator=(program&& rhs) noexcept {
		binding = rhs.binding;
		rhs.binding = 0;
		return *this;
	}
	void program::use() {
		glUseProgram(binding);
	}
	GLint program::set_up_uniform(const char* name) {
		use();
		return glGetUniformLocation(binding, name);
	}
	void program::set_float_uniform(const char* name, glm::float32 value) {
		glUniform1f(set_up_uniform(name), value);
	}
	void program::set_uint_uniform(const char* name, glm::uint value) {
		glUniform1ui(set_up_uniform(name), value);
	}
	void program::set_vec2_uniform(const char* name, glm::vec2 value) {
		glUniform2f(set_up_uniform(name), value.x, value.y);
	}
	void program::set_vec3_uniform(const char* name, glm::vec3 value) {
		glUniform3f(set_up_uniform(name), value.x, value.y, value.z);
	}
	void program::set_vec4_uniform(const char* name, glm::vec4 value) {
		glUniform4f(set_up_uniform(name), value.x, value.y, value.z, value.w);
	}
	void program::set_mat4_uniform(const char* name, glm::mat4 value) {
		glUniformMatrix4fv(set_up_uniform(name), 1, GL_FALSE, glm::value_ptr(value));
	}
	void program::set_uniforms(std::vector<uniform>& uniforms) {
		for (uniform& uniform : uniforms) {
			std::visit(uniform_visitor{
				[this, uniform](glm::vec2 value) { set_vec2_uniform(uniform.name.c_str(), value); },
				[this, uniform](glm::vec3 value) { set_vec3_uniform(uniform.name.c_str(), value); },
				[this, uniform](glm::vec4 value) { set_vec4_uniform(uniform.name.c_str(), value); },
				[this, uniform](glm::mat4 value) { set_mat4_uniform(uniform.name.c_str(), value); },
				[this, uniform](glm::float32 value) { set_float_uniform(uniform.name.c_str(), value); },
				[this, uniform](glm::uint value) { set_uint_uniform(uniform.name.c_str(), value); },
			}, uniform.data);
		}
	}
}

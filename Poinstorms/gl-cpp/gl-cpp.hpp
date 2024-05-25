#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <variant>

namespace kali {
	template <typename T, typename E> struct result {
		std::variant<T, E> data;
		result(T value) : data(std::move(value)) {}
		result(E error) : data(std::move(error)) {}
		static result ok(T value) { return result{ std::move(value) }; }
		static result err(E error) { return result{ std::move(error) }; }

		T&& unwrap() {
			if (std::holds_alternative<T>(data)) {
				return std::move(std::get<T>(data));
			} else {
				throw std::move(std::get<E>(data));
			}
		}
	};

	template <class... T> struct uniform_visitor : T... { using T::operator()...; };

	struct uniform {
		bool edit_allowed{ false };
		std::string name;

		using variant_type =
			std::variant<
				glm::vec2,
				glm::vec3,
				glm::vec4,
				glm::mat4,
				glm::float32,
				glm::uint
			>;
		variant_type data;
		uniform(std::string name, variant_type data, bool edit_allowed = true) : name(name), data(data), edit_allowed(edit_allowed) {}
		template <typename T> uniform(std::string name, T data, bool edit_allowed) : name(name), data(data), edit_allowed(edit_allowed) {}
		template <typename T> static uniform make(std::string name, T data, bool edit_allowed = true) {
			return uniform{ name, data, edit_allowed };
		}
		const char* const glsl_type() const;
		bool show_gui(size_t i);
	};
	const char* const uniform_typename(const uniform::variant_type& variant);
	struct window {
		GLFWwindow* handle;
		int width, height;

		window(int width, int height, const char* title = "OpenGL Window", GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr);

		void update();

		window(const window& rhs) = delete;
		window& operator=(const window& rhs) = delete;
		~window();
	};

	template <typename T> struct vao {
		GLuint handle;
		GLuint index{ 0 };

		vao();
		void bind();
		void reset();
		void add_attribute(size_t size, size_t offset, GLenum type = GL_FLOAT, GLboolean normalized = GL_FALSE);

		vao(const vao& rhs) = delete;
		vao& operator=(const vao& rhs) = delete;
		vao(vao&& rhs);
		vao& operator=(vao&& rhs);
		~vao();
	};

	struct shader {
		GLuint binding{ 0 };
		shader(const char* src, GLenum type);
		result<shader, std::runtime_error> try_new(const char* src, GLenum type);

		shader(const shader& rhs) = delete;
		shader& operator=(const shader& rhs) = delete;
		shader(shader&& rhs) noexcept;
		shader& operator=(shader&& rhs) noexcept;
		~shader();
	};

	struct program {
		GLuint binding{ 0 };
		program();
		program(std::initializer_list<std::pair<const char*, GLenum>> shaders);
		program(std::initializer_list<std::pair<std::string, GLenum>> shaders);
		program(shader* shaders, size_t count);

		program(const program& rhs) = delete;
		program& operator=(const program& rhs) = delete;
		program(program&& rhs) noexcept;
		program& operator=(program && rhs) noexcept;
		~program();

		void use();

		GLint set_up_uniform(const char* name);
		void set_float_uniform(const char* name, glm::float32 value);
		void set_uint_uniform(const char* name, glm::uint value);
		void set_vec2_uniform(const char* name, glm::vec2 value);
		void set_vec3_uniform(const char* name, glm::vec3 value);
		void set_vec4_uniform(const char* name, glm::vec4 value);
		void set_mat4_uniform(const char* name, glm::mat4 value);
		void set_uniforms(std::vector<uniform>& uniforms);
	};

	template<typename T> struct buffer {
		GLuint binding{ 0 };

		buffer();
		buffer(const buffer& rhs) = delete;
		buffer& operator=(const buffer& rhs) = delete;
		buffer(buffer&& rhs);
		buffer& operator=(buffer&& rhs);
		~buffer();

		void bind(GLenum target);
		void set(GLenum target, T* data, size_t count, GLenum usage_hint);
	};
}

#include "gl-cpp_impl.tcc"

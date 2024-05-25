#pragma once
#include "drawable.hpp"
#include "shaders.hpp"
#include <sstream>
#include <numbers>
#include <optional>

namespace kali {

	static const char* womper_vss_body = R"GLSL(
	const float pi = 3.1415926535897;
	const float phi = 2.399963229728653;

	vec3 get() {
		float findex = float(gl_VertexID);
		float fcount = float(count);

		float norm = findex / fcount;
		float norm_theta = norm * pi * 2.0;

		float latitude = asin(-1.0 + 2.0 * findex / (fcount + 1.0));
		float longitude = phi * findex;

		float wompness = time * womp_speed * pi;

		return vec3(
			cos(longitude + wompness + norm_theta / 3.0 * swildriness) * cos(latitude) * radius,
			sin(longitude) * cos(latitude) * radius,
			sin(latitude + wompness / 10.0 * womplitude) * radius
		);
	}
    )GLSL";

	static const char* torus_vss_body = R"GLSL(
const float pi = 3.1415926535897;

vec3 get() {
	float period = gl_VertexID / float(count) * pi * 2;
	float amp = sin(period * turns) * thickness + radius;

	return vec3(
		sin(period) * amp,
		cos(period * turns) * thickness,
		cos(period) * amp
	);
}
)GLSL";

	static const char* pointstorm_fsh = R"GLSL(#version 330 core
    out vec4 output_color;
	uniform vec4 color;

    void main() {
        output_color = color;
    })GLSL";

	// `head` should define a `get()` function that returns a `vec3`.
	// a `mat4` uniform named `mvp` will be added; this is the transform matrix.
	// a `float` uniform named `time` will be added; this signifies time.
	std::string generate_pointstorm_vss(std::vector<uniform>& uniforms, const std::string& head);

	struct pointstorm : public drawable {
		program shader_program;
		std::string vss;
		std::string last_working_vss;
		vao<std::monostate> vao{};
		std::vector<uniform> uniforms;

		std::optional<std::string> vss_error;
		std::string new_uniform_name;
		size_t new_uniform_type{ 0 };

		size_t count{ 1000 };
		float rotation_speed{ 1 };
		
		pointstorm() {
			uniforms = {
				uniform::make<glm::float32>("womp_speed", 1),
				uniform::make<glm::float32>("womplitude", 1),
				uniform::make<glm::float32>("swildriness", 1),
				uniform::make<glm::float32>("radius", 1),
				uniform::make<glm::vec4>("color", glm::vec4{0.0, 1.0, 0.0, 1.0}),
			};
			vss = generate_pointstorm_vss(uniforms, womper_vss_body);
			shader_program = program({
				{ vss, GL_VERTEX_SHADER},
				{ std::string(pointstorm_fsh), GL_FRAGMENT_SHADER }
			});
		}
		pointstorm(std::string vss_body, std::vector<uniform>&& uniforms)
			: uniforms(uniforms), vss(generate_pointstorm_vss(uniforms, vss_body)) {

			shader_program = program({
				{ vss, GL_VERTEX_SHADER},
				{ std::string(pointstorm_fsh), GL_FRAGMENT_SHADER }
			});
		}

		static std::unique_ptr<drawable> demo() {
			return std::make_unique<kali::pointstorm>(kali::torus_vss_body, std::vector<kali::uniform>{
				kali::uniform::make<glm::float32>("turns", 40, true),
				kali::uniform::make<glm::float32>("thickness", 0.25, true),
				kali::uniform::make<glm::float32>("radius", 1, true),
				kali::uniform::make<glm::vec4>("color", glm::vec4{ 0.0, 1.0, 0.0, 1.0 }, true),
			});
		}

		void start(window& target) override;
		void draw(float delta, window& target, size_t frame, glm::mat4 transform) override;
		void populate_options() override;
		void recompile_program();
	};
}

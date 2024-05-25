#pragma once
#include "drawable.hpp"
#include "shaders.hpp"
#include "pointstorm.hpp"
#include <sstream>
#include <numbers>
#include <optional>

namespace kali {

	static const char* hmd_vss_body = R"GLSL(
	const float pi = 3.1415926535897;
	const float phi = 2.399963229728653;
	const float e = 2.718281828459045;

	layout(location = 0) in vec3 mdpos;

	vec3 fib_pos(float r) {
		float findex = float(gl_VertexID);
		float fcount = float(count);

		float norm = findex / fcount;
		float norm_theta = norm * pi * 2.0;

		float latitude = asin(-1.0 + 2.0 * findex / (fcount + 1.0));
		float longitude = phi * findex;

		return vec3(
			cos(longitude) * cos(latitude) * r,
			sin(longitude) * cos(latitude) * r,
			sin(latitude) * r
		);
	}

	float sigmoid(float x) {
		return 1.0 / (1.0 + pow(10, x));
	}

	vec3 get() {
		float norm = float(gl_VertexID) / count;
		float t = time - norm;

		float mt = (4 - t) * 4;

		float r = sigmoid((2 - time) * 2);
		return mix(fib_pos(r), mdpos, sigmoid(mt));
	}
    )GLSL";

	// `head` should define a `get()` function that returns a `vec3`.
	// a `mat4` uniform named `mvp` will be added; this is the transform matrix.
	// a `float` uniform named `time` will be added; this signifies time.
	std::vector<glm::vec3> generate_hmd_points();

	struct pointmorph : public drawable {
		program shader_program;
		std::string vss;
		std::string last_working_vss;
		std::vector<uniform> uniforms;

		std::optional<std::string> vss_error;
		std::string new_uniform_name;
		size_t new_uniform_type{ 0 };

		size_t count{ 1000 };
		float rotation_speed{ 1 };

		buffer<glm::vec3> vpos;
		vao<glm::vec3> vao{};

		pointmorph(std::vector<glm::vec3>&& points) {
			uniforms = {
				uniform::make<glm::float32>("womp_speed", 0),
				uniform::make<glm::float32>("womplitude", 1),
				uniform::make<glm::float32>("swildriness", 1),
				uniform::make<glm::float32>("radius", 1),
				uniform::make<glm::vec4>("color", glm::vec4{1.0, 0.0, 0.5, 1.0}),
			};
			vss = generate_pointstorm_vss(uniforms, hmd_vss_body);
			last_working_vss = vss;
			shader_program = program({
				{ vss, GL_VERTEX_SHADER},
				{ std::string(pointstorm_fsh), GL_FRAGMENT_SHADER }
			});

			vpos.set(GL_ARRAY_BUFFER, points.data(), points.size(), GL_STATIC_DRAW);
			count = points.size();
		}

		void start(window& target) override;
		void draw(float delta, window& target, size_t frame, glm::mat4 transform) override;
		void populate_options() override;
		glm::vec3 default_camera_position() override;

		void recompile_program();
	};
}

#include "pointmorph.hpp"

#include <numbers>
#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"

// included in vertex shader
/*std::vector<glm::vec3> fib_sphere(size_t count, float radius, float wompness, float womplitude) {
	std::vector<glm::vec3> out;
	out.reserve(count);
	
	float phi = ;
	for (size_t i = 0; i < count; i++) {
	}
	
	return out;
}*/

/*std::vector<glm::uint> count(size_t count) {
	std::vector<glm::uint> out;
	out.reserve(count);
	for (size_t i = 0; i < count; i++) {
		out.push_back(i);
	}
	return out;
}*/

namespace kali {

	static const char* const hmd_bigtext[3][5] = {
		{
			"H H AAA PPP PPP Y Y",
			"H H A A P P P P Y Y",
			"HHH AAA PPP PPP  Y ",
			"H H A A P   P    Y ",
			"H H A A P   P    Y ",
		},
		{
			"M   M OOO TTT H H EEE RRR SSS",
			"MM MM O O  T  H H E   R R S  ",
			"M M M O O  T  HHH EE  RR  SSS",
			"M   M O O  T  H H E   R R   S",
			"M   M OOO  T  H H EEE R R SSS",
		},
		{
			"DD  AAA Y Y !",
			"D D A A Y Y !",
			"D D AAA  Y  !",
			"D D A A  Y   ",
			"DD  A A  Y  !",
		},
	};

	std::vector<glm::vec3> generate_hmd_points() {
		std::vector<glm::vec3> out;

		size_t cube_width = 16;
		size_t cube_height = 16;
		size_t cube_depth = 1;
		size_t cube_points = cube_width * cube_height * cube_depth;

		// introspective reservation
		size_t count{ 0 };
		for (size_t major_index = 0; major_index < 3; major_index++) {
			for (size_t minor_index = 0; minor_index < 5; minor_index++) {
				for (size_t chindex = 0; hmd_bigtext[major_index][minor_index][chindex]; chindex++) {
					if (hmd_bigtext[major_index][minor_index][chindex] != ' ') {
						count += cube_points;
					}
				}
			}
		}
		out.reserve(count);

		// position generation and caching
		// this nesting pyramid is disgusting as hell
		for (size_t major_index = 0; major_index < 3; major_index++) {
			// line collection
			size_t start = out.size();
			for (size_t minor_index = 0; minor_index < 5; minor_index++) {
				for (size_t chindex = 0; hmd_bigtext[major_index][minor_index][chindex]; chindex++) {
					if (hmd_bigtext[major_index][minor_index][chindex] != ' ') {
						for (size_t x = 0; x < cube_width; x++){
							for (size_t y = 0; y < cube_height; y++) {
								for (size_t z = 0; z < cube_depth; z++) {
									out.push_back(glm::vec3{
										chindex * 0.1f + x / (float)cube_width * 0.1,
										-(minor_index * 0.1f) - major_index * 0.6 + y / (float)cube_width * 0.1,
										z / (float)cube_width * 0.1,
									});
								}
							}
						}
					}
				}
			}
			size_t end = out.size();
			// line centering and shifting
			glm::vec3 min{};
			glm::vec3 max{};
			for (size_t i = start; i < end; i++) {
				min = glm::min(min, out[i]);
				max = glm::max(max, out[i]);
			}
			float xshift = glm::mix(max, min, 0.5f).x;
			for (size_t i = start; i < end; i++) {
				out[i].x -= xshift;
			}
		}
		glm::vec3 min{};
		glm::vec3 max{};
		for (glm::vec3& v : out) {
			min = glm::min(min, v);
			max = glm::max(max, v);
		}
		glm::vec3 shift = glm::mix(max, min, 0.5);
		std::cout << shift.x << ", " << shift.y << ", " << shift.z;
		std::cout << min.x << ", " << min.y << ", " << min.z;
		std::cout << max.x << ", " << max.y << ", " << max.z;
		for (glm::vec3& v : out) {
			v -= shift;
		}

		return out;
	}

	glm::vec3 pointmorph::default_camera_position() { return glm::vec3(0, 0, 1); }
	void pointmorph::start(window& target) {}
	void pointmorph::draw(float time, window& target, size_t frame, glm::mat4 transform) {
		glClearColor(0, 0, 0, 255);
		glClear(GL_COLOR_BUFFER_BIT);

		vao.reset();
		vao.add_attribute(3, 0);
		
		shader_program.use();
		glm::mat4 model = glm::rotate(glm::identity<glm::mat4>(), (float)time * rotation_speed * 0, glm::vec3(0, 1, 0));

		shader_program.set_uniforms(uniforms);

		shader_program.set_mat4_uniform("mvp", transform * model);
		shader_program.set_float_uniform("time", time);
		shader_program.set_uint_uniform("count", count);

		vpos.bind(GL_ARRAY_BUFFER);
		
		glDrawArrays(GL_POINTS, 0, count);
	}
	void pointmorph::recompile_program() {
		try {
			program new_program = program({ {vss, GL_VERTEX_SHADER}, {std::string(pointstorm_fsh), GL_FRAGMENT_SHADER} });
			// at this point the shaders are good
			shader_program = std::move(new_program);
			last_working_vss = vss;
			vss_error = std::optional<std::string>();
		}
		catch (std::runtime_error& ex) {
			vss_error = ex.what();
		}
	}
	void pointmorph::populate_options() {
		{
			for (size_t i = 0; i < uniforms.size();) {
				if (uniforms[i].show_gui(i)) {
					uniforms.erase(uniforms.begin() + i);
				}
				else {
					i++;
				}
			}
		}
		ImGui::InputText("Uniform Name", &new_uniform_name);
		ImGui::SameLine();
		const size_t DEFAULT_UNIFORM_COUNT = 6;
		static const uniform::variant_type default_uniforms[DEFAULT_UNIFORM_COUNT]{
			uniform::variant_type::variant(glm::float32(0)),
			uniform::variant_type::variant(glm::uint(0)),
			uniform::variant_type::variant(glm::vec2(0)),
			uniform::variant_type::variant(glm::vec3(0)),
			uniform::variant_type::variant(glm::vec4(0)),
			uniform::variant_type::variant(glm::identity<glm::mat4>()),
		};
		if (ImGui::BeginCombo("##utype", uniform_typename(default_uniforms[new_uniform_type]))) {
			for (size_t i = 0; i < DEFAULT_UNIFORM_COUNT; i++) {
				if (ImGui::Selectable(uniform_typename(default_uniforms[i]))) {
					new_uniform_type = i;
				}
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("New Uniform")) {
			uniforms.push_back(uniform{new_uniform_name, default_uniforms[new_uniform_type]});
		}
		ImGui::Separator();
		ImGui::DragFloat("rotation speed", &rotation_speed, 0.01);
		glm::uint min = 0;
		glm::uint max = 1000000;
		ImGui::DragScalar("point count", ImGuiDataType_U32, &count, 1, &min, &max);
		ImGui::Separator();
		if (vss_error.has_value()) {
			ImGui::Text(vss_error.value().c_str());
			if (ImGui::Button("Restore to Last Working")) {
				vss = last_working_vss;
				vss_error = std::optional<std::string>();
			}
		}
		ImGui::Text("Point Shader");
		if (ImGui::InputTextMultiline("##shed", &vss, ImVec2{800, 450}, ImGuiInputTextFlags_AllowTabInput)) {
			recompile_program();
		}
		// TODO: add preset selection (e.g. womping sphere)
	}
}

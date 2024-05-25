#include "pointstorm.hpp"

#include <numbers>
#include "../imgui/imgui.h"
#include "../imgui/imgui_stdlib.h"

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

	std::string generate_pointstorm_vss(std::vector<uniform>& uniforms, const std::string& head) {
		std::stringstream output;
		output << "#version 330 core\n";
		output << "uniform mat4 mvp;\n";
		output << "uniform float time;\n";
		output << "uniform uint count;\n";
		for (uniform& uniform : uniforms) {
			output << "uniform " << uniform.glsl_type() << ' ' << uniform.name << ";\n";
		}
		output << head << "\nvoid main() { gl_Position = mvp * vec4(get(), 1.0); }";
		std::cout << output.str();
		return output.str();
	}

	void pointstorm::start(window& target) {}
	void pointstorm::draw(float time, window& target, size_t frame, glm::mat4 transform) {
		glClearColor(0, 0, 0, 255);
		glClear(GL_COLOR_BUFFER_BIT);

		vao.reset();
		vao.add_attribute(0, 0);
		
		shader_program.use();
		glm::mat4 model = glm::rotate(glm::identity<glm::mat4>(), (float)time * rotation_speed, glm::vec3(0, 1, 0));

		shader_program.set_uniforms(uniforms);

		shader_program.set_mat4_uniform("mvp", transform * model);
		shader_program.set_float_uniform("time", time);
		shader_program.set_uint_uniform("count", count);
		
		glDrawArrays(GL_POINTS, 0, count);
	}
	void pointstorm::recompile_program() {
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
	void pointstorm::populate_options() {
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
			}
		}
		ImGui::Text("Point Shader");
		if (ImGui::InputTextMultiline("##shed", &vss, ImVec2{800, 450}, ImGuiInputTextFlags_AllowTabInput)) {
			recompile_program();
		}
		// TODO: add preset selection (e.g. womping sphere)
	}
}

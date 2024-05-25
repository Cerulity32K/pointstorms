#pragma once

#include <functional>
#include <format>

#include "framework/drawable.hpp"

#include <imgui/imgui.h>

namespace kali {
	struct demo {
		std::function<std::unique_ptr<drawable>()> generator;
		const char* name;
		bool is_secret;

		demo(const char* name, std::function<std::unique_ptr<drawable>()> generator, bool is_secret = false)
			: generator(generator),
			name(name),
			is_secret(is_secret) {}

		bool demo_button(size_t i) const {
			return ImGui::Button(std::format("{}##demobutton{}", name, i).c_str());
		}
		std::unique_ptr<drawable> operator()() const {
			return generator();
		}
	};
}

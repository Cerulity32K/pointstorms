#pragma once

#include "../gl-cpp/gl-cpp.hpp"

namespace kali {
	struct drawable {
		virtual void start(window& target) = 0;
		virtual void draw(float delta, window& target, size_t frame, glm::mat4 transform) = 0;
		virtual void populate_options() = 0;
	};
}

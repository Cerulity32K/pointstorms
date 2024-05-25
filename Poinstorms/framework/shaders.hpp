#pragma once

namespace kali {
    static const char* basic_vsh = R"GLSL(#version 330 core
    layout (location = 0) in vec3 aPos;

    void main() {
        gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    })GLSL";

    static const char* mvp_vsh = R"GLSL(#version 330 core
    layout (location = 0) in vec3 pos;

	uniform mat4 mvp;

    void main() {
        gl_Position = mvp * vec4(pos.x, pos.y, pos.z, 1.0);
    })GLSL";

    static const char* basic_fsh = R"GLSL(#version 330 core
    out vec4 color;

    void main() {
        color = vec4(1.0, 0.5, 0.8, 1.0);
    })GLSL";
}

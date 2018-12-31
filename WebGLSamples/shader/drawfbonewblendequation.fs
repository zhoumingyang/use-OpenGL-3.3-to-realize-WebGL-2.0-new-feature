#version 330
#define FRAGCOLORLOCATION 0
precision highp float;
precision highp int;
uniform sampler2D diffuse;
in vec2 v_st;
layout(location = FRAGCOLORLOCATION) out vec4 color;
void main() {
    color = texture(diffuse, v_st);
}
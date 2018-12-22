#version 330
precision highp float;
precision highp int;
uniform sampler2D diffuse;
in vec2 uv;
out vec4 color;
void main() {
    color = texture(diffuse, uv);
}
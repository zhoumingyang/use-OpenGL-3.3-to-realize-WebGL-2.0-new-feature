#version 330
#define POSITIONLOCATION 0
precision highp float;
precision highp int;
uniform mat4 MVP;
layout(location = POSITIONLOCATION) in vec2 position;
void main() {
    gl_Position = MVP * vec4(position, 0.0, 1.0);
}
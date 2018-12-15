#version 330
#define POSITIONLOCATION 0
precision highp float;
precision highp int;
layout(location = POSITIONLOCATION) in vec2 pos;
void main() {
    gl_Position = vec4(pos, 0.0, 1.0);
}
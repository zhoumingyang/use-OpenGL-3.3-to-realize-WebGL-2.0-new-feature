#version 330
#define POSITIONLOCATION 0
precision highp float;
precision highp int;
layout(location = POSITIONLOCATION) in vec4 position;
void main() {
    gl_Position = position;
}

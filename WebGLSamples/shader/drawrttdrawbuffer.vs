#version 330
#define POSITION_LOCATION 0
precision highp float;
precision highp int;
layout(location = POSITION_LOCATION) in vec4 position;
void main() {
    gl_Position = position;
}

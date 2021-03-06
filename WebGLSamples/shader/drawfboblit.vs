#version 330
#define POSITIONLOCATION 0
#define TEXCOORDLOCATION 4
precision highp float;
precision highp int;
uniform mat4 MVP;
layout(location = POSITIONLOCATION) in vec2 position;
layout(location = TEXCOORDLOCATION) in vec2 texcoord;
out vec2 v_st;
void main() {
    v_st = texcoord;
    gl_Position = MVP * vec4(position, 0.0, 1.0);
}
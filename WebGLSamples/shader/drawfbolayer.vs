#version 330
#define POSITIONLOCATION 0
#define TEXCOORDLOCATION 1
precision highp float;
precision highp int;
uniform mat4 mvp;
layout(location = POSITIONLOCATION) in vec2 position;
layout(location = TEXCOORDLOCATION) in vec2 textureCoordinates;
out vec2 v_st;
void main() {
    v_st = textureCoordinates;
    gl_Position = mvp * vec4(position, 0.0, 1.0);
}

#version 330
#define POSITIONLOCATION 0
#define COLORLOCATION 1
precision highp float;
precision highp int;
layout(location = POSITIONLOCATION) in vec2 pos;
layout(location = COLORLOCATION) in vec3 color;
flat out vec3 vcolor;
void main() {
    vcolor = color;
    gl_Position = vec4(pos + vec2(float(gl_InstanceID) - 1.0, 0.0), 0.0, 1.0);
}

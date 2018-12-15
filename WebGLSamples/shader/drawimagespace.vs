#version 330
precision highp float;
precision highp int;
void main() {
    gl_Position = vec4(2.f * float(uint(gl_VertexID) % 2u) - 1.f, 2.f * float(uint(gl_VertexID) / 2u) - 1.f, 0.0, 1.0);
}

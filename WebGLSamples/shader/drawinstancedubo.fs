#version 330
precision highp float;
precision highp int;
layout(std140) uniform;
uniform Material {
    vec4 Diffuse[2];
}material;
flat in int instance;
flat in vec2 vpos;
void main() {
    gl_FragColor = material.Diffuse[instance % 2];
}

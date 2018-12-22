#version 330
precision highp float;
precision highp int;
uniform sampler2D diffuse;
in vec2 v_st;
void main() {
    gl_FragColor = texture(diffuse, v_st);
}
#version 330
precision highp float;
precision highp int;
flat in vec3 vcolor;
void main() {
    gl_FragColor = vec4(vcolor, 1.0);
}
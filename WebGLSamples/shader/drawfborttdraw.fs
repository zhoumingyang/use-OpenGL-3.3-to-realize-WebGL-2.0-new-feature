#version 330
precision highp float;
precision highp int;
uniform sampler2D depthMap;
in vec2 v_st;
void main() {
     vec3 depth = vec3(texture(depthMap, v_st).r);
     gl_FragColor = vec4(1.0-depth, 1.0);
}

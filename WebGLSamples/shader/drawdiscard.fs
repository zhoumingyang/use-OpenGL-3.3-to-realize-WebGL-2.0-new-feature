#version 330
precision highp float;
precision highp int;
uniform sampler2D diffuse;
in vec2 v_st;
out vec4 color;
void main() {
    if(length(v_st - 0.5) > 0.5) {
        discard;
    }
    color = texture(diffuse, v_st);
}

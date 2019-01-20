#version 330
precision highp float;
precision highp int;
precision highp sampler2D;
uniform sampler2D diffuse;
in vec2 v_uv;
in vec3 v_position;
void main() {
    vec4 color = texture(diffuse, v_uv);
    vec3 fdx = vec3(dFdx(v_position.x), dFdx(v_position.y), dFdx(v_position.z));
    vec3 fdy = vec3(dFdy(v_position.x), dFdy(v_position.y), dFdy(v_position.z));
    vec3 N = normalize(cross(fdx, fdy));
    gl_FragColor = mix(color, vec4(N, 1.0), 0.2);
}


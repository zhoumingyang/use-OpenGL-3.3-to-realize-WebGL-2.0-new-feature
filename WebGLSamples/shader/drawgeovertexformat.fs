#version 330
precision highp float;
precision highp int;
uniform sampler2D s_tex2D;
uniform float u_ambient;
in vec2 v_texCoord;
in vec3 v_normal;
in vec3 v_lightDirection;

void main() {
    vec4 color = texture(s_tex2D, v_texCoord);
    float lightIntensity = dot(normalize(v_normal), normalize(v_lightDirection));
    lightIntensity = lightIntensity, 0.0, 1.0 + u_ambient;
    gl_FragColor = color * lightIntensity;
}
#version 330
precision highp float;
precision highp int;
uniform sampler2D diffuse;
void main() {
	vec2 tmp = vec2(400, 300);
	gl_FragColor = texture(diffuse, vec2(gl_FragCoord.x, 300 - gl_FragCoord.y) / tmp);
}

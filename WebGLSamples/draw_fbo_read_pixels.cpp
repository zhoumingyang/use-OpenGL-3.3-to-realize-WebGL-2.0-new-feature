#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma comment(lib,"glew32.lib")
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define TEXTURE_RED 0
#define TEXTURE_GREEN 1
#define TEXTURE_BLUE 2
#define TEXTURE_MAX 3

struct ViewPoint {
	float x;
	float y;
	float z;
	float w;
};
std::vector<ViewPoint> viewport;

const char* layerVertexShaderFile = "./shader/drawfbolayer.vs";
const char* layerFragmentShaderFile = "./shader/drawfbolayer.fs";
const char* multiOutVertexShaderFile = "./shader/drawmultipleoutput.vs";
const char* multiOutFragmentShaderFile = "./shader/drawmultipleoutput.fs";

GLfloat positions[] = {
	-1.0, -1.0,
	1.0, -1.0,
	1.0,  1.0,
	1.0,  1.0,
	-1.0,  1.0,
	-1.0, -1.0
};

GLfloat texcoords[] = {
	0.0, 0.0,
	1.0, 0.0,
	1.0, 1.0,
	1.0, 1.0,
	0.0, 1.0,
	0.0, 0.0
};

GLuint multipleOutputProgram;
GLuint layerProgram;

GLuint multipleOutputUniformMvpLocation;
GLuint layerUniformMvpLocation;
GLuint layerUniformDiffuseLocation;
GLuint layerUniformLayerLocation;

GLuint vertexPosBuffer;
GLuint vertexTexBuffer;
GLuint multipleOutputVertexArray;
GLuint layerVertexArray;
GLuint texture;
GLuint frameBuffer;
GLenum drawBuffers[3];

bool readfile(std::string& shader, const char* fileName) {
	std::ifstream f(fileName);
	if (f.is_open()) {
		std::string line;
		while (getline(f, line)) {
			shader.append(line);
			shader.append("\n");
		}
		f.close();
		return true;
	}
	return false;
}

void attchshader(GLuint shaderProgram, const char* shaderCode, GLenum shaderType) {
	GLuint shaderObj = glCreateShader(shaderType);
	if (shaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", shaderType);
		exit(1);
	}
	const GLchar* p[1];
	p[0] = shaderCode;
	GLint lengths[1];
	lengths[0] = strlen(shaderCode);
	glShaderSource(shaderObj, 1, p, lengths);
	glCompileShader(shaderObj);
	GLint success;
	glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar err[1024];
		glGetShaderInfoLog(shaderObj, 1024, NULL, err);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", shaderType, err);
		exit(1);
	}
	glAttachShader(shaderProgram, shaderObj);
}

GLuint compileshaders(const char* vsfile, const char* fsfile) {
	GLuint shaderProgram = glCreateProgram();
	if (shaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}
	std::string vs, fs;
	if (!readfile(vs, vsfile)) {
		exit(1);
	}
	if (!readfile(fs, fsfile)) {
		exit(1);
	}
	attchshader(shaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);
	attchshader(shaderProgram, vs.c_str(), GL_VERTEX_SHADER);
	GLint success = 0;
	GLchar err[1024] = { 0 };
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (success == 0) {
		glGetProgramInfoLog(shaderProgram, sizeof(err), NULL, err);
		fprintf(stderr, "Error linking shader program: '%s'\n", err);
		exit(1);
	}
	glValidateProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, sizeof(err), NULL, err);
		fprintf(stderr, "Invalid shader program: '%s'\n", err);
		exit(1);
	}
	return shaderProgram;
}

void initViewport() {
	viewport.resize(TEXTURE_MAX);
	ViewPoint textureRed;
	textureRed.x = WINDOW_WIDTH / 2.0;
	textureRed.y = 0;
	textureRed.z = WINDOW_WIDTH / 2.0;
	textureRed.w = WINDOW_HEIGHT / 2.0;
	viewport[TEXTURE_RED] = textureRed;

	ViewPoint textureGreen;
	textureGreen.x = WINDOW_WIDTH / 2.0;
	textureGreen.y = WINDOW_HEIGHT / 2.0;
	textureGreen.z = WINDOW_WIDTH / 2.0;
	textureGreen.w = WINDOW_HEIGHT / 2.0;
	viewport[TEXTURE_GREEN] = textureGreen;

	ViewPoint textureBlue;
	textureBlue.x = 0.0;
	textureBlue.y = WINDOW_HEIGHT / 2.0;
	textureBlue.z = WINDOW_WIDTH / 2.0;
	textureBlue.w = WINDOW_HEIGHT / 2.0;
	viewport[TEXTURE_BLUE] = textureBlue;
}

int w = 16;
int h = 16;

void init() {
	initViewport();
	multipleOutputProgram = compileshaders(multiOutVertexShaderFile, multiOutFragmentShaderFile);
	layerProgram = compileshaders(layerVertexShaderFile, layerFragmentShaderFile);
	
	multipleOutputUniformMvpLocation = glGetUniformLocation(multipleOutputProgram, "mvp");
	layerUniformMvpLocation = glGetUniformLocation(layerProgram, "mvp");
	layerUniformDiffuseLocation = glGetUniformLocation(layerProgram, "diffuse");
	layerUniformLayerLocation = glGetUniformLocation(layerProgram, "layer");
	
	glGenBuffers(1, &vertexPosBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexPosBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vertexTexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexTexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &multipleOutputVertexArray);
	glBindVertexArray(multipleOutputVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, vertexPosBuffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenVertexArrays(1, &layerVertexArray);
	glBindVertexArray(layerVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, vertexPosBuffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexTexBuffer);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, w, h, 3, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
	drawBuffers[TEXTURE_RED] = GL_COLOR_ATTACHMENT0;
	drawBuffers[TEXTURE_GREEN] = GL_COLOR_ATTACHMENT1;
	drawBuffers[TEXTURE_BLUE] = GL_COLOR_ATTACHMENT2;

	glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, TEXTURE_RED);
	glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, texture, 0, TEXTURE_GREEN);
	glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, texture, 0, TEXTURE_BLUE);
	GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << status << std::endl;
		return;
	}
	glDrawBuffers(3, drawBuffers);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
	glUseProgram(multipleOutputProgram);
	GLfloat matrix[] = {
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	};
	glUniformMatrix4fv(multipleOutputUniformMvpLocation, 1, GL_FALSE, matrix);
	glBindVertexArray(multipleOutputVertexArray);
	glViewport(0, 0, w, h);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glUseProgram(layerProgram);
	glUniformMatrix4fv(layerUniformMvpLocation, 1, GL_FALSE, matrix);
	glUniform1i(layerUniformDiffuseLocation, 0);
	glBindVertexArray(layerVertexArray);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << status << std::endl;
		return;
	}
	for (int i = 0; i < TEXTURE_MAX; i++) {
		glUniform1i(layerUniformLayerLocation, i);
		glViewport(viewport[i].x, viewport[i].y, viewport[i].z, viewport[i].w);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	unsigned int *data1 = new unsigned int[w * h * 4];
	unsigned int *data2 = new unsigned int[w * h * 4];
	unsigned int *data3 = new unsigned int[w * h * 4];
	glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data1);
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data2);
	glReadBuffer(GL_COLOR_ATTACHMENT2);
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data3);
	if (data1 != NULL) {
		delete[] data1;
	}
	if (data2 != NULL) {
		delete[] data2;
	}
	if (data3 != NULL) {
		delete[] data3;
	}
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindVertexArray(0);
	glutSwapBuffers();
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutCreateWindow("demo");
	glutDisplayFunc(render);
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		getchar();
		return 1;
	}
	init();
	glutMainLoop();
	getchar();
	return 0;
}
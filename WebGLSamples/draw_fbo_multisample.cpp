#include <iostream>
#include <fstream>
#include <string>
#include <GL/glew.h>
#include <GL/glut.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma comment(lib,"glew32.lib")
#define PROGRAMTEXTURE 0
#define PROGRAMSPLASH 1
#define PROGRAMMAX 2

const char* vertexSplashFile = "./shader/drawsplash.vs";
const char* fragmentSplashFile = "./shader/drawsplash.fs";
const char* vertexShaderFile = "./shader/drawfbomultisample.vs";
const char* fragmentShaderFile = "./shader/drawfbomultisample.fs";
const char* filepath = "./res/Di-3d.png";

GLuint renderShaderProgram;
GLuint splashShaderProgram;
GLuint mvpLocationTexture;
GLuint mvpLocation;
GLuint diffuseLocation;
GLfloat data[36];

GLuint vertexDataBuffer;
GLuint vertexPosBuffer;
GLuint vertexTexBuffer;
GLuint texture;

GLuint renderFrameBuffer;
GLuint colorFrameBuffer;
GLuint colorRenderBuffer;

GLuint vertexArrays[2];

GLfloat positions[] = {
	-1.0, -1.0,
	1.0, -1.0,
	1.0,  1.0,
	1.0,  1.0,
	-1.0,  1.0,
	-1.0, -1.0
};

GLfloat texCoords[] = {
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0,
	1.0, 0.0,
	0.0, 0.0,
	0.0, 1.0
};

GLfloat IDENTITY[] = {
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
};

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

void init() {
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	renderShaderProgram = compileshaders(vertexShaderFile, fragmentShaderFile);
	splashShaderProgram = compileshaders(vertexSplashFile, fragmentSplashFile);
	mvpLocationTexture = glGetUniformLocation(renderShaderProgram, "MVP");
	mvpLocation = glGetUniformLocation(splashShaderProgram, "MVP");
	diffuseLocation = glGetUniformLocation(splashShaderProgram, "diffuse");
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << "err1: " << err << std::endl;
	}

	float angle = 0.0;
	float radius = 0.1;
	for (int i = 0; i < 18; i++) {
		angle = 3.1415926 * 2.0 * i / 18;
		data[2 * i] = radius * sin(angle);
		data[2 * i + 1] = radius * cos(angle);
	}
	err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << "err2: " << err << std::endl;
	}

	glGenBuffers(1, &vertexDataBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexDataBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &vertexPosBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexPosBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &vertexTexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexTexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << "err3: " << err << std::endl;
	}

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << "err4: " << err << std::endl;
	}

	glGenFramebuffers(1, &renderFrameBuffer);
	glGenFramebuffers(1, &colorFrameBuffer);

	glGenRenderbuffers(1, &colorRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, colorRenderBuffer);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, 800, 600);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << "err5: " << err << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, renderFrameBuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << "err6: " << err << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, colorFrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << "err7: " << err << std::endl;
	}

	glGenVertexArrays(2, vertexArrays);
	glBindVertexArray(vertexArrays[PROGRAMTEXTURE]);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexDataBuffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << "err8: " << err << std::endl;
	}

	glBindVertexArray(vertexArrays[PROGRAMSPLASH]);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexPosBuffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << "err9: " << err << std::endl;
	}

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vertexTexBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, renderFrameBuffer);
	GLfloat black[] = { 0.0, 0.0, 0.0, 1.0 };
	glClearBufferfv(GL_COLOR, 0, black);
	glUseProgram(renderShaderProgram);
	glBindVertexArray(vertexArrays[PROGRAMTEXTURE]);
	glUniformMatrix4fv(mvpLocationTexture, 1, GL_FALSE, IDENTITY);
	glDrawArrays(GL_LINE_LOOP, 0, 18);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, renderFrameBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, colorFrameBuffer);
	glClearBufferfv(GL_COLOR, 0, black);
	glBlitFramebuffer(0, 0, 800, 600, 0, 0, 800, 600, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(splashShaderProgram);
	glUniform1i(diffuseLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(vertexArrays[PROGRAMSPLASH]);
	GLfloat mvp[] = {
		8.0, 0.0, 0.0, 0.0,
		0.0, 8.0, 0.0, 0.0,
		0.0, 0.0, 8.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	};
	glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvp);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
	glUseProgram(0);
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
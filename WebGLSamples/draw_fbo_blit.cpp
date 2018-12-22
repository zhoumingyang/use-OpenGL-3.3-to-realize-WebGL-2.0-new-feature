#include <iostream>
#include <fstream>
#include <string>
#include <GL/glew.h>
#include <GL/glut.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma comment(lib,"glew32.lib")

const char* vertexShaderFile = "./shader/drawfboblit.vs";
const char* fragmentShaderFile = "./shader/drawfboblit.fs";
const char* filepath = "./res/Di-3d.png";

GLuint shaderProgram;
GLuint mvpLocation;
GLuint diffuseLocation;
GLuint vertexArray;
GLuint vertexPosBuffer;
GLuint vertexTexBuffer;

int width, height, components;
GLuint textureDiffuse;
GLuint textureColorBuffer;
GLuint framebufferRender;
GLuint framebufferResolve;

GLuint colorRenderbuffer;

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

GLfloat SCALE[] = {
	0.8, 0.0, 0.0, 0.0,
	0.0, 0.8, 0.0, 0.0,
	0.0, 0.0, 0.8, 0.0,
	0.0, 0.0, 0.0, 1.0
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

void compileshaders() {
	shaderProgram = glCreateProgram();
	if (shaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}
	std::string vs, fs;
	if (!readfile(vs, vertexShaderFile)) {
		exit(1);
	}
	if (!readfile(fs, fragmentShaderFile)) {
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
	glUseProgram(shaderProgram);
}

void init() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	mvpLocation = glGetUniformLocation(shaderProgram, "MVP");
	diffuseLocation = glGetUniformLocation(shaderProgram, "diffuse");

	glGenVertexArrays(1, &vertexArray);
	glGenBuffers(1, &vertexPosBuffer);
	glGenBuffers(1, &vertexTexBuffer);
	glBindVertexArray(vertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, vertexPosBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexTexBuffer);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), (GLvoid*)0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout <<"first: " <<err << std::endl;
	}

	unsigned char* data = stbi_load(filepath, &width, &height, &components, 0);
	GLenum imageType = components == 3 ? GL_RGB : GL_RGBA;
	glGenTextures(1, &textureDiffuse);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureDiffuse);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, imageType, width, height, 0, imageType, GL_UNSIGNED_BYTE, data);

	glGenTextures(1, &textureColorBuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, imageType, width, height, 0, imageType, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << "second: " << err << std::endl;
	}

	glGenFramebuffers(1, &framebufferRender);
	glGenFramebuffers(1, &framebufferResolve);

	glGenRenderbuffers(1, &colorRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, width, height);

	err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << "colorRenderbuffer: " << err << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, framebufferRender);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << "framebufferRender: " << err << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, framebufferResolve);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout <<"third: " <<err << std::endl;
	}
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgram);
	glUniform1i(diffuseLocation, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, framebufferRender);
	glViewport(0, 0, width, height);
	GLfloat color[] = { 0.3, 0.3, 0.3, 1.0 };
	glClearBufferfv(GL_COLOR, 0, color);

	glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, SCALE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureDiffuse);
	glBindVertexArray(vertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferRender);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferResolve);
	GLfloat redcolor[] = { 0.7, 0.0, 0.0, 1.0 };
	glClearBufferfv(GL_COLOR, 0, redcolor);

	int TILE = 4;
	int BORDER = 2;
	for (int j = 0; j < TILE; j++) {
		for (int i = 0; i < TILE; i++) {
			if ((i + j) % 2) {
				continue;
			}
			glBlitFramebuffer(0, 0, width, height,
				width / TILE * (i + 1) + BORDER, 
				width / TILE * (j + 1) + BORDER,
				height / TILE * (i + 0) - BORDER, 
				height / TILE * (j + 0) - BORDER,
				GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}

	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
	GLfloat backcolor[] = { 0.0, 0.0, 0.0, 1.0 };
	glClearBufferfv(GL_COLOR, 0, backcolor);

	glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, IDENTITY);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	glBindVertexArray(vertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

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
	compileshaders();
	init();
	glutMainLoop();
	getchar();
	return 0;
}
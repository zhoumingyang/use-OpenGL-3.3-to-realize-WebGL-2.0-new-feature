#include <fstream>
#include <string>
#include <vector>
#include "noise3D.h"
#include <GL/glew.h>
#include <GL/glut.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma comment(lib,"glew32.lib")
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

const char* vertexShaderFile = "./shader/drawtexturesrgb.vs";
const char* fragmentShaderFile = "./shader/drawtexturesrgb.fs";

GLuint program;
GLuint uniformMvpLocation;
GLuint uniformDiffuseLocation;

GLfloat positions[] = {
	-1.0, -1.0,
	1.0, -1.0,
	1.0,  1.0,
	1.0,  1.0,
	-1.0,  1.0,
	-1.0, -1.0
};
GLuint vertexPosBuffer;

GLfloat texcoords[] = {
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0,
	1.0, 0.0,
	0.0, 0.0,
	0.0, 1.0
};
GLuint vertexTexBuffer;

GLuint vertexArray;

const char* image = "./res/Di-3d.png";
int components, width, height;
GLuint texture;
unsigned char* data;

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
		getchar();
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
		getchar();
		exit(1);
	}
	glValidateProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, sizeof(err), NULL, err);
		fprintf(stderr, "Invalid shader program: '%s'\n", err);
		getchar();
		exit(1);
	}
	return shaderProgram;
}

void init() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	program = compileshaders(vertexShaderFile, fragmentShaderFile);
	uniformMvpLocation = glGetUniformLocation(program, "mvp");
	uniformDiffuseLocation = glGetUniformLocation(program, "material.diffuse");
	
	glGenVertexArrays(1, &vertexArray);
	glGenBuffers(1, &vertexPosBuffer);
	glGenBuffers(1, &vertexTexBuffer);

	glBindVertexArray(vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, vertexPosBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexTexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), 0);
	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << "err1: " << err << std::endl;
	}

	data = stbi_load(image, &width, &height, &components, 0);
	GLenum imageType = components == 3 ? GL_RGB : GL_RGBA;

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, imageType, width, height, 0, imageType, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << "err2: " << err << std::endl;
	}
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program);
	GLfloat matrix[] = {
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	};
	glUniformMatrix4fv(uniformMvpLocation, 1, GL_FALSE, matrix);
	glUniform1i(uniformDiffuseLocation, 0);
	glBindVertexArray(vertexArray);
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
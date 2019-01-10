#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>
#include "glm.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma comment(lib,"glew32.lib")
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

const char* vertexShaderFile = "./shader/drawgeovertexformat.vs";
const char* fragmentShaderFile = "./shader/drawgeovertexformat.fs";

GLuint program;
GLuint unifModel;
GLuint unifModelInvTrans;
GLuint unifViewProj;
GLuint unifLightPosition;
GLuint unifTex2D;
GLuint unifAmbient;

GLfloat positions[] = {
	-1.0, -1.0,  1.0,
	1.0, -1.0,  1.0,
	1.0,  1.0,  1.0,
	-1.0,  1.0,  1.0,

	// Back face
	-1.0, -1.0, -1.0,
	-1.0,  1.0, -1.0,
	1.0,  1.0, -1.0,
	1.0, -1.0, -1.0,

	// Top face
	-1.0,  1.0, -1.0,
	-1.0,  1.0,  1.0,
	1.0,  1.0,  1.0,
	1.0,  1.0, -1.0,

	// Bottom face
	-1.0, -1.0, -1.0,
	1.0, -1.0, -1.0,
	1.0, -1.0,  1.0,
	-1.0, -1.0,  1.0,

	// Right face
	1.0, -1.0, -1.0,
	1.0,  1.0, -1.0,
	1.0,  1.0,  1.0,
	1.0, -1.0,  1.0,

	// Left face
	-1.0, -1.0, -1.0,
	-1.0, -1.0,  1.0,
	-1.0,  1.0,  1.0,
	-1.0,  1.0, -1.0
};

GLhalf normals[] = {
	// Front face
	0, 0, -1,
	0, 0, -1,
	0, 0, -1,
	0, 0, -1,

	// Back face
	0, 0, 1,
	0, 0, 1,
	0, 0, 1,
	0, 0, 1,

	// Top face
	0, 1, 0,
	0, 1, 0,
	0, 1, 0,
	0, 1, 0,

	// Bottom face
	0, -1, 0,
	0, -1, 0,
	0, -1, 0,
	0, -1, 0,

	// Right face
	-1, 0, 0,
	-1, 0, 0,
	-1, 0, 0,
	-1, 0, 0,

	// Left face
	1, 0, 0,
	1, 0, 0,
	1, 0, 0,
	1, 0, 0
};

GLhalf texCoords[] = {
	0.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0,

	// Back face
	1.0, 1.0,
	1.0, 0.0,
	0.0, 0.0,
	0.0, 1.0,

	// Top face
	0.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0,

	// Bottom face
	0.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0,

	// Right face
	0.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0,

	// Left face
	0.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0
};

GLuint cubeVertexIndices[] = {
	0,  1,  2,      0,  2,  3,    // front
	4,  5,  6,      4,  6,  7,    // back
	8,  9,  10,     8,  10, 11,   // top
	12, 13, 14,     12, 14, 15,   // bottom
	16, 17, 18,     16, 18, 19,   // right
	20, 21, 22,     20, 22, 23    // left
};

GLuint vertexPosBuffer;
GLuint vertexNorBuffer;
GLuint vertexTexBuffer;
GLuint indexBuffer;
GLuint vertexArray;

int width, height, components;
const char* filepath = "./res/Di-3d.png";
GLuint texture;

glm::mat4 modelMatrix;
glm::vec3 translate;
glm::mat4 viewMatrix;
glm::mat4 perspectiveMatrix;
glm::mat4 viewProj;
glm::mat4 modelInvTrans ;
glm::vec3 lightPosition(0.0f, 0.0f, 5.0f);

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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	program = compileshaders(vertexShaderFile, fragmentShaderFile);
	unifModel = glGetUniformLocation(program, "u_model");
	unifModelInvTrans = glGetUniformLocation(program, "u_modelInvTrans");
	unifViewProj = glGetUniformLocation(program, "u_viewProj");
	unifLightPosition = glGetUniformLocation(program, "u_lightPosition");
	unifTex2D = glGetUniformLocation(program, "s_tex2D");
	unifAmbient = glGetUniformLocation(program, "u_ambient");

	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);
	glGenBuffers(1, &vertexPosBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexPosBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vertexNorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexNorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_HALF_FLOAT, GL_FALSE, 3 * sizeof(GL_HALF_FLOAT), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vertexTexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexTexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_HALF_FLOAT, GL_FALSE, 2 * sizeof(GL_HALF_FLOAT), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeVertexIndices), cubeVertexIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << "err: " << err << std::endl;
	}

	unsigned char* data = stbi_load(filepath, &width, &height, &components, 0);
	GLenum imageType = components == 3 ? GL_RGB : GL_RGBA;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexImage2D(GL_TEXTURE_2D, 0, imageType, width, height, 0, imageType, GL_UNSIGNED_BYTE, data);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

	modelMatrix = glm::mat4(1.0);
	translate = glm::vec3(0.0f, 0.0f, -10.0f);
	viewMatrix = glm::translate(modelMatrix, translate);
	perspectiveMatrix = glm::perspective(0.785f, 1.0f, 1.0f, 1000.0f);
	modelInvTrans = glm::transpose(modelMatrix);
	modelInvTrans = glm::inverse(modelInvTrans);
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	viewMatrix = glm::rotate(viewMatrix, float(0.0050 * 3.1415926), glm::vec3(1.0, 0.0, 0.0));
	viewMatrix = glm::rotate(viewMatrix, float(0.0030 * 3.1415926), glm::vec3(0.0, 1.0, 0.0));
	viewMatrix = glm::rotate(viewMatrix, float(0.0009 * 3.1415926), glm::vec3(0.0, 0.0, 1.0));
	glBindVertexArray(vertexArray);
	glUseProgram(program);
	glUniformMatrix4fv(unifModel, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(unifModelInvTrans, 1, GL_FALSE, glm::value_ptr(modelInvTrans));
	viewProj = perspectiveMatrix * viewMatrix;
	glUniformMatrix4fv(unifViewProj, 1, GL_FALSE, glm::value_ptr(viewProj));
	glUniform3f(unifLightPosition, lightPosition.x, lightPosition.y, lightPosition.z);
	glUniform1i(unifTex2D, 0);
	glUniform1f(unifAmbient, 0.1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, cubeVertexIndices, 1);
	glUseProgram(0);
	glBindVertexArray(0);
	glutPostRedisplay();
	glutSwapBuffers();
}

void idle() {
	glutPostRedisplay();
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
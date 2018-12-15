#include <iostream>
#include <fstream>
#include <string>
#include <GL/glew.h>
#include <GL/glut.h>
#pragma comment(lib,"glew32.lib")

const char* vertexShaderFile = "./shader/drawinstancedubo.vs";
const char* fragmentShaderFile = "./shader/drawinstancedubo.fs";
GLuint vertexPosBuffer;
GLuint shaderProgram;
GLuint uniformTransformBuffer;
GLuint uniformMaterialBuffer;

GLfloat vertices[] = {
	-0.3, -0.5,
	0.3, -0.5,
	0.0, 0.5
};

GLfloat transforms[] = {
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	-0.5, 0.0, 0.0, 1.0,

	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.0, 0.0, 1.0
};

GLfloat materials[] = {
	1.0, 0.5, 0.0, 1.0,
	0.0, 0.5, 1.0, 1.0
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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glGenBuffers(1, &vertexPosBuffer);
	glGenBuffers(1, &uniformTransformBuffer);
	glGenBuffers(1, &uniformMaterialBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, vertexPosBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	
	GLuint uniformTransformLocation = glGetUniformBlockIndex(shaderProgram, "Transform");
	GLuint uniformMaterialLocation = glGetUniformBlockIndex(shaderProgram, "Material");
	glUniformBlockBinding(shaderProgram, uniformTransformLocation, 0);
	glUniformBlockBinding(shaderProgram, uniformMaterialLocation, 1);
	
	glBindBuffer(GL_UNIFORM_BUFFER, uniformTransformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(transforms), transforms, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBuffer(GL_UNIFORM_BUFFER, uniformMaterialBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(materials), materials, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << err << std::endl;
	}
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformTransformBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, uniformMaterialBuffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 2);
	glutSwapBuffers();
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
	compileshaders();
	init();
	glutMainLoop();
	getchar();
	return 0;
}
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma comment(lib,"glew32.lib")
#define CORNERS_TOP_LEFT 0
#define CORNERS_TOP_RIGHT 1
#define CORNERS_BOTTOM_RIGHT 2
#define CORNERS_BOTTOM_LEFT 3
#define CORNERS_MAX 4
//800, 600
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

struct ViewPoint {
	float x;
	float y;
	float z;
	float w;
};
std::vector<ViewPoint> viewport;

GLfloat positions[] = {
	-1.0, -1.0,
	1.0, -1.0,
	1.0,  1.0,
	1.0,  1.0,
	-1.0,  1.0,
	-1.0, -1.0
};

GLfloat texcoords[] = {
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0,
	1.0, 0.0,
	0.0, 0.0,
	0.0, 1.0
};

const char* vertexShaderFile = "./shader/drawfbonewblendequation.vs";
const char* fragmentShaderFile = "./shader/drawfbonewblendequation.fs";
const char* filepath = "./res/Di-3d.png";
GLuint shaderProgram;
GLuint vertexPosBuffer;
GLuint vertexTexBuffer;
GLuint vertexArray;
GLuint texture;
GLuint uniformMvpLocation;
GLuint uniformDiffuseLocation;

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
	viewport.resize(CORNERS_MAX);
	ViewPoint bottomleft;
	bottomleft.x = 0.0;
	bottomleft.y = 0.0; 
	bottomleft.z = WINDOW_WIDTH / 2.0;
	bottomleft.w = WINDOW_HEIGHT / 2.0;
	viewport[CORNERS_BOTTOM_LEFT] = bottomleft;

	ViewPoint bottomRight;
	bottomRight.x = WINDOW_WIDTH / 2.0;
	bottomRight.y = 0;
	bottomRight.z = WINDOW_WIDTH / 2.0;
	bottomRight.w = WINDOW_HEIGHT / 2.0;
	viewport[CORNERS_BOTTOM_RIGHT] = bottomRight;

	ViewPoint topRight;
	topRight.x = WINDOW_WIDTH / 2.0;
	topRight.y = WINDOW_HEIGHT / 2.0;
	topRight.z = WINDOW_WIDTH / 2.0;
	topRight.w = WINDOW_HEIGHT / 2.0;
	viewport[CORNERS_TOP_RIGHT] = topRight;

	ViewPoint topLeft;
	topLeft.x = 0.0;
	topLeft.y = WINDOW_HEIGHT / 2.0;
	topLeft.z = WINDOW_WIDTH / 2.0;
	topLeft.w = WINDOW_HEIGHT / 2.0;
	viewport[CORNERS_TOP_LEFT] = topLeft;
}

void init() {
	glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
	initViewport();
	shaderProgram = compileshaders(vertexShaderFile, fragmentShaderFile);
	uniformMvpLocation = glGetUniformLocation(shaderProgram, "mvp");
	uniformDiffuseLocation = glGetUniformLocation(shaderProgram, "diffuse");

	glGenBuffers(1, &vertexPosBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexPosBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vertexTexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexTexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, vertexPosBuffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexTexBuffer);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), 0);
	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	int width, height, components;
	unsigned char* data = stbi_load(filepath, &width, &height, &components, 0);
	GLenum imageType = components == 3 ? GL_RGB : GL_RGBA;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, imageType, width, height, 0, imageType, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgram);
	GLfloat matrix[] = {
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	};
	glUniformMatrix4fv(uniformMvpLocation, 1, GL_FALSE, matrix);
	glUniform1i(uniformDiffuseLocation, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(vertexArray);
	glEnable(GL_BLEND);

	for (int i = 0; i < CORNERS_MAX; i++) {
		glViewport(viewport[i].x, viewport[i].y, viewport[i].z, viewport[i].w);
		if (i == CORNERS_TOP_LEFT) {

		}
		else if (i == CORNERS_TOP_RIGHT) {
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		else if (i == CORNERS_BOTTOM_RIGHT) {
			glBlendEquation(GL_MIN);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		else if (i == CORNERS_BOTTOM_LEFT) {
			glBlendEquation(GL_MAX);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}
	glDisable(GL_BLEND);
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
	init();
	glutMainLoop();
	getchar();
	return 0;
}
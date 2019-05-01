#include <windows.h>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <stdlib.h>
#include "noise3D.h"
#include <GL/glew.h>
#include <GL/glut.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma comment(lib,"glew32.lib")
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

const char* emitVertexShaderFile = "./shader/drawtransformfeedbackinstancedemit.vs";
const char* emitFragmentShaderFile = "./shader/drawtransformfeedbackinstancedemit.fs";
const char* vertexShaderFile = "./shader/drawtransformfeedbackinstanced.vs";
const char* fragmentShaderFile = "./shader/drawtransformfeedbackinstanced.fs";

GLuint PROGRAM_TRANSFORM = 0;
GLuint PROGRAM_DRAW = 1;

GLuint programs[2];
GLuint programTransform;
GLuint programDraw;

GLuint NUM_INSTANCES = 1000;
GLuint currentSourceIdx = 0;

GLuint instanceOffsets[2000];
GLuint instanceRotations[1000];
GLuint instanceColors[3000];

GLuint OFFSET_LOCATION = 0;
GLuint ROTATION_LOCATION = 1;
GLuint POSITION_LOCATION = 2;
GLuint COLOR_LOCATION = 3;
GLuint NUM_LOCATIONS = 4;

GLfloat trianglePositions[] = {
	0.015, 0.0,
	-0.010, 0.010,
	-0.010, -0.010,
};

GLuint drawTimeLocation;
GLuint vertexArrays[2];
GLuint vertexBuffers[2][4];
GLuint transformFeedbacks[2];

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

float random() {
	return rand() / double(RAND_MAX);
}

void init() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	programTransform = compileshaders(emitVertexShaderFile, emitFragmentShaderFile);
	programDraw = compileshaders(vertexShaderFile, fragmentShaderFile);
	programs[0] = programTransform;
	programs[1] = programDraw;

	for (int i = 0; i < NUM_INSTANCES; i++) {
		int oi = i * 2;
		int ri = i;
		int ci = i * 3;

		instanceOffsets[oi] = random() * 2.0 - 1.0;
		instanceOffsets[oi + 1] = random() * 2.0 - 1.0;

		instanceRotations[i] = random() * 2 * 3.1415926;

		instanceColors[ci] = random();
		instanceColors[ci + 1] = random();
		instanceColors[ci + 2] = random();
	}

	drawTimeLocation = glGetUniformLocation(programs[PROGRAM_DRAW], "u_time");

	glGenVertexArrays(1, &vertexArrays[0]);
	glGenVertexArrays(1, &vertexArrays[1]);

	glGenTransformFeedbacks(1, &transformFeedbacks[0]);
	glGenTransformFeedbacks(1, &transformFeedbacks[1]);

	for (int va = 0;va < 2; ++va) {
		glBindVertexArray(vertexArrays[va]);
		glGenBuffers(1, &vertexBuffers[va][OFFSET_LOCATION]);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[va][OFFSET_LOCATION]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(instanceOffsets), instanceOffsets, GL_STREAM_COPY);
		glVertexAttribPointer(OFFSET_LOCATION, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), 0);
		glEnableVertexAttribArray(OFFSET_LOCATION);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &vertexBuffers[va][ROTATION_LOCATION]);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[va][ROTATION_LOCATION]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(instanceRotations), instanceRotations, GL_STREAM_COPY);
		glVertexAttribPointer(ROTATION_LOCATION, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(GL_FLOAT), 0);
		glEnableVertexAttribArray(ROTATION_LOCATION);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &vertexBuffers[va][POSITION_LOCATION]);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[va][POSITION_LOCATION]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(trianglePositions), trianglePositions, GL_STATIC_DRAW);
		glVertexAttribPointer(POSITION_LOCATION, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), 0);
		glEnableVertexAttribArray(POSITION_LOCATION);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &vertexBuffers[va][COLOR_LOCATION]);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[va][COLOR_LOCATION]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(instanceColors), instanceColors, GL_STATIC_DRAW);
		glVertexAttribPointer(COLOR_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), 0);
		glEnableVertexAttribArray(COLOR_LOCATION);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedbacks[va]);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vertexBuffers[va][OFFSET_LOCATION]);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, vertexBuffers[va][ROTATION_LOCATION]);

		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			std::cout << va <<"err: " << err << std::endl;
		}
	}
}

void transform() {
	GLuint programTransform = programs[PROGRAM_TRANSFORM];
	int destinationIdx = (currentSourceIdx + 1) % 2;

	GLuint sourceVAO = vertexArrays[currentSourceIdx];
	GLuint destinationTransformFeedback = transformFeedbacks[destinationIdx];

	glUseProgram(programTransform);
	glBindVertexArray(sourceVAO);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, destinationTransformFeedback);

	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vertexBuffers[destinationIdx][OFFSET_LOCATION]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vertexBuffers[destinationIdx][ROTATION_LOCATION]);

	glVertexAttribDivisor(OFFSET_LOCATION, 0);
	glVertexAttribDivisor(ROTATION_LOCATION, 1);

	glEnable(GL_RASTERIZER_DISCARD);

	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, NUM_INSTANCES);
	glEndTransformFeedback();

	glDisable(GL_RASTERIZER_DISCARD);
	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

	currentSourceIdx = (currentSourceIdx + 1) % 2;
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT);
	transform();

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT - 10);
	glBindVertexArray(vertexArrays[currentSourceIdx]);

	glVertexAttribDivisor(OFFSET_LOCATION, 1);
	glVertexAttribDivisor(ROTATION_LOCATION, 1);

	glUseProgram(programs[PROGRAM_DRAW]);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_NONE);

	float now;
	now = time(NULL);
	glUniform1f(drawTimeLocation, now);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 3, NUM_INSTANCES);
	glutPostRedisplay();
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
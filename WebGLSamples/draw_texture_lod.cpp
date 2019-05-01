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
#define CORNERS_TOP_LEFT 0
#define CORNERS_TOP_RIGHT 1
#define CORNERS_BOTTOM_RIGHT 2
#define CORNERS_BOTTOM_LEFT 3
#define CORNERS_MAX 4

const char* vertexShaderFile = "./shader/drawtexturelod.vs";
const char* fragmentShaderFile = "./shader/drawtexturelod.fs";

struct ViewPoint {
	float x, y, z, w;
};

ViewPoint viewport[CORNERS_MAX];

GLuint program;
GLuint uniformMvpLocation;
GLuint uniformDiffuseLocation;
GLuint uniformLodBiasLocation;

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
GLuint textures[CORNERS_MAX];
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

	ViewPoint bl;
	bl.x = 0.0;
	bl.y = 0.0;
	bl.z = float(WINDOW_WIDTH) / 2.0;
	bl.w = float(WINDOW_HEIGHT) / 2.0;
	viewport[CORNERS_BOTTOM_LEFT] = bl;

	ViewPoint br;
	br.x = float(WINDOW_WIDTH) / 2.0;
	br.y = 0.0;
	br.z = float(WINDOW_WIDTH) / 2.0;
	br.w = float(WINDOW_HEIGHT) / 2.0;
	viewport[CORNERS_BOTTOM_RIGHT] = br;

	ViewPoint tr;
	tr.x = float(WINDOW_WIDTH) / 2.0;
	tr.y = float(WINDOW_HEIGHT) / 2.0;
	tr.z = float(WINDOW_WIDTH) / 2.0;
	tr.w = float(WINDOW_HEIGHT) / 2.0;
	viewport[CORNERS_TOP_RIGHT] = br;

	ViewPoint tl;
	tl.x = 0.0;
	tl.y = float(WINDOW_HEIGHT) / 2.0;
	tl.z = float(WINDOW_WIDTH) / 2.0;
	tl.w = float(WINDOW_HEIGHT) / 2.0;
	viewport[CORNERS_TOP_LEFT] = tl;

	program = compileshaders(vertexShaderFile, fragmentShaderFile);
	uniformMvpLocation = glGetUniformLocation(program, "mvp");
	uniformDiffuseLocation = glGetUniformLocation(program, "diffuse");
	uniformLodBiasLocation = glGetUniformLocation(program, "lodBias");

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
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &textures[CORNERS_TOP_LEFT]);
	glBindTexture(GL_TEXTURE_2D, textures[CORNERS_TOP_LEFT]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, imageType, width, height, 0, imageType, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &textures[CORNERS_TOP_RIGHT]);
	glBindTexture(GL_TEXTURE_2D, textures[CORNERS_TOP_RIGHT]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 3.0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 3.0);
	glTexImage2D(GL_TEXTURE_2D, 0, imageType, width, height, 0, imageType, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &textures[CORNERS_BOTTOM_LEFT]);
	glBindTexture(GL_TEXTURE_2D, textures[CORNERS_BOTTOM_LEFT]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0.0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 10.0);
	glTexImage2D(GL_TEXTURE_2D, 0, imageType, width, height, 0, imageType, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &textures[CORNERS_BOTTOM_RIGHT]);
	glBindTexture(GL_TEXTURE_2D, textures[CORNERS_BOTTOM_RIGHT]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0.0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 10.0);
	glTexImage2D(GL_TEXTURE_2D, 0, imageType, width, height, 0, imageType, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

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

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(vertexArray);

	float lodBiasArray[4] = { 0.0, 0.0, 0.0, 0.0 };
	lodBiasArray[CORNERS_BOTTOM_LEFT] = 3.5;
	lodBiasArray[CORNERS_BOTTOM_RIGHT] = 4.0;
	for (int i = 0; i < CORNERS_MAX; i++) {
		glViewport(viewport[i].x, viewport[i].y, viewport[i].z, viewport[i].w);
		glUniform1f(uniformLodBiasLocation, lodBiasArray[i]);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
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
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

#define VIEW_BOTTOM_LEFT 0
#define VIEW_BOTTOM_CENTER 1
#define VIEW_BOTTOM_RIGHT 2
#define VIEW_MIDDLE_LEFT 3
#define VIEW_MIDDLE_CENTER 4
#define VIEW_MIDDLE_RIGHT 5
#define VIEW_TOP_LEFT 6
#define VIEW_TOP_CENTER 7
#define VIEW_TOP_RIGHT 8
#define VIEW_MAX 9

#define TEXTURE_TYPE_RGB 0
#define TEXTURE_TYPE_RGB8 1
#define TEXTURE_TYPE_RGBA 2
#define TEXTURE_TYPE_RGB16F 3
#define TEXTURE_TYPE_RGBA32F 4
#define TEXTURE_TYPE_R16F 5
#define TEXTURE_TYPE_RG16F 6
#define TEXTURE_TYPE_RGB8UI 7
#define TEXTURE_TYPE_RGBA8UI 8
#define TEXTURE_TYPE_MAX 9


const char* vertexShaderFile = "./shader/drawtextureformat.vs";
const char* fragmentShaderFile = "./shader/drawtextureformat.fs";
const char* uintFragmentShaderFile = "./shader/drawtextureformatuint.fs";

struct ViewPoint {
	float x, y, z, w;
};

ViewPoint viewport[VIEW_MAX];

GLuint programUint;
GLuint mvpUintLocation;
GLuint diffuseUintLocation;

GLuint programNormalized;
GLuint mvpNormalizedLocation;
GLuint diffuseNormalizedLocation;

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

GLuint vertexPosBuffer;
GLuint vertexTexBuffer;
GLuint vertexArray;

const char* image = "./res/Di-3d.png";
int components, width, height;
GLuint textures[TEXTURE_TYPE_MAX];

struct TextureType {
	GLenum internalFormat;
	GLenum format;
	GLenum type;
};

TextureType textureFormats[TEXTURE_TYPE_MAX];

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

void initViewPort() {
	for (int i = 0; i < VIEW_MAX; i++) {
		float row = floor(i / 3.0);
		float col = i % 3;
		ViewPoint vp;
		vp.x = WINDOW_WIDTH * col / 3.0;
		vp.y = WINDOW_HEIGHT * row / 3.0;
		vp.z = WINDOW_WIDTH / 3.0;
		vp.w = WINDOW_HEIGHT / 3.0;
		viewport[i] = vp;
	}
}

void initTextureFormats() {
	TextureType rgb;
	rgb.internalFormat = GL_RGB;
	rgb.format = GL_RGB;
	rgb.type = GL_UNSIGNED_BYTE;
	textureFormats[TEXTURE_TYPE_RGB] = rgb;

	TextureType rgb8;
	rgb8.internalFormat = GL_RGB8;
	rgb8.format = GL_RGB;
	rgb8.type = GL_UNSIGNED_BYTE;
	textureFormats[TEXTURE_TYPE_RGB8] = rgb8;

	TextureType rgb16f;
	rgb16f.internalFormat = GL_RGB16F;
	rgb16f.format = GL_RGB;
	rgb16f.type = GL_HALF_FLOAT;
	textureFormats[TEXTURE_TYPE_RGB16F] = rgb16f;

	TextureType rgba32f;
	rgba32f.internalFormat = GL_RGBA32F;
	rgba32f.format = GL_RGBA;
	rgba32f.type = GL_FLOAT;
	textureFormats[TEXTURE_TYPE_RGBA32F] = rgba32f;

	TextureType r16f;
	r16f.internalFormat = GL_R16F;
	r16f.format = GL_RED;
	r16f.type = GL_HALF_FLOAT;
	textureFormats[TEXTURE_TYPE_R16F] = r16f;

	TextureType rg16f;
	rg16f.internalFormat = GL_RG16F;
	rg16f.format = GL_RG;
	rg16f.type = GL_HALF_FLOAT;
	textureFormats[TEXTURE_TYPE_RG16F] = rg16f;

	TextureType rgba;
	rgba.internalFormat = GL_RGBA;
	rgba.format = GL_RGBA;
	rgba.type = GL_UNSIGNED_BYTE;
	textureFormats[TEXTURE_TYPE_RGBA] = rgba;

	TextureType rgb8ui;
	rgb8ui.internalFormat = GL_RGB8UI;
	rgb8ui.format = GL_RGB_INTEGER;
	rgb8ui.type = GL_UNSIGNED_BYTE;
	textureFormats[TEXTURE_TYPE_RGB8UI] = rgb8ui;

	TextureType rgba8ui;
	rgba8ui.internalFormat = GL_RGBA8UI;
	rgba8ui.format = GL_RGBA_INTEGER;
	rgba8ui.type = GL_UNSIGNED_BYTE;
	textureFormats[TEXTURE_TYPE_RGBA8UI] = rgba8ui;
}

void init() {
	glClearColor(0.0f, 0.0f, 0.0, 1.0f);
	initViewPort();
	initTextureFormats();
	programUint = compileshaders(vertexShaderFile, uintFragmentShaderFile);
	mvpUintLocation = glGetUniformLocation(programUint, "MVP");
	diffuseUintLocation = glGetUniformLocation(programUint, "diffuse");
	programNormalized = compileshaders(vertexShaderFile, fragmentShaderFile);
	mvpNormalizedLocation = glGetUniformLocation(programNormalized, "MVP");
	diffuseNormalizedLocation = glGetUniformLocation(programNormalized, "diffuse");

	glGenVertexArrays(1, &vertexArray);
	glGenBuffers(1, &vertexPosBuffer);
	glGenBuffers(1, &vertexTexBuffer);

	glBindVertexArray(vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, vertexPosBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexTexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	unsigned char* data = stbi_load(image, &width, &height, &components, 0);
	GLenum imageType = components == 3 ? GL_RGB : GL_RGBA;
	glActiveTexture(GL_TEXTURE0);
	for (int i = 0; i < TEXTURE_TYPE_MAX;i++) {
		glGenTextures(1, &textures[i]);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, imageType, width, height, 0, imageType, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	}

}

void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(vertexArray);
	GLfloat matrix[] = {
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	};

	glUseProgram(programNormalized);
	glUniformMatrix4fv(mvpNormalizedLocation, 1, GL_FALSE, matrix);
	glUniform1i(diffuseNormalizedLocation, 0);
	for (int i = 0; i < TEXTURE_TYPE_RGB8UI; i++) {
		glViewport(viewport[i].x, viewport[i].y, viewport[i].z, viewport[i].w);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glUseProgram(0);

	glUseProgram(programUint);
	glUniformMatrix4fv(mvpUintLocation, 1, GL_FALSE, matrix);
	glUniform1i(diffuseUintLocation, 0);
	for (int i = TEXTURE_TYPE_RGB8UI; i < TEXTURE_TYPE_MAX; i++) {
		glViewport(viewport[i].x, viewport[i].y, viewport[i].z, viewport[i].w);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glUseProgram(0);
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
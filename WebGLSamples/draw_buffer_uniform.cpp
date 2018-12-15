#include <iostream>
#include <fstream>
#include <string>
#include <GL/glew.h>
#include <GL/glut.h>
#pragma comment(lib,"glew32.lib")

const char* vertexShaderFile = "./shader/drawbufferuniform.vs";
const char* fragmentShaderFile = "./shader/drawbufferuniform.fs";
GLuint shaderProgram;
GLuint elementBuffer;
GLuint vertexBuffer;
GLuint uniformPerDrawBuffer;
GLuint uniformPerPassBuffer;
GLuint uniformPerSceneBuffer;
GLuint vertexArray;
GLfloat uTime = 0.0;

GLuint elementData[] = {
	0, 1, 2,
	2, 3, 0
};

//vec3 position, vec3 normal, vec4 color
GLfloat vertices[] = {
	-1.0, -1.0, -0.5,    0.0, 0.0, 1.0,     1.0, 0.0, 0.0, 1.0,
	1.0, -1.0, -0.5,    0.0, 0.0, 1.0,     0.0, 1.0, 0.0, 1.0,
	1.0, 1.0, -0.5,     0.0, 0.0, 1.0,     0.0, 0.0, 1.0, 1.0,
	-1.0, 1.0, -0.5,     0.0, 0.0, 1.0,     1.0, 1.0, 1.0, 1.0
};

//mat4 P, mat4 MV, mat3 Mnormal
GLfloat transforms[] = {
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0,

	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.0, 0.0, 0.0, 1.0,

	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
};

//vec3 ambient, diffuse, specular, float shininess
GLfloat material[] = {
	0.1, 0.0, 0.0,  0.0,
	0.5, 0.0, 0.0,  0.0,
	1.0, 1.0, 1.0,  4.0,
};

GLfloat lightPos[] = {
	0.0f, 0.0f, 0.0f, 0.0f
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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0);

	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	GLuint uniformPerDrawLocation = glGetUniformBlockIndex(shaderProgram, "PerDraw");
	GLuint uniformPerPassLocation = glGetUniformBlockIndex(shaderProgram, "PerPass");
	GLuint uniformPerSceneLocation = glGetUniformBlockIndex(shaderProgram, "PerScene");
	if (uniformPerDrawLocation == 0xffffffff) {
		std::cout << "no uniform PerDraw" << std::endl;
		return;
	}
	if (uniformPerPassLocation == 0xffffffff) {
		std::cout << "no uniform PerPass" << std::endl;
		return;
	}
	if (uniformPerSceneLocation == 0xffffffff) {
		std::cout << "no uniform PerScene" << std::endl;
		return;
	}
	glUniformBlockBinding(shaderProgram, uniformPerDrawLocation, 0);
	glUniformBlockBinding(shaderProgram, uniformPerPassLocation, 1);
	glUniformBlockBinding(shaderProgram, uniformPerSceneLocation, 2);

	glGenBuffers(1, &elementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementData), elementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &uniformPerDrawBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformPerDrawBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(transforms), transforms, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(transforms), transforms);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenBuffers(1, &uniformPerPassBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformPerPassBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(lightPos), lightPos, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lightPos), lightPos);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenBuffers(1, &uniformPerSceneBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformPerSceneBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(material), material, GL_STATIC_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(material), material);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(GL_FLOAT), (GLvoid*)12);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(GL_FLOAT), (GLvoid*)24);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformPerDrawBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, uniformPerPassBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, uniformPerSceneBuffer);
	glBindVertexArray(0);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << err << std::endl;
	}
}

void render() {
	uTime += 0.01;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(vertexArray);
	transforms[16] = 0.1 * cos(uTime) + 0.4;
	glBindBuffer(GL_UNIFORM_BUFFER, uniformPerDrawBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(transforms), transforms);

	lightPos[0] = cos(3.0 * uTime);
	lightPos[1] = sin(6.0 * uTime);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformPerPassBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lightPos), lightPos);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, elementData);
	glBindVertexArray(0);
	glutSwapBuffers();
	glutPostRedisplay();
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutCreateWindow("demo");
	glutDisplayFunc(render);
	glutIdleFunc(render);
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
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <SOIL.h>
#include <chrono>

using namespace std;

float vertices[] = {
	-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	-0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f
};

GLuint elements[] = {
	0, 1, 2,
	2, 3, 0
};

void LoadTextFile(char *file, const char **data)
{
	ifstream f(file);
	string line, str;
	while (getline(f, line))
		str += line + "\n";
	*data = _strdup(str.c_str());
}

int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow *window = glfwCreateWindow(800, 600, "flanarity", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	glewInit();

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	const char *vshdata;
	LoadTextFile("test.vsh", &vshdata);
	GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, 1, &vshdata, nullptr);
	glCompileShader(vshader);

	const char *pshdata;
	LoadTextFile("test.psh", &pshdata);
	GLuint pshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(pshader, 1, &pshdata, nullptr);
	glCompileShader(pshader);

	GLuint shprogram = glCreateProgram();
	glAttachShader(shprogram, vshader);
	glAttachShader(shprogram, pshader);
	glBindFragDataLocation(shprogram, 0, "outColor");
	glLinkProgram(shprogram);
	glUseProgram(shprogram);

	GLint posattrib = glGetAttribLocation(shprogram, "position");
	glEnableVertexAttribArray(posattrib);
	glVertexAttribPointer(posattrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
	GLint colattrib = glGetAttribLocation(shprogram, "color");
	glEnableVertexAttribArray(colattrib);
	glVertexAttribPointer(colattrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *)(2 * sizeof(float)));
	GLint texattrib = glGetAttribLocation(shprogram, "texcoord");
	glEnableVertexAttribArray(texattrib);
	glVertexAttribPointer(texattrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *)(5 * sizeof(float)));

	GLuint textures[2];
	glGenTextures(2, textures);

	int width, height;
	unsigned char *image;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	image = SOIL_load_image("sample.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glUniform1i(glGetUniformLocation(shprogram, "texKitten"), 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	image = SOIL_load_image("sample2.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glUniform1i(glGetUniformLocation(shprogram, "texPuppy"), 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLint uniTime = glGetUniformLocation(shprogram, "time");
	auto t_start = chrono::high_resolution_clock::now();

	while (!glfwWindowShouldClose(window)) {
		auto t_now = chrono::high_resolution_clock::now();
		float time = chrono::duration_cast<chrono::duration<float>>(t_now - t_start).count();
		glUniform1f(uniTime, time);
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}
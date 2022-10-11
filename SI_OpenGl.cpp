#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

#include <vector>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <string>
#include "source/stl.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "source/shader.h"

static void error_callback(int /*error*/, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void APIENTRY opengl_error_callback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	std::cout << message << std::endl;
}

int main(void)
{
#pragma region Create and open a window
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
#pragma endregion

	// Récupère les fonctions pointeurs d'OpenGL du driver
	if (!gladLoadGL()) {
		std::cerr << "Something went wrong!" << std::endl;
		exit(-1);
	}

	// Callbacks
	glDebugMessageCallback(opengl_error_callback, nullptr);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	// Charge et lie les shaders au contexte open gl
	// Shader
	const auto vertex = MakeShader(GL_VERTEX_SHADER, "resources/shaders/shader.vert");
	const auto fragment = MakeShader(GL_FRAGMENT_SHADER, "resources/shaders/shader.frag");

	const auto program = AttachAndLink({ vertex, fragment });

	glUseProgram(program);


	// Définie les matrices de données stockants les vertices du modèles
	// Buffers
	GLuint vbo, vao;
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);

	const auto tris = ReadStl("./resources/models/logo.stl");
	std::cout << tris.size() << std::endl;
	const auto nTriangles = tris.size();


	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, nTriangles * sizeof(Triangle), tris.data(), GL_STATIC_DRAW);


	// Définie une position au vertex shader
	// Bindings
	const auto index = glGetAttribLocation(program, "position");

	glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, sizeof(Triangle) / 3, nullptr);
	glEnableVertexAttribArray(index);

	// glPointSize(20.f);
	//
	glEnable(GL_DEPTH_TEST);


	// Boucle de rendu
	while (!glfwWindowShouldClose(window))
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, nTriangles * 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

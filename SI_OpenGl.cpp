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
#include "source/LightSource.h"
#include "source/Material.h"
#include "source/Triangle.h"

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

std::ostream& operator<<(std::ostream& os, const glm::vec3& v)
{
	os << '(' << v.x << ", " << v.y << ", " << v.z << ')' << '\n';
	return os;
}

void CreateTriangleWithNormals(const std::vector<Triangle>& triangles, std::vector<TriangleWithNormal>& outTrianglesWithNormals)
{
	outTrianglesWithNormals.reserve(triangles.size());

	for (size_t i = 0; i < triangles.size(); i++)
	{
		auto& t = triangles[i];

		glm::vec3 a = t.p0 - t.p1;
		glm::vec3 b = t.p0 - t.p2;
		glm::vec3 n = glm::normalize(glm::cross(a, b));

		//std::cout << "A" << a;
		//std::cout << "B" << b;
		//std::cout << "N" << n;
		//std::cout << std::endl;

		TriangleWithNormal trisWithNormal({ t.p0, n, t.p1, n, t.p2, n });
		outTrianglesWithNormals.push_back(trisWithNormal);
	}
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

	window = glfwCreateWindow(640, 480, "TP Open GL", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
#pragma endregion

	// R�cup�re les fonctions pointeurs d'OpenGL du driver
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


	// D�finie les matrices de donn�es stockants les vertices du mod�les
	// Buffers
	GLuint vbo, vao;
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);

	const auto trisWithoutNormals = ReadStl("resources/models/baby_yoda.stl");
	std::cout << trisWithoutNormals.size() << std::endl;

	std::vector<TriangleWithNormal> tris;
	CreateTriangleWithNormals(trisWithoutNormals, tris);
	const auto nTriangles = tris.size();


	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, nTriangles * sizeof(TriangleWithNormal), tris.data(), GL_STATIC_DRAW);


#pragma region Vertex Shader Loc
	// D�finie une position au vertex shader
	// Bindings
	const auto locPosition = glGetAttribLocation(program, "position");

	glVertexAttribPointer(locPosition, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), nullptr);
	glEnableVertexAttribArray(locPosition);
	assert(locPosition != -1);

	const auto locNormal = glGetAttribLocation(program, "normal");
	glVertexAttribPointer(locNormal, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (const void*) sizeof(glm::vec3));
	glEnableVertexAttribArray(locNormal);
	assert(locNormal != -1);

	const auto locTranslate = glGetUniformLocation(program, "translate");
	assert(locTranslate != -1);

	const auto locScale = glGetUniformLocation(program, "scale");
	assert(locScale != -1);

	const auto locRotate = glGetUniformLocation(program, "rotate");
	assert(locRotate != -1);
#pragma endregion


#pragma region Fragment Shader Loc
	//const auto locCustomColor = glGetUniformLocation(program, "customColor");
	//assert(locCustomColor != -1);
#pragma endregion


	// glPointSize(20.f);
	//
	glEnable(GL_DEPTH_TEST);

	float x = 0.2f;
	float y = -0.4f;
	const glm::vec2 SPEED(0.01f, 0.02f);
	const glm::vec2 LIMIT(1.7f, 1.7f);

	glm::vec2 direction(1, 1);

	glm::mat4 rotation(1.0f);
	rotation = glm::rotate(rotation, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	rotation = rotation + glm::rotate(rotation, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	// Boucle de rendu
	while (!glfwWindowShouldClose(window))
	{

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Vertex Shader
		glUniform3f(locTranslate, x, y, 0.0f);
		glUniform1f(locScale, 0.01f);
		glUniformMatrix4fv(locRotate, 1, GL_FALSE, glm::value_ptr(rotation));

		// Fragment Shader
		//glUniform4f(locCustomColor, 1.0f + cos(glfwGetTime()), sin(glfwGetTime()), cos(glfwGetTime()), 1.0f);
		glDrawArrays(GL_TRIANGLES, 0, nTriangles * 3);

		// Déplacement du modèle
		x += (float)direction.x * SPEED.x;
		if (x < -LIMIT.x || LIMIT.x < x)
		{
			direction.x *= -1;
		}

		y += (float)direction.y * SPEED.y;

		if (y < -LIMIT.y || LIMIT.y < y)
		{
			direction.y *= -1;
		}


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

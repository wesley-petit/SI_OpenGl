#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <string>

#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL/SOIL.h>

#include "source/stl.h"
#include "source/shader.h"
#include "source/LightSource.h"
#include "source/Material.h"
#include "source/Triangle.h"
#include "source/MeshModifier.h"

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

#pragma region Read and bind shader program
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
#pragma endregion

#pragma region Setup vertex buffers
	// D�finie les matrices de donn�es stockants les vertices du mod�les
	// Buffers
	GLuint vbo, vao;
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);

	// Modèle brute
	auto babyYodaRaw = ReadStl("resources/models/baby_yoda.stl");
	std::cout << babyYodaRaw.size() << std::endl;
	const auto nTrianglesYoda = babyYodaRaw.size();

	auto djinnMarsRaw = ReadStl("resources/models/djinn_mars.stl");
	std::cout << djinnMarsRaw.size() << std::endl;
	const auto nTrianglesDjinn = djinnMarsRaw.size();

	// Fusionne les modèles en un buffer
	std::vector<TriangleWithNormal> tris;
	tris.reserve(nTrianglesYoda + nTrianglesDjinn);
	CenterAllVertex(babyYodaRaw);
	CreateTriangleWithNormals(babyYodaRaw, tris);
	CenterAllVertex(djinnMarsRaw);
	CreateTriangleWithNormals(djinnMarsRaw, tris);

	const auto nTriangles = tris.size();

	std::cout << "Yoda Size : " << nTrianglesYoda << std::endl;
	std::cout << "Djinn Size : " << nTrianglesDjinn << std::endl;
	std::cout << "Total Size : " << nTriangles * sizeof(TriangleWithNormal) << std::endl;

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, nTriangles * sizeof(TriangleWithNormal), tris.data(), GL_STATIC_DRAW);
#pragma endregion

#pragma region Setup Textures
	int width(1280);
	int height(720);
	auto texture = SOIL_load_image("resources/textures/david_goodenough.jpg", &width, &height, 0, SOIL_LOAD_RGB);

	// Create an OpenGL texture
	GLuint texC;
	glCreateTextures(GL_TEXTURE_2D, 1, &texC);
	glTextureStorage2D(texC, 5, GL_RGB8, width, height);

	// Send the data
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTextureSubImage2D(texC, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, texture);
	glGenerateTextureMipmap(texC);
#pragma endregion

#pragma region Vertex Shader Loc
	// D�finie une position au vertex shader
	// Bindings
	const auto locPosition(glGetAttribLocation(program, "position"));
	assert(locPosition != -1);

	glVertexAttribPointer(locPosition, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), nullptr);
	glEnableVertexAttribArray(locPosition);


	const auto locNormal(glGetAttribLocation(program, "normal"));
	assert(locNormal != -1);
	
	glVertexAttribPointer(locNormal, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (const void*) sizeof(glm::vec3));
	glEnableVertexAttribArray(locNormal);


	const auto locTranslate(glGetUniformLocation(program, "translate"));
	assert(locTranslate != -1);

	const auto locTransform(glGetUniformLocation(program, "transform"));
	assert(locTransform != -1);
#pragma endregion

#pragma region Fragment Shader Loc
	const auto locLightPosition(glGetUniformLocation(program, "lightPosition"));
	assert(locLightPosition != -1);

	const auto locLightEmitted(glGetUniformLocation(program, "lightEmitted"));
	assert(locLightEmitted != -1);

	const auto locAlbedo(glGetUniformLocation(program, "albedo"));
	assert(locAlbedo != -1);

	const auto locTexture(glGetUniformLocation(program, "tex"));
	assert(locTexture != -1);
#pragma endregion

	// glPointSize(20.f);
	//
	glEnable(GL_DEPTH_TEST);

	const glm::vec2 SPEED(0.01f, 0.02f);
	const glm::vec2 LIMIT(1.7f, 1.7f);
	
	float x = 0.2f;
	float y = -0.4f;

	glm::vec2 direction(1, 1);

#pragma region Uniform variables
	LightSource lightSource{ glm::vec3(50 * cos(glfwGetTime()), -150, 50), glm::vec3(40000, 40000, 40000) };

	// Texture du modèle
	glBindTextureUnit(0, texC);
	glUniform1i(locTexture, 0);

	// Intialisation des composantes de la scene (lumière...)
	// Variables pour chaque buffer
	glm::mat4 yodaTransform(glm::mat4(1.0f));
	yodaTransform = glm::rotate(yodaTransform, glm::radians(180.0f), glm::vec3(0, 1, 0));
	yodaTransform = glm::rotate(yodaTransform, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	yodaTransform = glm::scale(yodaTransform, glm::vec3(0.01f, 0.01f, 0.01f));
	
	const Material yodaMaterial{ glm::vec3(0.1f, 0.8f, 0.15f) };


	glm::mat4 djinnTransform(glm::mat4(1.0f));
	djinnTransform = glm::rotate(djinnTransform, glm::radians(90.0f), glm::vec3(1, 0, 0));
	djinnTransform = glm::rotate(djinnTransform, glm::radians(-135.0f), glm::vec3(0, 1, 0));
	djinnTransform = glm::scale(djinnTransform, glm::vec3(0.01f, 0.01f, 0.01f));

	const Material djinnMaterial{ glm::vec3(0.75f, 0.2f, 0.1f) };
#pragma endregion

	// Boucle de rendu
	while (!glfwWindowShouldClose(window))
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Lumière
		lightSource.position = glm::vec3(100 * sin(glfwGetTime()), 150 * cos(glfwGetTime()), 50);
		glUniform3fv(locLightPosition, 1, glm::value_ptr(lightSource.position));
		glUniform3fv(locLightEmitted, 1, glm::value_ptr(lightSource.radianceEmitted));

		/* ------------------------------------ Yoda ------------------------------------ */ 
		// Vertex Shader
		glUniform3f(locTranslate, x, y, 0.0f);
		glUniformMatrix4fv(locTransform, 1, GL_FALSE, glm::value_ptr(yodaTransform));

		// Fragment Shader
		glUniform3fv(locAlbedo, 1, glm::value_ptr(yodaMaterial.albedo));
		glDrawArrays(GL_TRIANGLES, 0, nTrianglesYoda * 3);


		/* ------------------------------------ Djinn ------------------------------------ */
		// Vertex Shader
		glUniform3f(locTranslate, -0.5, 0, 0);
		glUniformMatrix4fv(locTransform, 1, GL_FALSE, glm::value_ptr(djinnTransform));

		// Fragment Shader
		glUniform3fv(locAlbedo, 1, glm::value_ptr(djinnMaterial.albedo));
		glDrawArrays(GL_TRIANGLES, nTrianglesYoda * 3, nTrianglesDjinn * 3);

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

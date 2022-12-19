#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Shader.h"
#include "Texture.h"

int width = 500;
int height = 1000;
float color[3] = { 1.0f, 1.0f, 1.0f };
float background[3] = { 0.7f, 0.3f, 0.1f };

float nearPlane = 0.1f;
float farPlane = 100.0f;
float angle = 0.0f; 
float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::mat4 i4 = glm::mat4(1.0f); //The identity matrix
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //The Normal of the camera

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);

	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;

	while (getline(stream, line))
	{
		if (line.find("#shader") !=	std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
				type = ShaderType::VERTEX;

			else if (line.find("fragment") != std::string::npos)
				type = ShaderType::FRAGMENT;
		}

		else
			ss[(int)type] << line << "\n";
	}

	return { ss[0].str(), ss[1].str() };
}


int main()
{
	//---------------INITIALIZING OPENGL 3.3---------------//
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(height, width, "Stinky", NULL, NULL);	

	if (window == NULL)
	{
		std::cout << "Your screen gone lol" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSwapInterval(5);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "You are not GLAD :(" << std::endl;
		return -1;
	}

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	glm::vec3 moreCubes[] = {
		glm::vec3( 0.0f,  0.0f, -5.0f),
		glm::vec3(-2.5f,  0.0f, -7.0f),
		glm::vec3( 1.5f, -1.0f, -9.0f),
		glm::vec3( 2.0f,  2.5f, -2.0f),
		glm::vec3(-3.5f, -3.5f,  0.0f),
	};

	glm::vec3 lightCube = glm::vec3(0.0f, 0.0f, -5.0f);

	//---------------Buffers, Arrays, and Linking Vertex Attributes---------------//
	VertexArray VAO;
	VertexArray lampVAO;
	VertexBufferLayout layout;
	VertexBuffer VBO(vertices, sizeof(vertices));
	ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
	ShaderProgramSource lampSource = ParseShader("res/shaders/Lamp.shader");
	Shader shaderProgram(source.VertexSource, source.FragmentSource);
	Shader lampShaderProgram(lampSource.VertexSource, lampSource.FragmentSource);
	
	Texture crate("res/textures/crate.jpg");
	crate.Bind(0);
	Texture frame("res/textures/frame.jpg");
	frame.Bind(1);

	shaderProgram.Bind();

	layout.Push<float>(3);
	layout.Push<float>(3);
	layout.Push<float>(2);
	VAO.AddBuffer(layout);
	lampVAO.AddBuffer(layout);

	crate.Unbind();
	frame.Unbind();
	VBO.Unbind();
	VAO.Unbind();
	lampVAO.Unbind();
	shaderProgram.Unbind();

	glEnable(GL_DEPTH_TEST); //z buffer
	
//---------------INITIALIZING IMGUI------------------//
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	//-------------------------RENDER LOOP-------------------------//
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	bool drawTriangle = true;
	while (!glfwWindowShouldClose(window))
	{
		//inputs and per-frame stuff
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);

		//rendering
		glClearColor(background[0], background[1], background[2], 1.0f); //1.0f, 0.7f, 0.7f for pink 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); //color and z buffer

		//imgui stuff
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//view and projection
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1800.0f / 900.0f, nearPlane, farPlane);

		//draw
		shaderProgram.Bind();
		crate.Bind(0);
		frame.Bind(1);

		shaderProgram.SetUniform3f("lightPos", lightCube.x, lightCube.y, lightCube.z);
		shaderProgram.SetUniform3f("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);
		shaderProgram.SetUniform3f("lightCol", color[0], color[1], color[2]);

		shaderProgram.SetUniform3f("choc.ambientCol", 0.05f, 0.05f, 0.05f);
		shaderProgram.SetUniform3f("choc.diffCol", 0.382, 0.147, 0.0);
		shaderProgram.SetUniform3f("choc.specCol", 0.75, 0.75, 0.75);
		shaderProgram.SetUniform1f("choc.alpha", 42.0);

		shaderProgram.SetUniform1i("material.diffuse", 1); // Index of texture (0: Crate, 1: Frame)
		shaderProgram.SetUniform1i("material.specular", 1); // Index of texture (0: Crate, 1: Frame)
		shaderProgram.SetUniform1f("material.shine", 42.0); // Higher more focused, Lower more dispersed. The number is represented by "alpha" in the equation.
		
		for (unsigned int i = 0; i < (sizeof(moreCubes)) / sizeof(moreCubes[0]); i++)
		{
			glm::mat4 model = glm::translate(i4, moreCubes[i]);
			model = glm::rotate(model, glm::radians(angle * (i + 1)), glm::vec3(0.5f, 1.0f, 0.75f));
			model = glm::scale(model, glm::vec3(2.0f));
			shaderProgram.SetUniformMat4f("model", model);
			shaderProgram.SetUniformMat4f("view", view);
			shaderProgram.SetUniformMat4f("projection", projection);

			glm::mat4 normalMat = transpose(inverse(model));
			shaderProgram.SetUniformMat4f("normalMat", normalMat);
			VAO.Bind();

			if (drawTriangle)
				glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//draw light source
		lampShaderProgram.Bind();
		lampShaderProgram.SetUniform4f("luminosity", color[0], color[1], color[2], 1.0f);

		for (unsigned int i = 0; i < 1; i++)
		{
			lightCube.x = sin(glfwGetTime() * 0.5) * 6.0f;
			lightCube.z = -5.0f + cos(glfwGetTime() * 0.5) * 6.0f;
			lightCube.y = 2.0f;

			glm::mat4 lampModel = glm::translate(i4, lightCube);
			lampModel = glm::rotate(lampModel, glm::radians(currentFrame * 100.0f), glm::vec3(0.5f, 1.0f, 0.75f));
			lampModel = glm::scale(lampModel, glm::vec3(0.3f, 0.3f, 0.3f));
			lampShaderProgram.SetUniformMat4f("model", lampModel);
			lampShaderProgram.SetUniformMat4f("view", view);
			lampShaderProgram.SetUniformMat4f("projection", projection);
			lampVAO.Bind();
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//glEnable(GL_FRAMEBUFFER_SRGB);

		ImGui::Begin("You're awake");
		ImGui::Text("What will you do now?");
		ImGui::Checkbox("Draw Boxes", &drawTriangle);
		ImGui::ColorEdit4("Color", color);
		ImGui::ColorEdit4("Background", background);
		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//glfw: swap buffer and take i/o events.
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	VBO.~VertexBuffer();
	VAO.~VertexArray();
	lampVAO.~VertexArray();
	shaderProgram.~Shader();

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		angle += 2.0f;
		if (angle > 360.0f)
			angle = 0.0f;
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		angle -= 2.0f;
		if (angle < 0.0f)
			angle = 360.f;
	}

	const float cameraSpeed = 5.0f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraUp;

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraUp;
}

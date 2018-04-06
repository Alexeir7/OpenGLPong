#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// window options
int WIDTH = 800;
int HEIGHT = 600;

// rackets
int racket_width = 20;
int racket_height = 175;
int racket_speed = 1;

// move
int move1 = 0;
int move2 = 0;

// left racket
float racket_left_x = 0.0f;
float racket_left_y = (HEIGHT - racket_height)/2;

// right racket
float racket_right_x = WIDTH - racket_width;
float racket_right_y = (HEIGHT - racket_height) / 2;

struct ShaderProgramSource {
	std::string VertexSource;
	std::string FragmentSource;
};


static ShaderProgramSource ParseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);

	enum class ShaderType
	{
		NONE = -1,
		VERTEX = 0,
		FRAGMENT = 1
	};

	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;

	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
				type = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)
				type = ShaderType::FRAGMENT;
		}
		else
		{
			ss[(int)type] << line << '\n';
		}
	}
	return { ss[0].str(), ss[1].str() };
}


static unsigned int CompileShader(unsigned int type, const std::string& source) {
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Failed to compile" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "shader!" << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}
	return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(WIDTH, HEIGHT, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();

		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// initialize glew
	if (glewInit() != GLEW_OK)
		std::cout << "Failed to initialize GLEW" << std::endl;

	glEnable(GL_DEPTH_TEST);

	ShaderProgramSource source = ParseShader("res/shaders/Shader.shader");
	std::cout << "VERTEX" << std::endl;
	std::cout << source.VertexSource << std::endl;
	std::cout << "FRAGMENT" << std::endl;
	std::cout << source.FragmentSource << std::endl;

	unsigned int shaderProgram = CreateShader(source.VertexSource, source.FragmentSource);
	glUseProgram(shaderProgram);


	// left racket position
	float left_racket[8] =
	{
		racket_left_x, racket_left_y, // botton left
		racket_left_x + racket_width, racket_left_y, // botton right
		racket_left_x + racket_width, racket_left_y + racket_height, // top right
		racket_left_x, racket_left_y + racket_height, // top left
	};

	// right racket position
	float right_racket[8] =
	{
		racket_right_x, racket_right_y,
		racket_right_x + racket_width, racket_right_y,
		racket_right_x + racket_width, racket_right_y + racket_height,
		racket_right_x, racket_right_y + racket_height,
	};

	unsigned int indices[6] =
	{
		0, 1, 2,
		2, 3, 0
	};

	unsigned int VBO[2], VAO[2], EBO[2];
	glGenVertexArrays(2, VAO);
	glGenBuffers(2, VBO);
	glGenBuffers(2, EBO);

	// setting up first paddle
	// -----------------------
	glBindVertexArray(VAO[0]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(left_racket), left_racket, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	// second paddle setup
	// ---------------------
	glBindVertexArray(VAO[1]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(right_racket), right_racket, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		// get keyboard input
		processInput(window);

		/* Render here */
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgram);

		// set projection matrix for positioning everything easily
		glm::mat4 projectionMatrix = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT, 1.0f, -1.0f);
		unsigned int vertexProjection = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(vertexProjection, 1, GL_FALSE, &projectionMatrix[0][0]);

		// set translation matrix
		glm::mat4 translateMatrix = glm::mat4(1.0f);
		translateMatrix = glm::translate(translateMatrix, glm::vec3(0.0f, (float)move1, 0.0f));
		unsigned int vertexTranslate = glGetUniformLocation(shaderProgram, "model");
		glUniformMatrix4fv(vertexTranslate, 1, GL_FALSE, glm::value_ptr(translateMatrix));

		// draw first racket
		glBindVertexArray(VAO[0]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// translate second paddle
		translateMatrix = glm::mat4(1.0f);
		translateMatrix = glm::translate(translateMatrix, glm::vec3(0.0f, (float)move2, 0.0f));
		glUniformMatrix4fv(vertexTranslate, 1, GL_FALSE, &translateMatrix[0][0]);

		// draw second racket
		glBindVertexArray(VAO[1]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glDeleteVertexArrays(2, VAO);
	glDeleteBuffers(2, VBO);
	glDeleteBuffers(2, EBO);

	glfwTerminate();
	return 0;
}

// process input, the racket doesn't move :(
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		move1 += racket_speed;
		if (move1 > 225)
			move1 = 225;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		move1 -= racket_speed;
		if (move1 < -225)
			move1 = -225;
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		move2 += racket_speed;
		if (move2 > 225)
			move2 = 225;
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		move2 -= racket_speed;
		if (move2 < -225)
			move2 = -225;
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

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

	// build and compile our shader
	// ----------------------------
	//vertex shader
	const char *vertexShaderSource =
		"#version 330 core\n"
		"\n"
		"layout(location = 0) in vec4 position;\n"
		"\n"
		"uniform mat4 projection;\n"
		"uniform mat4 translate;\n"
		"\n"
		"void main()\n"
		"{\n"
		"gl_Position = projection * translate * vec4(position.x, position.y, 0.0, 1.0);\n"
		"}\n\0";

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// chack for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// fragment shader
	const char *fragmentShaderSource =
		"#version 330 core\n"
		"\n"
		"layout(location = 0) out vec4 color;"
		"\n"
		"void main()\n"
		"{\n"
		"	color= vec4(1.0f, 0.5f, 0.2, 1.0);\n"
		"}\n\0";
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// chack for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// link shaders
	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

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
		unsigned int vertexTranslate = glGetUniformLocation(shaderProgram, "translate");
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
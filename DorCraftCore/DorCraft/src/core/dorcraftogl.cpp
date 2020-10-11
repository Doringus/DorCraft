#include "dorcraftogl.h"

#include "dorcraftutils.h"
#include "dorcraft.h"

#include <GLFW/glfw3.h>
#include <string.h>
#include <cstdlib>

static void processGlfwButtonInput(GLFWwindow *window, gameButtonState_t *newState, int key) {
	newState->pressed = glfwGetKey(window, key);
}

static void processGlfwMouseButtonInput(GLFWwindow *window, gameButtonState_t *newState, int key) {
	newState->pressed = glfwGetMouseButton(window, key);
}

static void moveCamera(camera_t *camera, gameInput_t *input, double dt) {
	if (input->forward.pressed) {
		camera->position += (GLfloat)(0.05f * dt) * camera->front;
	}
	if (input->back.pressed) {
		camera->position -= (GLfloat)(0.05f * dt) * camera->front;
	}
	if (input->right.pressed) {
		camera->position += glm::normalize(glm::cross(camera->front, camera->up)) * (GLfloat)(0.05f * dt);
	}
	if (input->left.pressed) {
		camera->position -= glm::normalize(glm::cross(camera->front, camera->up)) * (GLfloat)(0.05f * dt);
	}
}

void glGameLoop() {
	double timePerFrame = 1.0 / 60.0;
	double timer = glfwGetTime();
	double dt = 0.0;
	double lastTime = glfwGetTime();
	short updates = 0, frames = 0;

	char* fragmentShaderCode = (char*)readFile("src\\shaders\\basic.frag");
	char* vertexShaderCode = (char*)readFile("src\\shaders\\basic.vert");
	GLfloat vertices[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f, 0.5f, 0.0f
	};
	renderBuffer_t buf;
	createChunksRenderBuffer(&buf, vertices, sizeof(vertices));
	/*Create shader and init uniforms*/
	GLuint shader = createShader(vertexShaderCode, fragmentShaderCode);
	GLint mvpLocation = glGetUniformLocation(shader, "mvp");
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)windowInfo.windowWidth / (GLfloat)windowInfo.windowHeight, 0.1f, 2500.0f);
	/* Setup camera*/
	camera_t cam = {};
	cam.position = glm::vec3(0.0f, 0.0f, 3.0f);
	cam.front = glm::vec3(0.0f, 0.0f, -1.0f);
	cam.up = glm::vec3(0.0f, 1.0f, 0.0f);

	while (!glfwWindowShouldClose(windowInfo.window)) {
		/* Poll events */
		glfwPollEvents();
		/* Input */
		gameInput_t input = {};
		processGlfwButtonInput(windowInfo.window, &input.back, GLFW_KEY_S);
		processGlfwButtonInput(windowInfo.window, &input.forward, GLFW_KEY_W);
		processGlfwButtonInput(windowInfo.window, &input.left, GLFW_KEY_A);
		processGlfwButtonInput(windowInfo.window, &input.right, GLFW_KEY_D);
		processGlfwMouseButtonInput(windowInfo.window, &input.leftMouseButton, GLFW_MOUSE_BUTTON_LEFT);
		processGlfwMouseButtonInput(windowInfo.window, &input.rightMouseButton, GLFW_MOUSE_BUTTON_RIGHT);

		double currentTime = glfwGetTime();
		dt += (currentTime - lastTime) / timePerFrame;
		lastTime = currentTime;
		while (dt >= 1.0) {
			moveCamera(&cam, &input, dt);
			view = glm::lookAt(cam.position, cam.position + cam.front, cam.up);
			gameUpdate(&input);
			updates++;
			dt--;
		}
		frames++;
		if (currentTime - timer > 1.0) {
			timer = currentTime;
			//printf("Updates %d, frames %d\n", updates, frames);
			updates = 0;
			frames = 0;
		}

		glClear(GL_COLOR_BUFFER_BIT);
		/* Render */
		glUseProgram(shader);
		glm::mat4 mvpMatrix = projection * view * model;
		glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));

		glBindVertexArray(buf.vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
		/***********/
		glfwSwapBuffers(windowInfo.window);
	}
	glfwTerminate();
}

bool initGlContext(int windowWidth, int windowHeight, char *windowTitle) {
	if (!glfwInit()) {
		ERROR_LOG("Cannot initialize glfw lib");
		return(false);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	windowInfo.window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
	windowInfo.windowHeight = windowHeight;
	windowInfo.windowWidth = windowWidth;
	windowInfo.windowTitle = windowTitle;
	if (!windowInfo.window) {
		ERROR_LOG("Cannot create window");
		return(false);
	}
	glfwMakeContextCurrent(windowInfo.window);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		ERROR_LOG("Cannot initialize glew lib");
		return(false);
	}
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(glDebugOutput, NULL);

}


static void createChunksRenderBuffer(renderBuffer_t *buffer, void *data, uint64_t dataSize) {
	glCreateBuffers(1, &buffer->vbo);
	// Vertices must be float
	glNamedBufferData(buffer->vbo, sizeof(GLfloat) * dataSize, data, GL_DYNAMIC_DRAW);
	glCreateVertexArrays(1, &buffer->vao);
	glVertexArrayAttribBinding(buffer->vao, 0, defaultBindingIndex);
	glVertexArrayAttribFormat(buffer->vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(buffer->vao, 0);
	glVertexArrayVertexBuffer(buffer->vao, defaultBindingIndex, buffer->vbo, 0, sizeof(GLfloat) * 3);
}

static GLuint createShader(char *vertexShaderCode, char *fragmentShaderCode) {
	GLint result;
	GLchar info[512];
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(vertexShader, 512, NULL, info);
		ERROR_LOG("%s", info);
	}
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, info);
		ERROR_LOG("%s", info);
	}
	GLuint shader = glCreateProgram();
	glAttachShader(shader, vertexShader);
	glAttachShader(shader, fragmentShader);
	glLinkProgram(shader);
	glGetProgramiv(shader, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shader, 512, NULL, info);
		ERROR_LOG("%s", info);
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return(shader);
}

static void glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const *message, void const *user_param) {
	char sourceStr[20];
	char typeStr[20];
	char severityStr[15];
	switch (source) {
		case GL_DEBUG_SOURCE_API: {
			strcpy_s(sourceStr, _countof(sourceStr), "API");
			break;
		}
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
			strcpy_s(sourceStr, _countof(sourceStr), "WINDOW_SYSTEM");
			break;
		}
		case GL_DEBUG_SOURCE_SHADER_COMPILER: {
			strcpy_s(sourceStr, _countof(sourceStr), "SHADER_COMPILER");
			break;
		}
		case GL_DEBUG_SOURCE_THIRD_PARTY: {
			strcpy_s(sourceStr, _countof(sourceStr), "THIRD_PARTY");
			break;
		}
		case GL_DEBUG_SOURCE_APPLICATION: {
			strcpy_s(sourceStr, _countof(sourceStr), "APPLICATION");
			break;
		}
		case GL_DEBUG_SOURCE_OTHER: {
			strcpy_s(sourceStr, _countof(sourceStr), "OTHER");
			break;
		}
	}
	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR: {
			strcpy_s(typeStr, _countof(typeStr), "ERROR");
			break;
		}
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
			strcpy_s(typeStr, _countof(typeStr), "DEPRECATED_BEHAVIOR");
			break;
		}
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
			strcpy_s(typeStr, _countof(typeStr), "UNDEFINED BEHAVIOR");
			break;
		}
		case GL_DEBUG_TYPE_PORTABILITY: {
			strcpy_s(typeStr, _countof(typeStr), "PORTABILITY");
			break;
		}
		case GL_DEBUG_TYPE_PERFORMANCE: {
			strcpy_s(typeStr, _countof(typeStr), "PERFORMANCE");
			break;
		}
		case GL_DEBUG_TYPE_MARKER: {
			strcpy_s(typeStr, _countof(typeStr), "MARKER");
			break;
		}
		case GL_DEBUG_TYPE_OTHER: {
			strcpy_s(typeStr, _countof(typeStr), "OTHER");
			break;
		}
	}
	switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION: {
			strcpy_s(severityStr, _countof(severityStr), "NOTIFICATION");
			break;
		}
		case GL_DEBUG_SEVERITY_LOW: {
			strcpy_s(severityStr, _countof(severityStr), "LOW");
			break;
		}
		case GL_DEBUG_SEVERITY_MEDIUM: {
			strcpy_s(severityStr, _countof(severityStr), "MEDIUM");
			break;
		}
		case GL_DEBUG_SEVERITY_HIGH: {
			strcpy_s(severityStr, _countof(severityStr), "HIGH");
			break;
		}
	}
	WARNING_LOG("OPENGL: %s, %s, %s, %u, %s", sourceStr, typeStr, severityStr, id, message);
}

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdint.h>

#include "../../vendor/glm/glm.hpp"
#include "../../vendor/glm/gtc/matrix_transform.hpp"
#include "../../vendor/glm/gtc/type_ptr.hpp"

struct uniformLocation_t {
	GLint location;
};

struct renderBuffer_t {
	GLuint vbo;
	GLuint vao;
	uint64_t verticesCount;
};

struct windowInfo_t {
	char* windowTitle;
	uint16_t windowWidth;
	uint16_t windowHeight;
	GLFWwindow* window;
};

struct camera_t {
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	double pitch;
	double yaw;
};

struct mousePosition_t {
	double currentX;
	double currentY;
	double prevX;
	double prevY;
};

static GLuint defaultBindingIndex = 0;
static windowInfo_t windowInfo;

void glGameLoop();
bool initGlContext(int windowWidth, int screenHeight, char *windowTitle);

static void createChunksRenderBuffer(renderBuffer_t *buffer, void *data, uint64_t dataSize);
static GLuint createShader(char *vertexShaderCode, char *fragmentShaderCode);
static void glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const *message, void const *user_param);
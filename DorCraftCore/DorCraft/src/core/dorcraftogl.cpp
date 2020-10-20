#include "dorcraftogl.h"

#include "dorcraftutils.h"
#include "dorcraft.h"

#include <GLFW/glfw3.h>
#include <string.h>
#include <cstdlib>
#include <stdint.h>

struct opengl_t {
	GLint mvpLocation;
	GLuint basicShader;
	GLuint vaoId;
	GLuint vboId;
};

static opengl_t globalOpenGlInfo;

static void glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param) {
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


void initOpenGl(uint16_t windowWidth, uint16_t windowHeight) {
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		ERROR_LOG("Cannot initialize glew lib");
	}
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(glDebugOutput, NULL);
	glViewport(0, 0, windowWidth, windowHeight);

	char* fragmentShaderCode = (char*)readFile("src\\shaders\\basic.frag");
	char* vertexShaderCode = (char*)readFile("src\\shaders\\basic.vert");
	globalOpenGlInfo.basicShader = createShader(vertexShaderCode, fragmentShaderCode);
	globalOpenGlInfo.mvpLocation = glGetUniformLocation(globalOpenGlInfo.basicShader, "mvp");
	glCreateBuffers(1, &globalOpenGlInfo.vboId);
	glCreateVertexArrays(1, &globalOpenGlInfo.vaoId);
	glVertexArrayAttribBinding(globalOpenGlInfo.vaoId, 0, 1);
	glVertexArrayAttribFormat(globalOpenGlInfo.vaoId, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(globalOpenGlInfo.vaoId, 0);
	glVertexArrayVertexBuffer(globalOpenGlInfo.vaoId, 1, globalOpenGlInfo.vboId, 0, sizeof(GLfloat) * 3);
}

void renderToOutput(renderGroup_t* renderGroup) {
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(globalOpenGlInfo.basicShader);
	glUniformMatrix4fv(globalOpenGlInfo.mvpLocation, 1, GL_FALSE, glm::value_ptr(
												renderGroup->projectionMatrix * 
												renderGroup->viewMatrix * renderGroup->modelMatrix));
	glBindVertexArray(globalOpenGlInfo.vaoId);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void allocateTexture(uint32_t width, uint32_t height, void* data) {

}

void allocateRenderBuffer(void *data, uint64_t size) {
	glNamedBufferData(globalOpenGlInfo.vboId, sizeof(GLfloat) * size, data, GL_DYNAMIC_DRAW);
}

static void createChunksRenderBuffer(renderBuffer_t *buffer, void *data, uint64_t dataSize) {
	glCreateBuffers(1, &buffer->vbo);
	// Vertices must be float
	glNamedBufferData(buffer->vbo, sizeof(GLfloat) * dataSize, data, GL_DYNAMIC_DRAW);
	glCreateVertexArrays(1, &buffer->vao);
	// glVertexArrayAttribBinding(buffer->vao, 0, defaultBindingIndex);
	glVertexArrayAttribFormat(buffer->vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(buffer->vao, 0);
	//glVertexArrayVertexBuffer(buffer->vao, defaultBindingIndex, buffer->vbo, 0, sizeof(GLfloat) * 3);
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
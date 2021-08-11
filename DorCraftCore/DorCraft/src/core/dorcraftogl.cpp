#include "dorcraftogl.h"

#include <GLFW/glfw3.h>
#include <string.h>
#include <cstdlib>
#include <stdint.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../../vendor/stb/stb.h"

#include "chunk.h"

#define CHUNK_BUFFER_OFFSET 737280 / 5

struct opengl_t {
	GLint mvpLocation;
	GLint textureSamplerLocation;
	GLuint basicShader;
	GLuint vaoId;
	GLuint vboId;
	GLuint textureId;
	int64_t subBuffersInfo[75];
	GLuint VBO1, VBO2;
};

static opengl_t globalOpenGlInfo;
static int64_t trianglesCount = 0;

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

static void allocateTexture(uint32_t width, uint32_t height, uint32_t bpp, unsigned char *data) {
	glCreateTextures(GL_TEXTURE_2D, 1, &globalOpenGlInfo.textureId);
	glTextureParameteri(globalOpenGlInfo.textureId, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(globalOpenGlInfo.textureId, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(globalOpenGlInfo.textureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTextureParameteri(globalOpenGlInfo.textureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureStorage2D(globalOpenGlInfo.textureId, 1, GL_RGBA8, width, height);
	glTextureSubImage2D(globalOpenGlInfo.textureId, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateTextureMipmap(globalOpenGlInfo.textureId); 
}

void initOpenGl(uint16_t windowWidth, uint16_t windowHeight) {
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		ERROR_LOG("Cannot initialize glew lib");
	}
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEPTH_TEST);
	glDebugMessageCallback(glDebugOutput, NULL);
	glViewport(0, 0, windowWidth, windowHeight);

	char *fragmentShaderCode = (char*)readFile("src\\shaders\\basic.frag");
	char *vertexShaderCode = (char*)readFile("src\\shaders\\basic.vert");
	globalOpenGlInfo.basicShader = createShader(vertexShaderCode, fragmentShaderCode);
	globalOpenGlInfo.mvpLocation = glGetUniformLocation(globalOpenGlInfo.basicShader, "mvp");
	globalOpenGlInfo.textureSamplerLocation = glGetUniformLocation(globalOpenGlInfo.basicShader, "textureSampler");
	
	glCreateVertexArrays(1, &globalOpenGlInfo.vaoId);

	glVertexArrayAttribBinding(globalOpenGlInfo.vaoId, 0, 1);
	glVertexArrayAttribFormat(globalOpenGlInfo.vaoId, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(globalOpenGlInfo.vaoId, 0);

	glVertexArrayAttribBinding(globalOpenGlInfo.vaoId, 1, 1);
	glVertexArrayAttribFormat(globalOpenGlInfo.vaoId, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat));
	glEnableVertexArrayAttrib(globalOpenGlInfo.vaoId, 1);

	stbi_set_flip_vertically_on_load(1);
	int32_t textureWidth, textureHeight, bpp;
	unsigned char *data = stbi_load("res\\grassTexture2.png", &textureWidth, &textureHeight, &bpp, 4);
 	allocateTexture((uint32_t)textureWidth, (uint32_t)textureHeight, bpp, data);
	stbi_image_free(data);
	
}

void initRenderBuffer(renderBuffer_t *renderBuffer) {
	glCreateBuffers(1, &renderBuffer->vbo);
	glCreateBuffers(1, &renderBuffer->ibo);
	renderBuffer->vao = globalOpenGlInfo.vaoId;
	renderBuffer->verticesCount = 0;
	renderBuffer->indicesCount = 0;
}

void fillRenderBuffer(renderBuffer_t *renderBuffer) {
	glNamedBufferData(renderBuffer->vbo, renderBuffer->verticesCount * 5 * sizeof(GLfloat),
		renderBuffer->vertices, GL_DYNAMIC_DRAW);
	glNamedBufferData(renderBuffer->ibo, renderBuffer->indicesCount * sizeof(GLuint), renderBuffer->indices, GL_DYNAMIC_DRAW);
}

void renderChunks(viewProjectionMatrices_t *vpMatrices, chunk_t *chunks[], uint8_t chunksCount) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(globalOpenGlInfo.basicShader);

	// Set constant shader variables like texture
	glUniform1i(globalOpenGlInfo.textureSamplerLocation, 0);
	glBindTextureUnit(0, globalOpenGlInfo.textureId);;

	// bind vao for chunks
	glBindVertexArray(globalOpenGlInfo.vaoId);

	for (uint8_t i = 0; i < chunksCount; ++i) {
		renderChunk(vpMatrices, chunks[i]);
	}
	glUseProgram(0);
 }	

void renderChunk(viewProjectionMatrices_t *vpMatrices, chunk_t *chunk) {
	glUniformMatrix4fv(globalOpenGlInfo.mvpLocation, 1, GL_FALSE, glm::value_ptr(
		vpMatrices->projectionMatrix *
		vpMatrices->viewMatrix * chunk->modelMatrix));
	glVertexArrayElementBuffer(chunk->renderInfo.vao, chunk->renderInfo.ibo);
	glVertexArrayVertexBuffer(chunk->renderInfo.vao, 1, chunk->renderInfo.vbo, 0, sizeof(GLfloat) * 5);
	glDrawElements(GL_TRIANGLES, chunk->renderInfo.indicesCount, GL_UNSIGNED_INT, 0);
}

void pushVertices(int64_t offset, int64_t size, void *data) {
	if (size == 0) {
		return;
	}
	globalOpenGlInfo.subBuffersInfo[offset / CHUNK_RENDER_BUFFER_SIZE] = size / (sizeof(GLfloat) * 5);
	glNamedBufferSubData(globalOpenGlInfo.vboId, offset, size, data);
}



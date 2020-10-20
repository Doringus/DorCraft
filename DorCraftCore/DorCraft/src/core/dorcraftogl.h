#pragma once

#include <GL/glew.h>
#include <stdint.h>

#include "../../vendor/glm/glm.hpp"
#include "../../vendor/glm/gtc/matrix_transform.hpp"
#include "../../vendor/glm/gtc/type_ptr.hpp"

struct renderGroup_t {
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
};

struct renderBuffer_t {
	GLuint vbo;
	GLuint vao;
	uint64_t verticesCount;
};

void initOpenGl(uint16_t windowWidth, uint16_t windowHeight);
void renderToOutput(renderGroup_t *renderGroup);
void allocateTexture(uint32_t width, uint32_t height, void* data);
void allocateRenderBuffer(void *data, uint64_t size);

static void createChunksRenderBuffer(renderBuffer_t *buffer, void *data, uint64_t dataSize);
static GLuint createShader(char *vertexShaderCode, char *fragmentShaderCode);
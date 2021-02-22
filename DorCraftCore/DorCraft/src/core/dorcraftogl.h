#pragma once

#include <GL/glew.h>
#include <stdint.h>

#include "dorcraftutils.h"

#include "../../vendor/glm/glm.hpp"
#include "../../vendor/glm/gtc/matrix_transform.hpp"
#include "../../vendor/glm/gtc/type_ptr.hpp"

#define CHUNK_RENDER_BUFFER_SIZE KILOBYTES(2880)

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
void pushVertices(int64_t offset, int64_t size, void *data);
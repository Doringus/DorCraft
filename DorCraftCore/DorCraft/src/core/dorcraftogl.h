#ifndef DORCRAFTOGL_H
#define DORCRAFTOGL_H

#include <GL/glew.h>
#include <stdint.h>

#include "dorcraftutils.h"
#include "dorcraft.h"

#include "../../vendor/glm/glm.hpp"
#include "../../vendor/glm/gtc/matrix_transform.hpp"
#include "../../vendor/glm/gtc/type_ptr.hpp"

#define CHUNK_RENDER_BUFFER_SIZE KILOBYTES(10000) 
#define CHUNK_INDICES_BUFFER_SIZE KILOBYTES(4610)
#define BLOCK_FACE_VERTICES_COUNT 6 * 5
#define BLOCK_VERTICES_COUNT BLOCK_FACE_VERTICES * 6
#define MAX_VERTICES_COUNT CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT * BLOCK_VERTICES_COUNT 

struct chunk_t;

struct viewProjectionMatrices_t {
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
};

struct renderBuffer_t {
	bool isDirty; /* flag marks that vertices were updated and need to update opengl buffer */
	GLuint vbo;
	GLuint vao;
	GLuint ibo;
	uint64_t verticesCount;
	uint64_t indicesCount;
	GLfloat *vertices;
	GLuint *indices;
};

void initOpenGl(uint16_t windowWidth, uint16_t windowHeight);
void createRenderBuffer(renderBuffer_t *renderBuffer);
void initRenderBuffer(renderBuffer_t *renderBuffer);
void fillRenderBuffer(renderBuffer_t *renderBuffer);
void renderChunks(viewProjectionMatrices_t *vpMatrices, chunk_t *chunks, uint8_t chunksCount);
void renderChunk(viewProjectionMatrices_t *vpMatrices, chunk_t *chunk);
void pushVertices(int64_t offset, int64_t size, void *data);

#endif
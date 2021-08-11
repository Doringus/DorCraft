#include "chunk.h"
#include "dorcraftutils.h"
#include "noisegenerator.h"
#include "dorcraft.h"
#include "dorcraftogl.h"

#include <GL/glew.h>


static void pushQuad(int64_t offsetX, int64_t offsetY, int64_t offsetZ, uint32_t type, quadFace face, int64_t *quadsCount, renderBuffer_t *renderBuffer) {
	GLfloat vertices[] = {
		0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ, 0.0f, 0.0f, // bottom-left back
		1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ, 0.2f, 0.0f, // bottom-right back
		1.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ, 0.2f, 1.0f, // top-right back
		0.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ, 0.0f, 1.0f, // top-left back
		0.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ, 0.0f, 0.0f, // bottom-left front
		1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ, 0.2f, 0.0f, // bottom-right front
		1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ, 0.2f, 1.0f, // top-right front
		0.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ, 0.0f, 1.0f  // top-left front
	};
	GLuint indicesTemplate[6] = {
		0, 1, 3,
		1, 2, 3
	};

	int64_t offset = renderBuffer->verticesCount * 5;

	switch (face) {
		case quadFace::TOP: {
			memcpy(renderBuffer->vertices + offset, &vertices[35], 5 * sizeof(GLfloat)); // top-left front
			offset += 5;
			memcpy(renderBuffer->vertices + offset, &vertices[30], 5 * sizeof(GLfloat)); // top-right front
			offset += 5;
			memcpy(renderBuffer->vertices + offset, &vertices[10], 10 * sizeof(GLfloat)); // top-right back && top-left back
			break;
		}
		case quadFace::BOTTOM: {
			memcpy(renderBuffer->vertices + offset, &vertices[20], 10 * sizeof(GLfloat)); // bottom-left front && bottom-right front
			offset += 10;
			memcpy(renderBuffer->vertices + offset, &vertices[5], 5 * sizeof(GLfloat)); // bottom-right back
			offset += 5;
			memcpy(renderBuffer->vertices + offset, &vertices[0], 5 * sizeof(GLfloat)); // bottom-left back
			break;
		}
		case quadFace::LEFT: {
			memcpy(renderBuffer->vertices + offset, &vertices[35], 5 * sizeof(GLfloat)); // top-left front
			offset += 5;
			memcpy(renderBuffer->vertices + offset, &vertices[20], 5 * sizeof(GLfloat)); // bottom-left front
			offset += 5;
			memcpy(renderBuffer->vertices + offset, &vertices[0], 5 * sizeof(GLfloat)); // bottom-left back
			offset += 5;
			memcpy(renderBuffer->vertices + offset, &vertices[15], 5 * sizeof(GLfloat)); // top-left back
			break;
		}
		case quadFace::RIGHT: {
			memcpy(renderBuffer->vertices + offset, &vertices[30], 5 * sizeof(GLfloat)); // top-right front
			offset += 5;
			memcpy(renderBuffer->vertices + offset, &vertices[25], 5* sizeof(GLfloat)); // bottom-right front
			offset += 5;
			memcpy(renderBuffer->vertices + offset, &vertices[5], 5 * sizeof(GLfloat)); // bottom-right back
			offset += 5;
			memcpy(renderBuffer->vertices + offset, &vertices[10], 5 * sizeof(GLfloat)); // top-right back
			break;
		}	
		case quadFace::FRONT: {
			memcpy(renderBuffer->vertices + offset, &vertices[35], 5 * sizeof(GLfloat)); // top-left front
			offset += 5;
			memcpy(renderBuffer->vertices + offset, &vertices[20], 15 * sizeof(GLfloat)); // bottom-left front && bottom-right front &&  top-right front
			break;
		}
		case quadFace::BACK: {
			memcpy(renderBuffer->vertices + offset, &vertices[15], 5 * sizeof(GLfloat)); // top-left back
			offset += 5;
			memcpy(renderBuffer->vertices + offset, &vertices[0], 15 * sizeof(GLfloat)); // bottom-left back && bottom-right back && top-right back
			break;
		}
	}
	for (uint8_t i = 0; i < 6; ++i) {
		indicesTemplate[i] += (*quadsCount * 4);
	}
	memcpy(renderBuffer->indices + renderBuffer->indicesCount, &indicesTemplate[0], 6 * sizeof(GLuint));
	renderBuffer->indicesCount += 6;
	renderBuffer->verticesCount += 4;
	(*quadsCount)++;
}

void createChunk(chunk_t *chunk, int64_t offsetX, int64_t offsetZ) {
	chunk->offsetX = offsetX;
	chunk->offsetZ = offsetZ;
	chunk->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(offsetX, 0, offsetZ));
	initRenderBuffer(&chunk->renderInfo);
	createChunkBlocks(chunk);
	createChunkMesh(chunk);
	fillRenderBuffer(&chunk->renderInfo);
}

/// Start point of chunk e.g chunk coord - bottom-left back point
void createChunkMesh(chunk_t *chunk) {
	int64_t quadsCount = 0;
	for (int64_t x = 1; x < CHUNK_SIZE - 1; ++x) {
		for (int64_t y = 1; y < CHUNK_HEIGHT - 1; ++y) {
			for (int64_t z = 1; z < CHUNK_SIZE; ++z) {
				if (!chunk->blocks[get1DimIndex(x, y, z)]) {
					continue;
				}
				// for each block check neighbours
				//left
				if (!chunk->blocks[get1DimIndex(x - 1, y, z)]) {
					pushQuad(x, y, z, 1, LEFT, &quadsCount, &chunk->renderInfo);
				}
				//right
				if (!chunk->blocks[get1DimIndex(x + 1, y, z)]) {
					pushQuad(x, y, z, 1, RIGHT, &quadsCount, &chunk->renderInfo);
				}
				//top
				if (!chunk->blocks[get1DimIndex(x, y + 1, z)]) {
					pushQuad(x, y, z, 1, TOP, &quadsCount, &chunk->renderInfo);
				}
				// bottom
				if (!chunk->blocks[get1DimIndex(x, y - 1, z)]) {
					pushQuad(x, y, z, 1, BOTTOM, &quadsCount, &chunk->renderInfo);
				}
				//back
				if (!chunk->blocks[get1DimIndex(x, y, z + 1)]) {
					pushQuad(x, y, z, 1, FRONT, &quadsCount, &chunk->renderInfo);
				}
				//front
				if (!chunk->blocks[get1DimIndex(x, y, z - 1)]) {
					pushQuad(x, y, z, 1, BACK, &quadsCount, &chunk->renderInfo);
				}
			}
		}
	}
}

void createChunkBlocks(chunk_t *chunk) {
	double frequency = 1.0 / 90.0;
	double heightMap[CHUNK_SIZE][CHUNK_SIZE];
	// create height map
	for (int64_t i = chunk->offsetX, mx = 0; i < CHUNK_SIZE + chunk->offsetX; ++i, ++mx) {
		for (int64_t j = chunk->offsetZ, mz = 0; j < CHUNK_SIZE + chunk->offsetZ; ++j, ++mz) {
			heightMap[mx][mz] = octavePerlin((double)i * frequency, (double)j * frequency, 6) * 48;
		}
	}

	// fill chunk
	for (uint32_t x = 0; x < CHUNK_SIZE; ++x) {
		for (uint32_t y = 0; y < CHUNK_HEIGHT; ++y) {
			for (uint32_t z = 0; z < CHUNK_SIZE; ++z) {
				uint32_t blockType;
				if (y > heightMap[x][z]) {
					blockType = 0;
				}
				else {
					blockType = 1;
				}
				int t = get1DimIndex(x, y, z);
				chunk->blocks[get1DimIndex(x, y, z)] = blockType;
			}
		}
	}
}
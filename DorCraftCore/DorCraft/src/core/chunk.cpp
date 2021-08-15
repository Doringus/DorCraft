#include "chunk.h"
#include "dorcraftutils.h"
#include "noisegenerator.h"
#include "dorcraft.h"
#include "dorcraftogl.h"

#include <GL/glew.h>

int64_t getChunkCoord(int64_t coord) {
	if (coord >= 0) {
		return ((coord / CHUNK_SIZE) * CHUNK_SIZE);
	} else {
		return ((coord / CHUNK_SIZE) * CHUNK_SIZE - CHUNK_SIZE);
	}
}

static void pushQuad(int64_t offsetX, int64_t offsetY, int64_t offsetZ, uint32_t type, quadFace face, renderBuffer_t *renderBuffer) {
	renderBuffer->isDirty = true;

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
		indicesTemplate[i] += renderBuffer->verticesCount;
	}

	memcpy(renderBuffer->indices + renderBuffer->indicesCount, &indicesTemplate[0], 6 * sizeof(GLuint));
	renderBuffer->indicesCount += 6;
	renderBuffer->verticesCount += 4;
}

void createChunk(world_t *world, chunk_t *chunk, int64_t offsetX, int64_t offsetZ) {
	chunk->x = offsetX;
	chunk->z = offsetZ;
	chunk->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(offsetX, 0, offsetZ));
	initRenderBuffer(&chunk->renderInfo);
	createChunkBlocks(chunk);
	createChunkMesh(world, chunk);
	fillRenderBuffer(&chunk->renderInfo);
}

/// Start point of chunk e.g chunk coord - bottom-left back point
static void createChunkMesh(world_t *world, chunk_t *chunk) {
	int64_t quadsCount = 0;
	for (int64_t x = 0; x < CHUNK_SIZE; ++x) {
		for (int64_t y = 0; y < CHUNK_HEIGHT; ++y) {
			for (int64_t z = 0; z < CHUNK_SIZE; ++z) {
				if (!chunk->blocks[get1DimIndex(x, y, z)]) {
					continue;
				}
				// for each block check neighbours
				//left
				if (x > 0 && !chunk->blocks[get1DimIndex(x - 1, y, z)]) {
					pushQuad(x, y, z, 1, LEFT, &chunk->renderInfo);
				}
				//right
				if (x < CHUNK_SIZE - 1 && !chunk->blocks[get1DimIndex(x + 1, y, z)]) {
					pushQuad(x, y, z, 1, RIGHT, &chunk->renderInfo);
				}
				//top
				if (y < CHUNK_HEIGHT - 1 &&  !chunk->blocks[get1DimIndex(x, y + 1, z)]) {
					pushQuad(x, y, z, 1, TOP, &chunk->renderInfo);
				}
				// bottom
				if (y > 0 && !chunk->blocks[get1DimIndex(x, y - 1, z)]) {
					pushQuad(x, y, z, 1, BOTTOM, &chunk->renderInfo);
				}
				//front
				if (z < CHUNK_SIZE - 1 && !chunk->blocks[get1DimIndex(x, y, z + 1)]) {
					pushQuad(x, y, z, 1, FRONT, &chunk->renderInfo);
				}
				//back
				if (z > 0 && !chunk->blocks[get1DimIndex(x, y, z - 1)]) {
					pushQuad(x, y, z, 1, BACK, &chunk->renderInfo);
				}
			}
		}
	}
	// fill border blocks
	//Right border
	chunk_t *borderChunk = getChunk(&world->hashMap, chunk->x + CHUNK_SIZE, chunk->z);
	if (borderChunk != NULL) {
		for (int z = 0; z < CHUNK_SIZE; ++z) {
			for (int y = 0; y < CHUNK_HEIGHT; ++y) {
				// for right neighbour
				if (borderChunk->blocks[get1DimIndex(0, y, z)] && !chunk->blocks[get1DimIndex(CHUNK_SIZE - 1, y, z)]) {
					pushQuad(0, y, z, 1, LEFT, &borderChunk->renderInfo);
				}
				// for current chunk
				if (!borderChunk->blocks[get1DimIndex(0, y, z)] && chunk->blocks[get1DimIndex(CHUNK_SIZE - 1, y, z)]) {
					pushQuad(CHUNK_SIZE - 1, y, z, 1, RIGHT, &chunk->renderInfo);
				}
			}
		} 
	}
	//Left border
	borderChunk = getChunk(&world->hashMap, chunk->x - CHUNK_SIZE, chunk->z);
	if (borderChunk != NULL) {
		for (int z = 0; z < CHUNK_SIZE; ++z) {
			for (int y = 0; y < CHUNK_HEIGHT; ++y) {
				// for left chunk
				if (borderChunk->blocks[get1DimIndex(CHUNK_SIZE - 1, y, z)] && !chunk->blocks[get1DimIndex(0, y, z)]) {
					pushQuad(CHUNK_SIZE - 1, y, z, 1, RIGHT, &borderChunk->renderInfo);
				}
				// for current chunk
				if (!borderChunk->blocks[get1DimIndex(CHUNK_SIZE - 1, y, z)] && chunk->blocks[get1DimIndex(0, y, z)]) {
					pushQuad(0, y, z, 1, LEFT, &chunk->renderInfo);
				}
			}
		} 
	}
	
	// front border
	borderChunk = getChunk(&world->hashMap, chunk->x, chunk->z + CHUNK_SIZE);
	if (borderChunk != NULL) {
		for (int x = 0; x < CHUNK_SIZE; ++x) {
			for (int y = 0; y < CHUNK_HEIGHT; ++y) {
				// for front neighbour 
				if (borderChunk->blocks[get1DimIndex(x, y, 0)] && !chunk->blocks[get1DimIndex(x, y, CHUNK_SIZE - 1)]) {
					pushQuad(x, y, 0, 1, BACK, &borderChunk->renderInfo);
				}
				// for current chunk
				if (!borderChunk->blocks[get1DimIndex(x, y, 0)] && chunk->blocks[get1DimIndex(x, y, CHUNK_SIZE - 1)]) {
					pushQuad(x, y, CHUNK_SIZE - 1, 1, FRONT, &chunk->renderInfo);
				}
			}
		}
	}
	
	// back border
	borderChunk = getChunk(&world->hashMap, chunk->x, chunk->z - CHUNK_SIZE);
	if (borderChunk != NULL) {
		for (int x = 0; x < CHUNK_SIZE; ++x) {
			for (int y = 0; y < CHUNK_HEIGHT; ++y) {
				// for back neighbour
				if (borderChunk->blocks[get1DimIndex(x, y, CHUNK_SIZE - 1)] && !chunk->blocks[get1DimIndex(x, y, 0)]) {
					pushQuad(x, y, CHUNK_SIZE - 1, 1, FRONT, &borderChunk->renderInfo);
				}
				// for current chunk
				if (!borderChunk->blocks[get1DimIndex(x, y, CHUNK_SIZE - 1)] && chunk->blocks[get1DimIndex(x, y, 0)]) {
					pushQuad(x, y, 0, 1, BACK, &chunk->renderInfo);
				}
			}
		}
	}
}

static void createChunkBlocks(chunk_t *chunk) {
	double frequency = 1.0 / 90.0;
	double heightMap[CHUNK_SIZE][CHUNK_SIZE];
	// create height map
	for (int64_t i = chunk->x, mx = 0; i < CHUNK_SIZE + chunk->x; ++i, ++mx) {
		for (int64_t j = chunk->z, mz = 0; j < CHUNK_SIZE + chunk->z; ++j, ++mz) {
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
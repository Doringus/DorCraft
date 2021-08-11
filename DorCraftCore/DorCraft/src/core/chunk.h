#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include "../../vendor/glm/glm.hpp"

#include "dorcraftogl.h"
#include "dorcraft.h"




struct chunk_t {
	int64_t offsetX;
	int64_t offsetZ;
	renderBuffer_t renderInfo;
	glm::mat4 modelMatrix;
	memoryArena_t blocksArena;
	uint32_t *blocks; // 3-dim array contained blocks types
};

enum quadFace {
	TOP,
	BOTTOM,
	LEFT,
	RIGHT,
	FRONT,
	BACK
};

void createChunk(chunk_t *chunk, int64_t offsetX, int64_t offsetZ);
void createChunkMesh(chunk_t *chunk);
void createChunkBlocks(chunk_t *chunk);

static void pushQuad(int64_t offsetX, int64_t offsetY, int64_t offsetZ, uint32_t type, quadFace face, int64_t *quadsCount, renderBuffer_t *renderBuffer);

#endif
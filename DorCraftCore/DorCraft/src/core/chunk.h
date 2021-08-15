#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include "../../vendor/glm/glm.hpp"

#include "dorcraftogl.h"
#include "dorcraft.h"




struct chunk_t {
	int64_t x, z; /* Chunk coords - bottom-left back block*/
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

/* return origin chunk coordinate e.g. bottom-left back point based on passed coord from within chunk*/
int64_t getChunkCoord(int64_t coord);

void createChunk(world_t *world, chunk_t *chunk, int64_t offsetX, int64_t offsetZ);

static void createChunkMesh(world_t *world, chunk_t *chunk);

static void createChunkBlocks(chunk_t *chunk);

static void pushQuad(int64_t offsetX, int64_t offsetY, int64_t offsetZ, uint32_t type, quadFace face, renderBuffer_t *renderBuffer);

#endif
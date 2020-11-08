#pragma once

#include <stdint.h>

#include "dorcraft.h"

#define CHUNK_SIZE 16

struct chunk_t {
	uint8_t *blocks;
	int64_t offsetX;
	int64_t offsetY;
	int64_t offsetZ;
	chunk_t *next;
};

struct world_t {
	int16_t radius;
	chunk_t *chunksHash[32];
};

chunk_t *getChunk(world_t *world, int64_t chunkX, int64_t chunkY, int64_t chunkZ, memoryArena_t *chunksArena = 0);
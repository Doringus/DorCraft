#pragma once

#include <stdint.h>

#include "dorcraft.h"

#define CHUNK_SIZE 16
#define CHUNK_HEIGHT 256

struct chunk_t {
	uint8_t *blocks;
	int64_t offsetX;
	int64_t offsetY;
	int64_t offsetZ;
	int64_t glBufferOffset;
	int64_t verticesCount;
	memoryArena_t *renderArena;
	chunk_t *next;
	chunk_t *prev;
};

struct chunkHashSlot_t {
	chunkHashSlot_t *next;
	chunkHashSlot_t *prev;
	chunk_t *chunk;
};

struct world_t {
	int16_t radius;
	memoryArena_t worldArena;
	chunk_t *chunksHash[32];
};

/* 
	WARNING: result can be NULL if dont pass chunksArena 
	If chunksArena != 0, then it will allocate memory for chunk from passed arena
	If chunksArena == 0, then it will find chunk with passed coords
*/
chunk_t *getChunk(world_t *world, int64_t chunkX, int64_t chunkY, int64_t chunkZ, memoryArena_t *chunksArena = 0);

/*
	Finds invisible chunk and removes it from hash map
*/
chunk_t *popInvisibleChunk(world_t *world, int64_t centerX, int64_t centerZ, int64_t radius);

void moveChunk(world_t *world, chunk_t *chunk);
#ifndef DORCRAFTWORLD_H
#define DORCRAFTWORLD_H

#include <stdint.h>

struct renderBuffer_t;
struct chunk_t;

struct chunkHashSlot_t {
	chunkHashSlot_t *next;
	chunkHashSlot_t *prev;
	chunk_t *chunk; 
};

struct worldHashMap_t {
	chunkHashSlot_t *chunksHash[32] = {NULL};
};

struct world_t {
	int16_t radius;
	worldHashMap_t hashMap;
};

chunk_t *getChunk(worldHashMap_t *hashMap, int64_t chunkX, int64_t chunkZ);

void insertChunk(worldHashMap_t *hashMap, chunk_t *chunk);

void removeChunk(worldHashMap_t *hashMap, chunk_t *chunk);

void freeWorld(worldHashMap_t *hashMap);

static inline int64_t getHashSlot(int64_t chunkX, int64_t chunkZ);

static inline chunkHashSlot_t *createSlot(chunk_t *chunk, chunkHashSlot_t *prev);

static inline chunkHashSlot_t *getSlot(worldHashMap_t *hashMap, int64_t chunkX, int64_t chunkZ);

#endif
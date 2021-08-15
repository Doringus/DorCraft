#include "dorcraftworld.h"
#include "dorcraft.h"
#include "chunk.h"

#define HASH_SLOTS_COUNT 32

inline int64_t getHashSlot(int64_t chunkX, int64_t chunkZ) {
	int64_t hashValue = (11 * chunkX + chunkZ) % (HASH_SLOTS_COUNT - 1);
	return((HASH_SLOTS_COUNT - 1) & hashValue);
}

inline chunkHashSlot_t *createSlot(chunk_t *chunk, chunkHashSlot_t *prev) {
	chunkHashSlot_t *slot = (chunkHashSlot_t*)malloc(sizeof(chunkHashSlot_t));
	slot->chunk = chunk;
	slot->next = NULL;
	slot->prev = NULL;
	if (prev) {
		prev->next = slot;
		slot->prev = prev;
	}
	return (slot);
}

inline chunkHashSlot_t *getSlot(worldHashMap_t *hashMap, int64_t chunkX, int64_t chunkZ) {
	int64_t hashSlot = getHashSlot(chunkX, chunkZ);
	return (*(hashMap->chunksHash + hashSlot));
}

chunk_t *getChunk(worldHashMap_t *hashMap, int64_t chunkX, int64_t chunkZ) {
	chunkHashSlot_t *slot = getSlot(hashMap, chunkX, chunkZ);
	printf("GET %d %d\n", chunkX, chunkZ);
	if (!slot) {
		return NULL;
	}
	chunk_t *chunk = slot->chunk;
	while (slot->next) {
		if (chunkX == slot->chunk->x &&
			chunkZ == slot->chunk->z) {
			break;
		}
		slot = slot->next;
		chunk = slot->chunk;
	}
	if (chunkX != slot->chunk->x) {
		return NULL;
	}
	printf("POINTER %p\n", (void*)chunk);
	return(chunk);
}

void insertChunk(worldHashMap_t *hashMap, chunk_t *chunk) {
	chunkHashSlot_t *slot = getSlot(hashMap, chunk->x, chunk->z);
	int64_t slotIndex = getHashSlot(chunk->x, chunk->z);
	printf("INSERT %p\n", (void*)chunk);

	if (!slot) {
		hashMap->chunksHash[slotIndex] = createSlot(chunk, NULL);
	} else {
		while (slot->next) {
			slot = slot->next;
		}
		slot->next = createSlot(chunk, slot);
	}
}

void removeChunk(worldHashMap_t *hashMap, chunk_t *chunk) {
	chunkHashSlot_t *slot = getSlot(hashMap, chunk->x, chunk->z);
	int64_t slotIndex = getHashSlot(chunk->x, chunk->z);

	if (!slot) {
		return;
	}
	while (slot) {
		if (chunk->x == slot->chunk->x &&
			chunk->z == slot->chunk->z) {
			break;
		}
		slot = slot->next;
	}
	if (!slot) {
		return;
	}
	// remove first in sequence
	if (!slot->prev) {
		hashMap->chunksHash[slotIndex] = slot->next;
		if (slot->next) {
			slot->next->prev = NULL;
		}
	} else {
		slot->prev->next = slot->next;
		if (slot->next) {
			slot->next->prev = slot->prev;
		}
	}
	free(slot);
}

void freeWorld(worldHashMap_t * hashMap) {
}

/*
chunk_t *popInvisibleChunk(world_t *world, int64_t centerX, int64_t centerZ, int64_t radius) {
	chunk_t *result = NULL;
	int32_t hashSlot;
	for (hashSlot = 0; hashSlot < 32; ++hashSlot) {
		if (world->chunksHash[hashSlot]) {
			chunk_t *chunk = world->chunksHash[hashSlot];
			do {
				int64_t x1, z1, x2, z2, x3, z3, x4, z4;
				x1 = chunk->offsetX; z1 = chunk->offsetZ;
				x2 = x1 + CHUNK_SIZE; z2 = z1;
				x3 = x2; z3 = z2 + CHUNK_SIZE;
				x4 = x1; z4 = z3;
				if (pow((x1 - centerX), 2) + pow(z1 - centerZ, 2) >= pow(radius, 2) &&
					pow((x2 - centerX), 2) + pow(z2 - centerZ, 2) >= pow(radius, 2) &&
					pow((x3 - centerX), 2) + pow(z3 - centerZ, 2) >= pow(radius, 2) &&
					pow((x4 - centerX), 2) + pow(z4 - centerZ, 2) >= pow(radius, 2)) {
						result = chunk;
						removeFromHash(world, result, hashSlot);
						result->prev = NULL;
						result->next = NULL;
						return(result);
				}
				chunk = chunk->next;
			} while (chunk);
		}
	}
	return(result);
} */
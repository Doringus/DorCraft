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
	if (chunkX != slot->chunk->x || chunkZ != slot->chunk->z) {
		return NULL;
	}
	return(chunk);
}

void insertChunk(worldHashMap_t *hashMap, chunk_t *chunk) {
	chunkHashSlot_t *slot = getSlot(hashMap, chunk->x, chunk->z);
	int64_t slotIndex = getHashSlot(chunk->x, chunk->z);

	if (!slot) {
		hashMap->chunksHash[slotIndex] = createSlot(chunk, NULL);
	} else {
		while (slot->next) {
			slot = slot->next;
		}
		slot->next = createSlot(chunk, slot);
	}
}

chunk_t *removeChunk(worldHashMap_t *hashMap, int64_t chunkX, int64_t chunkZ) {
	chunkHashSlot_t *slot = getSlot(hashMap, chunkX, chunkZ);
	int64_t slotIndex = getHashSlot(chunkX, chunkZ);
	chunk_t *result = NULL;

	if (!slot) {
		return(NULL);
	}
	while (slot) {
		if (chunkX == slot->chunk->x &&
			chunkZ == slot->chunk->z) {
			result = slot->chunk;
			break;
		}
		slot = slot->next;
	}
	if (!slot) {
		return(NULL);
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
	return(result);
}

void freeWorld(worldHashMap_t * hashMap) {
}


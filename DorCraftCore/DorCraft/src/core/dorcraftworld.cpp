#include "dorcraftworld.h"
#include "dorcraft.h"

int64_t getHashSlot(int64_t chunkX, int64_t chunkY, int64_t chunkZ) {
	int64_t hashValue = (11 * chunkX + 6 * chunkY + 3 * chunkZ) % 31;
	return((32 - 1) & hashValue );
}

chunk_t *getChunk(world_t *world, int64_t chunkX, int64_t chunkY, int64_t chunkZ, memoryArena_t *chunksArena) {
	int64_t hashSlot = getHashSlot(chunkX, chunkY, chunkZ);
	chunk_t *chunk = *(world->chunksHash + hashSlot);
	if (!chunk && !chunksArena) {
		return(chunk);
	}
	if (!chunk && chunksArena) {
		chunk = pushStruct(chunksArena, chunk_t);
		*(world->chunksHash + hashSlot) = chunk;
		return(chunk);
	}
	do {
		if (chunksArena && !chunk->next) {
			chunk_t *newChunk = pushStruct(chunksArena, chunk_t);
			chunk->next = newChunk;
			chunk = newChunk;
			break;
		}
		if (chunkX == chunk->offsetX &&
			chunkY == chunk->offsetY &&
			chunkZ == chunk->offsetZ) {
			break;
		}
		chunk = chunk->next;
	} while (chunk);
	return(chunk);
}

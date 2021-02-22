#include "dorcraftworld.h"
#include "dorcraft.h"

int64_t getHashSlot(int64_t chunkX, int64_t chunkY, int64_t chunkZ) {
	int64_t hashValue = (11 * chunkX + 6 * chunkY + 3 * chunkZ) % 31;
	return((32 - 1) & hashValue );
}

chunk_t *getChunk(world_t *world, int64_t chunkX, int64_t chunkY, int64_t chunkZ) {

}

chunk_t *createChunk(world_t *world, int64_t chunkX, int64_t chunkY, int64_t chunkZ) {

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
		if (chunkX == chunk->offsetX &&
			chunkY == chunk->offsetY &&
			chunkZ == chunk->offsetZ) {
			break;
		}
		if (chunksArena && !chunk->next) {
			chunk_t *newChunk = pushStruct(chunksArena, chunk_t);
			chunk->next = newChunk;
			newChunk->prev = chunk;
			chunk = newChunk;
			break;
		}
		chunk = chunk->next;
	} while (chunk);
	return(chunk);
}

static void removeFromHash(world_t *world, chunk_t *chunk, int32_t hashSlot) {
	if (chunk->prev) {
		chunk->prev->next = chunk->next;
		if (chunk->next) {
			chunk->next->prev = chunk->prev;
		}
	}
	else {
		if (chunk->next) {
			world->chunksHash[hashSlot] = chunk->next;
			chunk->next->prev = NULL;
		}
		else {
			world->chunksHash[hashSlot] = NULL;
		}
	}
}

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
}

void moveChunk(world_t *world, chunk_t *chunk) {
	int64_t hashSlot = getHashSlot(chunk->offsetX, chunk->offsetY, chunk->offsetZ);
	chunk_t *chunkSlot = *(world->chunksHash + hashSlot);
	if (!chunkSlot) {
		*(world->chunksHash + hashSlot) = chunk;
		return;
	}
	do {
		if (!chunkSlot->next) {
			chunkSlot->next = chunk;
			chunk->prev = chunkSlot;
			break;
		}
		chunkSlot = chunkSlot->next;
	} while (chunkSlot);
}
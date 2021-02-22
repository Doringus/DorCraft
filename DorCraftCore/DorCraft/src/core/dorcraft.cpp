#include "dorcraft.h"

#include <inttypes.h>

#include "dorcraftutils.h"
#include "dorcraftogl.h"
#include "noisegenerator.h"
#include "dorcraftworld.h"

static renderGroup_t renderGroup;
static gameState_t *gameState;

static void moveAndRotateCamera(gameInput_t *input, camera_t *camera) {
	/* Move camera */
	if (input->forward.pressed) {
		camera->position += (GLfloat)(3.0f * input->dt) * camera->front;
	}
	if (input->back.pressed) {
		camera->position -= (GLfloat)(3.0f * input->dt) * camera->front;
	}
	if (input->right.pressed) {
		camera->position += glm::normalize(glm::cross(camera->front, camera->up)) * (GLfloat)(3.0f * input->dt);
	}
	if (input->left.pressed) {
		camera->position -= glm::normalize(glm::cross(camera->front, camera->up)) * (GLfloat)(3.0f * input->dt);
	}
	/* Rotate camera */
	double offsetX = input->mousePosition.currentX - input->mousePosition.prevX;
	double offsetY = input->mousePosition.currentY - input->mousePosition.prevY;
	offsetX *= 3.0f * input->dt;
	offsetY *= 3.0f * input->dt;
	camera->yaw += offsetX;
	camera->pitch -= offsetY;

	if (camera->pitch > 89.0f) {
		camera->pitch = 89.0f;
	}
	if (camera->pitch < -89.0f) {
		camera->pitch = -89.0f;
	}
	camera->front.x = cos(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
	camera->front.y = sin(glm::radians(camera->pitch));
	camera->front.z = sin(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
	camera->front = glm::normalize(camera->front);
}


static void pushQuad(int64_t offsetX, int64_t offsetY, int64_t offsetZ, uint8_t type, quadFace face, memoryArena_t *renderArena) {
	GLfloat vertices[] = {
		0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ, 0.0f, 0.0f,
		1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ, 0.20f, 0.0f,
		1.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ, 0.20f, 1.0f,
		1.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ, 0.20f, 1.0f,
		0.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ, 0.0f, 1.0f,
		0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ, 0.0f, 0.0f,

		0.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ, 0.0f, 0.0f,
		1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ, 0.20f, 0.0f,
		1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ, 0.20f, 1.0f,
		1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ, 0.20f, 1.0f,
		0.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ, 0.0f, 1.0f,
		0.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ, 0.0f, 0.0f,

		0.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ, 0.20f, 0.0f,
		0.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ, 0.20f, 1.0f,
		0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ, 0.0f, 1.0f,
		0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ, 0.0f, 1.0f,
		0.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ, 0.0f, 0.0f,
		0.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ, 0.20f, 0.0f,

		1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ, 0.20f, 0.0f,
		1.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ, 0.20f, 1.0f,
		1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ, 0.0f, 1.0f,
		1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ, 0.0f, 1.0f,
		1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ, 0.0f, 0.0f,
		1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ, 0.20f, 0.0f,

		0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ, 0.0f, 1.0f,
		1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ, 0.20f, 1.0f,
		1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ, 0.20f, 0.0f,
		1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ, 0.20f, 0.0f,
		0.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ, 0.0f, 0.0f,
		0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ, 0.0f, 1.0f,

		0.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ, 0.0f, 1.0f,
		1.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ, 0.20f, 1.0f,
		1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ, 0.20f, 0.0f,
		1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ, 0.20f, 0.0f,
		0.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ, 0.0f, 0.0f,
		0.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ, 0.0f, 1.0f
	};
	uint8_t *buf = (uint8_t*)pushArray(renderArena, 30, GLfloat);
	switch (face) {
		case quadFace::TOP: {
			memcpy(buf, &vertices[150], 30 * sizeof(GLfloat));
			break;
		}
		case quadFace::BOTTOM: {
			memcpy(buf, &vertices[120], 30 * sizeof(GLfloat));
			break;
		}
		case quadFace::LEFT: {
			memcpy(buf, &vertices[60], 30 * sizeof(GLfloat));
			break;
		}
		case quadFace::RIGHT: {
			memcpy(buf, &vertices[90], 30 * sizeof(GLfloat));
			break;
		}
		case quadFace::FRONT: {
			memcpy(buf, &vertices[30], 30 * sizeof(GLfloat));
			break;
		}
		case quadFace::BACK: {
			memcpy(buf, &vertices[0], 30 * sizeof(GLfloat));
			break;
		}
	}
}

static void createChunkMesh(world_t *world, chunk_t *chunk) {
	if (chunk == NULL) {
		return;
	}
	/* Left neighbour*/
	chunk_t *nearChunk = getChunk(world, chunk->offsetX - 16, chunk->offsetY, chunk->offsetZ);
	if (nearChunk != NULL) {
		for (int32_t i = 0; i < CHUNK_SIZE; ++i) {
			for (int32_t j = 0; j < CHUNK_SIZE; ++j) {
				if (nearChunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * j + 15] != 0) {
					if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * j] == 0) {
						pushQuad(nearChunk->offsetX + 15, nearChunk->offsetY + j, nearChunk->offsetZ + i, 1, quadFace::RIGHT, nearChunk->renderArena);
					}
				}
				if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * j] != 0) {
					if (nearChunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * j + 15] == 0) {
						pushQuad(chunk->offsetX, chunk->offsetY + j, chunk->offsetZ + i, 1, quadFace::LEFT, chunk->renderArena);
					}
				}
			}
		}
	}
	/* Right neighbour*/
	nearChunk = getChunk(world, chunk->offsetX + 16, chunk->offsetY, chunk->offsetZ);
	if (nearChunk != NULL) {
		for (int32_t i = 0; i < CHUNK_SIZE; ++i) {
			for (int32_t j = 0; j < CHUNK_SIZE; ++j) {
				if (nearChunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * j] != 0) {
					if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * j + 15] == 0) {
						pushQuad(nearChunk->offsetX, nearChunk->offsetY + j, nearChunk->offsetZ + i, 1, quadFace::LEFT, nearChunk->renderArena);
					}
				}
				if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * j + 15] != 0) {
					if (nearChunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * j] == 0) {
						pushQuad(chunk->offsetX + 15, chunk->offsetY + j, chunk->offsetZ + i, 1, quadFace::RIGHT, chunk->renderArena);
					}
				}
			}
		}
	}
	/* Top neighbour*/
	nearChunk = getChunk(world, chunk->offsetX, chunk->offsetY + 16, chunk->offsetZ);
	if (nearChunk != NULL) {
		for (int32_t i = 0; i < CHUNK_SIZE; ++i) {
			for (int32_t j = 0; j < CHUNK_SIZE; ++j) {
				if (nearChunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + j] != 0) {
					if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * 15 + j] == 0) {
						pushQuad(nearChunk->offsetX + j, nearChunk->offsetY, nearChunk->offsetZ + i, 1, quadFace::BOTTOM, nearChunk->renderArena);
					}
				}
				if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * 15 + j] != 0) {
					if (nearChunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + j] == 0) {
						pushQuad(chunk->offsetX + j, chunk->offsetY + 15, chunk->offsetZ + i, 1, quadFace::TOP, chunk->renderArena);
					}
				}
			}
		}
	}
	/* Bottom neighbour*/
	nearChunk = getChunk(world, chunk->offsetX, chunk->offsetY - 16, chunk->offsetZ);
	if (nearChunk != NULL) {
		for (int32_t i = 0; i < CHUNK_SIZE; ++i) {
			for (int32_t j = 0; j < CHUNK_SIZE; ++j) {
				if (nearChunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * 15 + j] != 0) {
					if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + j] == 0) {
						pushQuad(nearChunk->offsetX + j, nearChunk->offsetY + 15, nearChunk->offsetZ + i, 1, quadFace::TOP, nearChunk->renderArena);
					}
				}
				if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + j] != 0) {
					if (nearChunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * 15 + j] == 0) {
						pushQuad(chunk->offsetX + j, chunk->offsetY, chunk->offsetZ + i, 1, quadFace::BOTTOM, chunk->renderArena);
					}
				}
			}
		}
	} 
	/* Back neighbour*/
	nearChunk = getChunk(world, chunk->offsetX, chunk->offsetY, chunk->offsetZ - 16);
	if (nearChunk != NULL) {
		for (int32_t i = 0; i < CHUNK_SIZE; ++i) {
			for (int32_t j = 0; j < CHUNK_SIZE; ++j) {
				if (nearChunk->blocks[CHUNK_SIZE * CHUNK_SIZE * 15 + CHUNK_SIZE * j + i] != 0) {
					if (chunk->blocks[CHUNK_SIZE * j + i] == 0) {
						pushQuad(nearChunk->offsetX + i, nearChunk->offsetY + j, nearChunk->offsetZ + 15, 1, quadFace::FRONT, nearChunk->renderArena);
					}
				}
				if (chunk->blocks[CHUNK_SIZE * j + i] != 0) {
					if (nearChunk->blocks[CHUNK_SIZE * CHUNK_SIZE * 15 + CHUNK_SIZE * j + i] == 0) {
						pushQuad(chunk->offsetX + i, chunk->offsetY + j, chunk->offsetZ, 1, quadFace::BACK, chunk->renderArena);
					}
				}
			}
		}
	} 
	/* Front neighbour*/
	nearChunk = getChunk(world, chunk->offsetX, chunk->offsetY, chunk->offsetZ + 16);
	if (nearChunk != NULL) {
		for (int32_t i = 0; i < CHUNK_SIZE; ++i) {
			for (int32_t j = 0; j < CHUNK_SIZE; ++j) {
				if (nearChunk->blocks[CHUNK_SIZE * j + i] != 0) {
					if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * 15 + CHUNK_SIZE * j + i] == 0) {
						pushQuad(nearChunk->offsetX + i, nearChunk->offsetY + j, nearChunk->offsetZ, 1, quadFace::BACK, nearChunk->renderArena);
					}
				}
				if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * 15 + CHUNK_SIZE * j + i] != 0) {
					if (nearChunk->blocks[CHUNK_SIZE * j + i] == 0) {
						pushQuad(chunk->offsetX + i, chunk->offsetY + j, chunk->offsetZ + 15, 1, quadFace::FRONT, chunk->renderArena);
					}
				}
			}
		}
	} 

	for (uint32_t i = 0; i < CHUNK_SIZE; ++i) {
		for (uint32_t j = 0; j < CHUNK_SIZE; ++j) {
			for (uint32_t k = 0; k < CHUNK_SIZE; ++k) {
				if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * k + CHUNK_SIZE * j + i] != 0) {
					/* Right neighbour */
					if (i < CHUNK_SIZE - 1) {
						if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * k + CHUNK_SIZE * j + (i + 1)] == 0) {
							pushQuad(chunk->offsetX + i, chunk->offsetY + j, chunk->offsetZ + k, 1, quadFace::RIGHT, chunk->renderArena);
						}
					}
					/* Left neighbour */
					if (i > 0) {
						if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * k + CHUNK_SIZE * j + (i - 1)] == 0) {
							pushQuad(chunk->offsetX + i, chunk->offsetY + j, chunk->offsetZ + k, 1, quadFace::LEFT, chunk->renderArena);
						}
					}
					/* Top neighbour */
					if (j < CHUNK_SIZE - 1) {
						if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * k + CHUNK_SIZE * (j + 1) + i] == 0) {
							pushQuad(chunk->offsetX + i, chunk->offsetY + j, chunk->offsetZ + k, 1, quadFace::TOP, chunk->renderArena);
						}
					}
					/* Bottom neighbour */
					if (j > 0) {
						if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * k + CHUNK_SIZE * (j - 1) + i] == 0) {
							pushQuad(chunk->offsetX + i, chunk->offsetY + j, chunk->offsetZ + k, 1, quadFace::BOTTOM, chunk->renderArena);
						}
					}
					/* Front neighbour */
					if (k < CHUNK_SIZE - 1) {
						if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * (k + 1) + CHUNK_SIZE * j + i] == 0) {
							pushQuad(chunk->offsetX + i, chunk->offsetY + j, chunk->offsetZ + k, 1, quadFace::FRONT, chunk->renderArena);
						}
					}
					/* Back neighbour */
					if (k > 0) {
						if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * (k - 1) + CHUNK_SIZE * j + i] == 0) {
							pushQuad(chunk->offsetX + i, chunk->offsetY + j, chunk->offsetZ + k, 1, quadFace::BACK, chunk->renderArena);
						}
					}
				}
			}
		}
	}
	pushVertices(chunk->glBufferOffset, chunk->renderArena->used ,chunk->renderArena->base);
}

static int64_t getChunkRenderBufferOffset() {
	static int64_t offset = 0;
	int64_t result = offset;
	offset += KILOBYTES(2880);
	assert(offset <= MEGABYTES(500));
	return(result);
}

static void createChunksSection(int64_t offsetX, int64_t offsetZ, gameState_t *gameState, bool rewriteChunks, int64_t centerX, int64_t centerZ) {
	double frequency = 1.0 / 90.0;
	
	/* Create height map */
	double heightMap[16][16];
	for (int64_t i = offsetX, mx = 0; i < CHUNK_SIZE + offsetX; ++i, ++mx) {
		for (int64_t j = offsetZ, mz = 0; j < CHUNK_SIZE + offsetZ; ++j, ++mz) {
			heightMap[mx][mz] = octavePerlin((double)i * frequency, (double)j * frequency, 6) * 48;
		}
	}
	/* Fill chunks */
	chunk_t *chunk = NULL;
	for (uint64_t y = 0; y < WORLD_HEIGHT; ++y) {
		if (y % 16 == 0) {
			if (rewriteChunks) {
				chunk = popInvisibleChunk(gameState->world, centerX, centerZ ,CHUNK_SIZE + 8);
				assert(chunk != NULL);
				popArena(chunk->renderArena);
			} else {
				chunk = getChunk(gameState->world, offsetX, y, offsetZ, &gameState->worldArena);
				chunk->blocks = pushArray(&gameState->chunksData, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, uint8_t);
				chunk->renderArena = pushStruct(&gameState->renderData, memoryArena_t);
				chunk->glBufferOffset = getChunkRenderBufferOffset();
				initializeArena(chunk->renderArena, CHUNK_RENDER_BUFFER_SIZE, (uint8_t*)reserveMemory(&gameState->renderData, CHUNK_RENDER_BUFFER_SIZE));
			}
			chunk->offsetX = offsetX;
			chunk->offsetY = y;
			chunk->offsetZ = offsetZ;
			if (rewriteChunks) {
				moveChunk(gameState->world, chunk);
			}
		}
		for (uint64_t x = 0; x < CHUNK_SIZE; ++x) {
			for (uint64_t z = 0; z < CHUNK_SIZE; ++z) {
				uint8_t blockType;
				if (y  > heightMap[x][z]) {
					blockType = 0;
				} else {
					blockType = 1;
				}
				chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * z + CHUNK_SIZE * (y % 16) + x] = blockType;
			}
		}
	}
}

static inline void createChunkSectionMesh(int64_t offsetX, int64_t offsetZ, world_t *world) {
	for (int i = 0; i < WORLD_HEIGHT / CHUNK_SIZE; ++i) {
		createChunkMesh(world,
			getChunk(world, offsetX, (int64_t)CHUNK_SIZE * i, offsetZ));
	}
}

static void initializeArena(memoryArena_t *arena, uint64_t size, uint8_t *base) {
	arena->base = base;
	arena->size = size;
	arena->used = 0;
}

static inline int64_t getChunkCoord(int64_t coord) {
	if (coord > 0) {
		return (coord / CHUNK_SIZE) * CHUNK_SIZE;
	} else {
		return (coord / CHUNK_SIZE) * CHUNK_SIZE - CHUNK_SIZE;
	}
}

static void checkDrawableChunks(gameState_t *gameState) {
	/*
		New chunks replace old and far chunks
	*/
	if (!getChunk(gameState->world, getChunkCoord((int64_t)gameState->camera.position.x) - CHUNK_SIZE, getChunkCoord(gameState->camera.position.y),
		getChunkCoord(gameState->camera.position.z))) {
		createChunksSection(getChunkCoord((int64_t)gameState->camera.position.x) - CHUNK_SIZE, getChunkCoord(gameState->camera.position.z),
							gameState, true, getChunkCoord((int64_t)gameState->camera.position.x) + CHUNK_SIZE / 2, getChunkCoord((int64_t)gameState->camera.position.z) + CHUNK_SIZE / 2);
		createChunkSectionMesh(getChunkCoord((int64_t)gameState->camera.position.x) - CHUNK_SIZE, getChunkCoord(gameState->camera.position.z),
							gameState->world);
	}
	if (!getChunk(gameState->world, getChunkCoord((int64_t)gameState->camera.position.x) - CHUNK_SIZE, getChunkCoord(gameState->camera.position.y),
		getChunkCoord((int64_t)gameState->camera.position.z) + CHUNK_SIZE)) {
		createChunksSection(getChunkCoord((int64_t)gameState->camera.position.x) - CHUNK_SIZE, getChunkCoord((int64_t)gameState->camera.position.z) + CHUNK_SIZE,
			gameState, true, getChunkCoord((int64_t)gameState->camera.position.x) + CHUNK_SIZE / 2, getChunkCoord((int64_t)gameState->camera.position.z) + CHUNK_SIZE / 2);
		createChunkSectionMesh(getChunkCoord((int64_t)gameState->camera.position.x) - CHUNK_SIZE, getChunkCoord((int64_t)gameState->camera.position.z) + CHUNK_SIZE,
			gameState->world);
	}
	if (!getChunk(gameState->world, getChunkCoord(gameState->camera.position.x), getChunkCoord((int64_t)gameState->camera.position.y),
		getChunkCoord((int64_t)gameState->camera.position.z) + CHUNK_SIZE)) {
		createChunksSection(getChunkCoord((int64_t)gameState->camera.position.x), getChunkCoord((int64_t)gameState->camera.position.z) + CHUNK_SIZE,
			gameState, true, getChunkCoord((int64_t)gameState->camera.position.x) + CHUNK_SIZE / 2, getChunkCoord((int64_t)gameState->camera.position.z) + CHUNK_SIZE / 2);
		createChunkSectionMesh(getChunkCoord((int64_t)gameState->camera.position.x), getChunkCoord((int64_t)gameState->camera.position.z) + CHUNK_SIZE,
			gameState->world);
	}
	if (!getChunk(gameState->world, getChunkCoord((int64_t)gameState->camera.position.x) + CHUNK_SIZE, getChunkCoord(gameState->camera.position.y), 
		getChunkCoord((int64_t)gameState->camera.position.z) + CHUNK_SIZE)) {
		createChunksSection(getChunkCoord((int64_t)gameState->camera.position.x) + CHUNK_SIZE, getChunkCoord((int64_t)gameState->camera.position.z) + CHUNK_SIZE,
			gameState, true, getChunkCoord((int64_t)gameState->camera.position.x) + CHUNK_SIZE / 2, getChunkCoord((int64_t)gameState->camera.position.z) + CHUNK_SIZE / 2);
		createChunkSectionMesh(getChunkCoord((int64_t)gameState->camera.position.x) + CHUNK_SIZE, getChunkCoord((int64_t)gameState->camera.position.z) + CHUNK_SIZE,
			gameState->world);
	}
	if (!getChunk(gameState->world, getChunkCoord((int64_t)gameState->camera.position.x) + CHUNK_SIZE, getChunkCoord(gameState->camera.position.y), 
		getChunkCoord(gameState->camera.position.z))) {
		createChunksSection(getChunkCoord((int64_t)gameState->camera.position.x) + CHUNK_SIZE, getChunkCoord(gameState->camera.position.z),
			gameState, true, getChunkCoord((int64_t)gameState->camera.position.x) + CHUNK_SIZE / 2, getChunkCoord((int64_t)gameState->camera.position.z) + CHUNK_SIZE / 2);
		createChunkSectionMesh(getChunkCoord((int64_t)gameState->camera.position.x) + CHUNK_SIZE, getChunkCoord(gameState->camera.position.z),
			gameState->world);
	}
	if (!getChunk(gameState->world, getChunkCoord((int64_t)gameState->camera.position.x) + CHUNK_SIZE, getChunkCoord(gameState->camera.position.y),
		getChunkCoord((int64_t)gameState->camera.position.z) - CHUNK_SIZE)) {
		createChunksSection(getChunkCoord((int64_t)gameState->camera.position.x) + CHUNK_SIZE, getChunkCoord((int64_t)gameState->camera.position.z) - CHUNK_SIZE,
			gameState, true, getChunkCoord((int64_t)gameState->camera.position.x) + CHUNK_SIZE / 2, getChunkCoord((int64_t)gameState->camera.position.z) + CHUNK_SIZE / 2);
		createChunkSectionMesh(getChunkCoord((int64_t)gameState->camera.position.x) + CHUNK_SIZE, getChunkCoord((int64_t)gameState->camera.position.z) - CHUNK_SIZE,
			gameState->world);
	}
	if (!getChunk(gameState->world, getChunkCoord(gameState->camera.position.x), getChunkCoord(gameState->camera.position.y),
		getChunkCoord((int64_t)gameState->camera.position.z) - CHUNK_SIZE)) {
		createChunksSection(getChunkCoord((int64_t)gameState->camera.position.x), getChunkCoord((int64_t)gameState->camera.position.z) - CHUNK_SIZE,
			gameState, true, getChunkCoord((int64_t)gameState->camera.position.x) + CHUNK_SIZE / 2, getChunkCoord((int64_t)gameState->camera.position.z) + CHUNK_SIZE / 2);
		createChunkSectionMesh(getChunkCoord((int64_t)gameState->camera.position.x), getChunkCoord((int64_t)gameState->camera.position.z) - CHUNK_SIZE,
			gameState->world);
	}
	if (!getChunk(gameState->world, getChunkCoord((int64_t)gameState->camera.position.x) - CHUNK_SIZE, 
		getChunkCoord(gameState->camera.position.y), getChunkCoord((int64_t)gameState->camera.position.z) - CHUNK_SIZE)) {
		createChunksSection(getChunkCoord((int64_t)gameState->camera.position.x) - CHUNK_SIZE, getChunkCoord((int64_t)gameState->camera.position.z) - CHUNK_SIZE,
			gameState, true, getChunkCoord((int64_t)gameState->camera.position.x) + CHUNK_SIZE / 2, getChunkCoord((int64_t)gameState->camera.position.z) + CHUNK_SIZE / 2);
		createChunkSectionMesh(getChunkCoord((int64_t)gameState->camera.position.x) - CHUNK_SIZE, getChunkCoord((int64_t)gameState->camera.position.z) - CHUNK_SIZE,
			gameState->world);
	} 
}


void gameUpdateAndRender(gameInput_t *input, gameMemory_t *memory, renderOutputArea_t *renderOutputArea) {
	static int64_t prevChunkX = 0, prevChunkZ = 0;
	if (!memory->isInitialized) {
		gameState = (gameState_t*)memory->permanentStorage;
		initializeArena(&gameState->worldArena, KILOBYTES(30),
			(uint8_t*)memory->permanentStorage + sizeof(gameState_t));
		initializeArena(&gameState->chunksData, MEGABYTES(10),
			(uint8_t*)memory->permanentStorage + sizeof(gameState_t) + gameState->worldArena.size);
		initializeArena(&gameState->renderData, MEGABYTES(500),
			(uint8_t*)memory->permanentStorage + sizeof(gameState_t) + gameState->worldArena.size + gameState->chunksData.size);
		gameState->world = pushStruct(&gameState->worldArena, world_t);
		gameState->world->radius = 1;
		perlinSeed(12041218833);
		int64_t blocksCount = 0;
		for (int i = -gameState->world->radius; i <= gameState->world->radius; ++i) {
			for (int j = -gameState->world->radius; j <= gameState->world->radius; ++j) {
				createChunksSection(i * CHUNK_SIZE, j * CHUNK_SIZE, gameState);
				createChunkSectionMesh(i * CHUNK_SIZE, j * CHUNK_SIZE, gameState->world);
			}
		}
		gameState->camera.position = glm::vec3(8.0f, 47.0f, 8.0f);
		gameState->camera.front = glm::vec3(0.0f, 0.0f, -1.0f);
		gameState->camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
		gameState->camera.yaw = -90.0f;

		renderGroup.modelMatrix = glm::mat4(1.0f);
		renderGroup.projectionMatrix = glm::perspective(45.0f, (GLfloat)renderOutputArea->areaWidth / (GLfloat)renderOutputArea->areaHeight, 0.1f, 2500.0f);

		prevChunkX = getChunkCoord(gameState->camera.position.x);
		prevChunkZ = getChunkCoord(gameState->camera.position.z);

		memory->isInitialized = true;
	}
	moveAndRotateCamera(input, &gameState->camera);
	renderGroup.viewMatrix = glm::lookAt(gameState->camera.position, gameState->camera.position + gameState->camera.front, gameState->camera.up);
	int64_t currentChunkX = getChunkCoord(gameState->camera.position.x);
	int64_t currentChunkZ = getChunkCoord(gameState->camera.position.z);
	if (currentChunkX != prevChunkX || currentChunkZ != prevChunkZ) {
		prevChunkX = currentChunkX;
		prevChunkZ = currentChunkZ;
		checkDrawableChunks(gameState);
	}
	renderToOutput(&renderGroup);

}


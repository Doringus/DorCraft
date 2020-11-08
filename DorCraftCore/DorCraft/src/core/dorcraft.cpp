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

static void pushQuad(int64_t offsetX, int64_t offsetY, int64_t offsetZ, uint8_t type, quadFace face, memoryArena_t *arena) {
	GLfloat vertices[] = {
		0.0f + offsetX , 0.0f + offsetY , 0.0f + offsetZ , 0.0f, 0.0f,
		1.0f + offsetX , 0.0f + offsetY , 0.0f + offsetZ , 0.20f, 0.0f,
		1.0f + offsetX ,  1.0f + offsetY , 0.0f + offsetZ , 0.20f, 1.0f,
		1.0f + offsetX ,  1.0f + offsetY , 0.0f + offsetZ ,  0.20f, 1.0f,
		0.0f + offsetX ,  1.0f + offsetY , 0.0f + offsetZ ,  0.0f, 1.0f,
		0.0f + offsetX , 0.0f + offsetY , 0.0f + offsetZ ,  0.0f, 0.0f,

		0.0f + offsetX , 0.0f + offsetY ,  1.0f + offsetZ ,  0.0f, 0.0f,
		1.0f + offsetX , 0.0f + offsetY ,  1.0f + offsetZ ,  0.20f, 0.0f,
		1.0f + offsetX ,  1.0f + offsetY ,  1.0f + offsetZ ,  0.20f, 1.0f,
		1.0f + offsetX ,  1.0f + offsetY ,  1.0f + offsetZ ,  0.20f, 1.0f,
		0.0f + offsetX ,  1.0f + offsetY ,  1.0f + offsetZ ,  0.0f, 1.0f,
		0.0f + offsetX , 0.0f + offsetY ,  1.0f + offsetZ ,  0.0f, 0.0f,

		0.0f + offsetX ,  1.0f + offsetY ,  1.0f + offsetZ ,  0.20f, 0.0f,
		0.0f + offsetX ,  1.0f + offsetY , 0.0f + offsetZ ,  0.20f, 1.0f,
		0.0f + offsetX , 0.0f + offsetY , 0.0f + offsetZ ,  0.0f, 1.0f,
		0.0f + offsetX , 0.0f + offsetY , 0.0f + offsetZ ,  0.0f, 1.0f,
		0.0f + offsetX , 0.0f + offsetY ,  1.0f + offsetZ ,  0.0f, 0.0f,
		0.0f + offsetX ,  1.0f + offsetY ,  1.0f + offsetZ ,  0.20f, 0.0f,

		1.0f + offsetX ,  1.0f + offsetY ,  1.0f + offsetZ ,  0.20f, 0.0f,
		1.0f + offsetX ,  1.0f + offsetY , 0.0f + offsetZ ,  0.20f, 1.0f,
		1.0f + offsetX , 0.0f + offsetY , 0.0f + offsetZ ,  0.0f, 1.0f,
		1.0f + offsetX , 0.0f + offsetY , 0.0f + offsetZ ,  0.0f, 1.0f,
		1.0f + offsetX , 0.0f + offsetY ,  1.0f + offsetZ ,  0.0f, 0.0f,
		1.0f + offsetX ,  1.0f + offsetY ,  1.0f + offsetZ ,  0.20f, 0.0f,

		0.0f + offsetX , 0.0f + offsetY , 0.0f + offsetZ ,  0.0f, 1.0f,
		1.0f + offsetX , 0.0f + offsetY , 0.0f + offsetZ ,  0.20f, 1.0f,
		1.0f + offsetX , 0.0f + offsetY ,  1.0f + offsetZ ,  0.20f, 0.0f,
		1.0f + offsetX , 0.0f + offsetY ,  1.0f + offsetZ ,  0.20f, 0.0f,
		0.0f + offsetX , 0.0f + offsetY ,  1.0f + offsetZ ,  0.0f, 0.0f,
		0.0f + offsetX , 0.0f + offsetY , 0.0f + offsetZ ,  0.0f, 1.0f,

		0.0f + offsetX ,  1.0f + offsetY , 0.0f + offsetZ ,  0.0f, 1.0f,
		1.0f + offsetX ,  1.0f + offsetY , 0.0f + offsetZ ,  0.20f, 1.0f,
		1.0f + offsetX ,  1.0f + offsetY ,  1.0f + offsetZ ,  0.20f, 0.0f,
		1.0f + offsetX ,  1.0f + offsetY ,  1.0f + offsetZ ,  0.20f, 0.0f,
		0.0f + offsetX ,  1.0f + offsetY ,  1.0f + offsetZ ,  0.0f, 0.0f,
		0.0f + offsetX ,  1.0f + offsetY , 0.0f + offsetZ ,  0.0f, 1.0f
	};
	void *buf = pushArray(arena, 30, GLfloat);
	switch (face) {
		case TOP: {
			memcpy(buf, &vertices[150], 30 * sizeof(GLfloat));
			break;
		}
		case BOTTOM: {
			memcpy(buf, &vertices[120], 30 * sizeof(GLfloat));
			break;
		}
		case LEFT: {
			memcpy(buf, &vertices[60], 30 * sizeof(GLfloat));
			break;
		}
		case RIGHT: {
			memcpy(buf, &vertices[90], 30 * sizeof(GLfloat));
			break;
		}
		case FRONT: {
			memcpy(buf, &vertices[30], 30 * sizeof(GLfloat));
			break;
		}
		case BACK: {
			memcpy(buf, &vertices[0], 30 * sizeof(GLfloat));
			break;
		}
	}
}

static void createChunkMesh(world_t *world, chunk_t *chunk, memoryArena_t *renderData) {
	if (chunk == NULL) {
		return;
	}
	/* Left neighbour*/
	chunk_t *nearChunk = getChunk(world, chunk->offsetX - 16, chunk->offsetY, chunk->offsetZ);
	if (nearChunk != NULL) {
		for (int32_t i = 0; i < CHUNK_SIZE; ++i) {
			for (int32_t j = 0; j < CHUNK_SIZE; ++j) {
				if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * j] != 0) {
					if (nearChunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * j + 15] == 0) {
						pushQuad(chunk->offsetX, chunk->offsetY + j, chunk->offsetZ + i, 1, LEFT, renderData);
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
				if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * j + 15] != 0) {
					if (nearChunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * j] == 0) {
						pushQuad(chunk->offsetX + 15, chunk->offsetY + j, chunk->offsetZ + i, 1, RIGHT, renderData);
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
				if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * 15 + j] != 0) {
					if (nearChunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + j] == 0) {
						pushQuad(chunk->offsetX + j, chunk->offsetY + 15, chunk->offsetZ + i, 1, TOP, renderData);
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
				if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + j] != 0) {
					if (nearChunk->blocks[CHUNK_SIZE * CHUNK_SIZE * i + CHUNK_SIZE * 15 + j] == 0) {
						pushQuad(chunk->offsetX + j, chunk->offsetY, chunk->offsetZ + i, 1, BOTTOM, renderData);
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
				if (chunk->blocks[CHUNK_SIZE * j + i] != 0) {
					if (nearChunk->blocks[CHUNK_SIZE * CHUNK_SIZE * 15 + CHUNK_SIZE * j + i] == 0) {
						pushQuad(chunk->offsetX + i, chunk->offsetY + j, chunk->offsetZ, 1, BACK, renderData);
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
				if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * 15 + CHUNK_SIZE * j + i] != 0) {
					if (nearChunk->blocks[CHUNK_SIZE * j + i] == 0) {
						pushQuad(chunk->offsetX + i, chunk->offsetY + j, chunk->offsetZ + 15, 1, FRONT, renderData);
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
							pushQuad(chunk->offsetX + i, chunk->offsetY + j, chunk->offsetZ + k, 1, RIGHT, renderData);
						}
					}
					/* Left neighbour */
					if (i > 0) {
						if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * k + CHUNK_SIZE * j + (i - 1)] == 0) {
							pushQuad(chunk->offsetX + i, chunk->offsetY + j, chunk->offsetZ + k, 1, LEFT, renderData);
						}
					}
					/* Top neighbour */
					if (j < CHUNK_SIZE - 1) {
						if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * k + CHUNK_SIZE * (j + 1) + i] == 0) {
							pushQuad(chunk->offsetX + i, chunk->offsetY + j, chunk->offsetZ + k, 1, TOP, renderData);
						}
					}
					/* Bottom neighbour */
					if (j > 0) {
						if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * k + CHUNK_SIZE * (j - 1) + i] == 0) {
							pushQuad(chunk->offsetX + i, chunk->offsetY + j, chunk->offsetZ + k, 1, BOTTOM, renderData);
						}
					}
					/* Front neighbour */
					if (k < CHUNK_SIZE - 1) {
						if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * (k + 1) + CHUNK_SIZE * j + i] == 0) {
							pushQuad(chunk->offsetX + i, chunk->offsetY + j, chunk->offsetZ + k, 1, FRONT, renderData);
						}
					}
					/* Back neighbour */
					if (k > 0) {
						if (chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * (k - 1) + CHUNK_SIZE * j + i] == 0) {
							pushQuad(chunk->offsetX + i, chunk->offsetY + j, chunk->offsetZ + k, 1, BACK, renderData);
						}
					}
				}
			}
		}
	}
}

static void createChunksSection(int64_t offsetX, int64_t offsetZ, gameState_t *gameState) {
	double frequency = 1.0 / 90.0;
	
	/* Create height map */
	double heightMap[16][16];
	for (int64_t i = CHUNK_SIZE * offsetX, mx = 0; i < CHUNK_SIZE + CHUNK_SIZE * offsetX; ++i, ++mx) {
		for (int64_t j = CHUNK_SIZE * offsetZ, mz = 0; j < CHUNK_SIZE + CHUNK_SIZE * offsetZ; ++j, ++mz) {
			heightMap[mx][mz] = octavePerlin((double)i * frequency, (double)j * frequency, 6) * 48;
		}
	}
	/* Fill chunks */
	chunk_t *chunk = NULL;
	for (uint64_t y = 0; y < WORLD_HEIGHT; ++y) {
		if (y % 16 == 0) {
			chunk = getChunk(gameState->world, CHUNK_SIZE * offsetX, y, CHUNK_SIZE * offsetZ, &gameState->worldArena);
			chunk->offsetX = offsetX * CHUNK_SIZE;
			chunk->offsetY = y;
			chunk->offsetZ = offsetZ * CHUNK_SIZE;
			chunk->blocks = pushArray(&gameState->chunksData, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, uint8_t);
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
static void createWorldMesh(world_t *world, memoryArena_t *renderData) {
	for (int i = -world->radius; i < world->radius + 1; ++i) {
		for (int j = 0; j < 3; ++j) {
			for (int k = -world->radius; k < world->radius + 1; ++k) {
				createChunkMesh(world,
					getChunk(world, (int64_t)CHUNK_SIZE * i, (int64_t)CHUNK_SIZE * j, (int64_t)CHUNK_SIZE * k), renderData);
			}
		}
	}
}

static void initializeArena(memoryArena_t *arena, uint64_t size, uint8_t *base) {
	arena->base = base;
	arena->size = size;
	arena->used = 0;
}

void gameUpdateAndRender(gameInput_t *input, gameMemory_t *memory, renderOutputArea_t *renderOutputArea) {
	if (!memory->isInitialized) {
		gameState = (gameState_t*)memory->permanentStorage;
		initializeArena(&gameState->worldArena, KILOBYTES(30),
			(uint8_t*)memory->permanentStorage + sizeof(gameState_t));
		initializeArena(&gameState->chunksData, MEGABYTES(10),
			(uint8_t*)memory->permanentStorage + sizeof(gameState_t) + gameState->worldArena.size);
		initializeArena(&gameState->renderData, MEGABYTES(370),
			(uint8_t*)memory->permanentStorage + sizeof(gameState_t) + gameState->worldArena.size + gameState->chunksData.size);
		gameState->world = pushStruct(&gameState->worldArena, world_t);
		gameState->world->radius = 2;
		perlinSeed(12041211233);
		int64_t blocksCount = 0;
		for (int i = -gameState->world->radius; i <= gameState->world->radius; ++i) {
			for (int j = -gameState->world->radius; j <= gameState->world->radius; ++j) {
				createChunksSection(i, j, gameState);
			}
		}
		createWorldMesh(gameState->world, &gameState->renderData);
		allocateRenderBuffer(gameState->renderData.base, gameState->renderData.used);
		
		gameState->camera.position = glm::vec3(0.0f, 50.0f, 3.0f);
		gameState->camera.front = glm::vec3(0.0f, 0.0f, -1.0f);
		gameState->camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
		gameState->camera.yaw = -90.0f;

		renderGroup.modelMatrix = glm::mat4(1.0f);
		renderGroup.projectionMatrix = glm::perspective(45.0f, (GLfloat)renderOutputArea->areaWidth / (GLfloat)renderOutputArea->areaHeight, 0.1f, 2500.0f);

		memory->isInitialized = true;
	}
	moveAndRotateCamera(input, &gameState->camera);
	renderGroup.viewMatrix = glm::lookAt(gameState->camera.position, gameState->camera.position + gameState->camera.front, gameState->camera.up);
	renderToOutput(&renderGroup);
}


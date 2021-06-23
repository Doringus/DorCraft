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

/*

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
*/

/*static void createChunksSection(int64_t offsetX, int64_t offsetZ, gameState_t *gameState, bool rewriteChunks, int64_t centerX, int64_t centerZ) {
	double frequency = 1.0 / 90.0;
	
	double heightMap[16][16];
	for (int64_t i = offsetX, mx = 0; i < CHUNK_SIZE + offsetX; ++i, ++mx) {
		for (int64_t j = offsetZ, mz = 0; j < CHUNK_SIZE + offsetZ; ++j, ++mz) {
			heightMap[mx][mz] = octavePerlin((double)i * frequency, (double)j * frequency, 6) * 48;
		}
	}
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
*/



static inline int64_t getChunkCoord(int64_t coord) {
	if (coord > 0) {
		return (coord / CHUNK_SIZE) * CHUNK_SIZE;
	} else {
		return (coord / CHUNK_SIZE) * CHUNK_SIZE - CHUNK_SIZE;
	}
}

void gameUpdateAndRender(gameInput_t *input, gameMemory_t *memory, renderOutputArea_t *renderOutputArea) {
	static int64_t prevChunkX = 0, prevChunkZ = 0;
	if (!memory->isInitialized) {
		gameState = (gameState_t*)memory->permanentStorage;
	/*	initializeArena(&gameState->worldArena, KILOBYTES(30),
			(uint8_t*)memory->permanentStorage + sizeof(gameState_t));
		initializeArena(&gameState->chunksData, MEGABYTES(10),
			(uint8_t*)memory->permanentStorage + sizeof(gameState_t) + gameState->worldArena.size);
		initializeArena(&gameState->renderData, MEGABYTES(500),
			(uint8_t*)memory->permanentStorage + sizeof(gameState_t) + gameState->worldArena.size + gameState->chunksData.size);
		gameState->world = pushStruct(&gameState->worldArena, world_t);
		gameState->world->radius = 1;
		perlinSeed(12041218833);
		int64_t blocksCount = 0;
		*/
		gameState->camera.position = glm::vec3(8.0f, 47.0f, 8.0f);
		gameState->camera.front = glm::vec3(0.0f, 0.0f, -1.0f);
		gameState->camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
		gameState->camera.yaw = -90.0f;

		renderGroup.modelMatrix = glm::mat4(1.0f);
		renderGroup.projectionMatrix = glm::perspective(45.0f, (GLfloat)renderOutputArea->areaWidth / (GLfloat)renderOutputArea->areaHeight, 0.1f, 2500.0f);

		prevChunkX = getChunkCoord(gameState->camera.position.x);
		prevChunkZ = getChunkCoord(gameState->camera.position.z);

		memory->isInitialized = true;

		world_t world;

		chunk_t *c1 = (chunk_t*)malloc(sizeof(chunk_t));
		chunk_t *c2 = (chunk_t*)malloc(sizeof(chunk_t));
		chunk_t *c3 = (chunk_t*)malloc(sizeof(chunk_t));
		c1->offsetX = 3;
		c1->offsetZ = 3;
		c2->offsetX = 34;
		c2->offsetZ = 34;
		c3->offsetX = 65;
		c3->offsetZ = 65;
		insertChunk(&world.hashMap, c1);
		insertChunk(&world.hashMap, c2);
		insertChunk(&world.hashMap, c3);

		removeChunk(&world.hashMap, c3);
		removeChunk(&world.hashMap, c1);
		removeChunk(&world.hashMap, c2);
	}
	moveAndRotateCamera(input, &gameState->camera);
	renderGroup.viewMatrix = glm::lookAt(gameState->camera.position, gameState->camera.position + gameState->camera.front, gameState->camera.up);
	int64_t currentChunkX = getChunkCoord(gameState->camera.position.x);
	int64_t currentChunkZ = getChunkCoord(gameState->camera.position.z);
	if (currentChunkX != prevChunkX || currentChunkZ != prevChunkZ) {
		prevChunkX = currentChunkX;
		prevChunkZ = currentChunkZ;
		//checkDrawableChunks(gameState);
	}
	renderToOutput(&renderGroup);

}


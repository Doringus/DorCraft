#include "dorcraft.h"
#include "dorcraftutils.h"
#include "dorcraftogl.h"
#include "noisegenerator.h"

static renderGroup_t renderGroup;
static gameState_t *gameState;

static void moveAndRotateCamera(gameInput_t *input) {
	/* Move camera */
	if (input->forward.pressed) {
		camera.position += (GLfloat)(3.0f * input->dt) * camera.front;
	}
	if (input->back.pressed) {
		camera.position -= (GLfloat)(3.0f * input->dt) * camera.front;
	}
	if (input->right.pressed) {
		camera.position += glm::normalize(glm::cross(camera.front, camera.up)) * (GLfloat)(3.0f * input->dt);
	}
	if (input->left.pressed) {
		camera.position -= glm::normalize(glm::cross(camera.front, camera.up)) * (GLfloat)(3.0f * input->dt);
	}
	/* Rotate camera */
	double offsetX = input->mousePosition.currentX - input->mousePosition.prevX;
	double offsetY = input->mousePosition.currentY - input->mousePosition.prevY;
	offsetX *= 3.0f * input->dt;
	offsetY *= 3.0f * input->dt;
	camera.yaw += offsetX;
	camera.pitch -= offsetY;

	if (camera.pitch > 89.0f) {
		camera.pitch = 89.0f;
	}
	if (camera.pitch < -89.0f) {
		camera.pitch = -89.0f;
	}
	camera.front.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
	camera.front.y = sin(glm::radians(camera.pitch));
	camera.front.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
	camera.front = glm::normalize(camera.front);
}

#define pushStruct(arena, type) (type*)pushStruct_(arena, sizeof(type))
static void *pushStruct_(memoryArena_t *arena, uint64_t size) {
	assert((arena->used + size) <= arena->size);
	void *result = arena->base + arena->used;
	arena->used += size;
	return result;
}

#define pushArray(arena, size, type) (type*)pushArray_(arena, size , sizeof(type))
static void *pushArray_(memoryArena_t *arena, uint64_t arraySize, uint64_t size) {
	assert((arena->used + size * arraySize) <= arena->size);
	void *result = arena->base + arena->used;
	arena->used += (size * arraySize);
	return result;
}

static void pushCube(int64_t offsetX, int64_t offsetY, int64_t offsetZ, uint8_t type, memoryArena_t *arena) {
	if(type == 0) {
		return;
	}
	GLfloat vertices[] = {
		-0.5f + offsetX , -0.5f + offsetY , -0.5f + offsetZ , 0.0f, 0.0f,
		 0.5f + offsetX , -0.5f + offsetY , -0.5f + offsetZ , 0.20f, 0.0f,
		 0.5f + offsetX ,  0.5f + offsetY , -0.5f + offsetZ , 0.20f, 1.0f,
		 0.5f + offsetX ,  0.5f + offsetY , -0.5f + offsetZ ,  0.20f, 1.0f,
		-0.5f + offsetX ,  0.5f + offsetY , -0.5f + offsetZ ,  0.0f, 1.0f,
		-0.5f + offsetX , -0.5f + offsetY , -0.5f + offsetZ ,  0.0f, 0.0f,

		-0.5f + offsetX , -0.5f + offsetY ,  0.5f + offsetZ ,  0.0f, 0.0f,
		 0.5f + offsetX , -0.5f + offsetY ,  0.5f + offsetZ ,  0.20f, 0.0f,
		 0.5f + offsetX ,  0.5f + offsetY ,  0.5f + offsetZ ,  0.20f, 1.0f,
		 0.5f + offsetX ,  0.5f + offsetY ,  0.5f + offsetZ ,  0.20f, 1.0f,
		-0.5f + offsetX ,  0.5f + offsetY ,  0.5f + offsetZ ,  0.0f, 1.0f,
		-0.5f + offsetX , -0.5f + offsetY ,  0.5f + offsetZ ,  0.0f, 0.0f,

		-0.5f + offsetX ,  0.5f + offsetY ,  0.5f + offsetZ ,  0.20f, 0.0f,
		-0.5f + offsetX ,  0.5f + offsetY , -0.5f + offsetZ ,  0.20f, 1.0f,
		-0.5f + offsetX , -0.5f + offsetY , -0.5f + offsetZ ,  0.0f, 1.0f,
		-0.5f + offsetX , -0.5f + offsetY , -0.5f + offsetZ ,  0.0f, 1.0f,
		-0.5f + offsetX , -0.5f + offsetY ,  0.5f + offsetZ ,  0.0f, 0.0f,
		-0.5f + offsetX ,  0.5f + offsetY ,  0.5f + offsetZ ,  0.20f, 0.0f,

		 0.5f + offsetX ,  0.5f + offsetY ,  0.5f + offsetZ ,  0.20f, 0.0f,
		 0.5f + offsetX ,  0.5f + offsetY , -0.5f + offsetZ ,  0.20f, 1.0f,
		 0.5f + offsetX , -0.5f + offsetY , -0.5f + offsetZ ,  0.0f, 1.0f,
		 0.5f + offsetX , -0.5f + offsetY , -0.5f + offsetZ ,  0.0f, 1.0f,
		 0.5f + offsetX , -0.5f + offsetY ,  0.5f + offsetZ ,  0.0f, 0.0f,
		 0.5f + offsetX ,  0.5f + offsetY ,  0.5f + offsetZ ,  0.20f, 0.0f,

		-0.5f + offsetX , -0.5f + offsetY , -0.5f + offsetZ ,  0.0f, 1.0f,
		 0.5f + offsetX , -0.5f + offsetY , -0.5f + offsetZ ,  0.20f, 1.0f,
		 0.5f + offsetX , -0.5f + offsetY ,  0.5f + offsetZ ,  0.20f, 0.0f,
		 0.5f + offsetX , -0.5f + offsetY ,  0.5f + offsetZ ,  0.20f, 0.0f,
		-0.5f + offsetX , -0.5f + offsetY ,  0.5f + offsetZ ,  0.0f, 0.0f,
		-0.5f + offsetX , -0.5f + offsetY , -0.5f + offsetZ ,  0.0f, 1.0f,

		-0.5f + offsetX ,  0.5f + offsetY , -0.5f + offsetZ ,  0.0f, 1.0f,
		 0.5f + offsetX ,  0.5f + offsetY , -0.5f + offsetZ ,  0.20f, 1.0f,
		 0.5f + offsetX ,  0.5f + offsetY ,  0.5f + offsetZ ,  0.20f, 0.0f,
		 0.5f + offsetX ,  0.5f + offsetY ,  0.5f + offsetZ ,  0.20f, 0.0f,
		-0.5f + offsetX ,  0.5f + offsetY ,  0.5f + offsetZ ,  0.0f, 0.0f,
		-0.5f + offsetX ,  0.5f + offsetY , -0.5f + offsetZ ,  0.0f, 1.0f
	};
	void *buf = pushArray(arena, 180, GLfloat);
	memcpy(buf, &vertices, 180 * sizeof(GLfloat));
}

static void createChunksSection(int64_t offsetX, int64_t offsetZ, memoryArena_t *chunksData, memoryArena_t *renderData) {
	double frequency = 1.0 / 85.0;
	for (int64_t i = CHUNK_SIZE * offsetX, mx = 0; i < CHUNK_SIZE + CHUNK_SIZE * offsetX; ++i) {
		for (int64_t j = CHUNK_SIZE * offsetZ, mz = 0; j < CHUNK_SIZE + CHUNK_SIZE * offsetZ; ++j) {
			double height = octavePerlin((double)i * frequency, (double)j * frequency, 6) * 48;
			chunk_t *chunk; 
			for (uint8_t y = 0; y < height; ++y) {
				if (y % 16 == 0) {
					chunk = pushStruct(chunksData, chunk_t);
				}
				uint8_t blockType;
				if (y > 30) {
					blockType = 0;
				} else {
					blockType = 1;
				}
				chunk->blocks[CHUNK_SIZE * CHUNK_SIZE * mz + CHUNK_SIZE * y + mx] = blockType;
				pushCube(i, y, j, blockType, renderData);
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
		initializeArena(&gameState->chunksData, (memory->permanentStorageSize - sizeof(gameState_t)) / 3,
						(uint8_t*)memory->permanentStorage + sizeof(gameState_t));
		initializeArena(&gameState->renderData, memory->permanentStorageSize - sizeof(gameState_t) - gameState->chunksData.size,
			(uint8_t*)memory->permanentStorage + sizeof(gameState_t) + gameState->chunksData.size);
		perlinSeed(12041211233);
		int64_t blocksCount = 0;
		createChunksSection(0, 0, &gameState->chunksData, &gameState->renderData);
		createChunksSection(1, 0, &gameState->chunksData, &gameState->renderData);
		GLfloat vertices[] = {
		 -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
		};
		allocateRenderBuffer(gameState->renderData.base, gameState->renderData.used);
		
		camera.position = glm::vec3(0.0f, 0.0f, 3.0f);
		camera.front = glm::vec3(0.0f, 0.0f, -1.0f);
		camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
		camera.yaw = -90.0f;

		renderGroup.modelMatrix = glm::mat4(1.0f);
		renderGroup.projectionMatrix = glm::perspective(45.0f, (GLfloat)renderOutputArea->areaWidth / (GLfloat)renderOutputArea->areaHeight, 0.1f, 2500.0f);

		memory->isInitialized = true;
	}
	moveAndRotateCamera(input);
	renderGroup.viewMatrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
	renderToOutput(&renderGroup);
}


#include "dorcraft.h"

#include <inttypes.h>

#include "dorcraftutils.h"
#include "dorcraftogl.h"
#include "noisegenerator.h"
#include "dorcraftworld.h"
#include "chunk.h"


static viewProjectionMatrices_t renderGroup;
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


static inline int64_t getChunkCoord(int64_t coord) {
	if (coord > 0) {
		return (coord / CHUNK_SIZE) * CHUNK_SIZE;
	} else {
		return (coord / CHUNK_SIZE) * CHUNK_SIZE - CHUNK_SIZE;
	}
}

static chunk_t *chunks[2];

static void createWorld(gameState_t *state, uint8_t radius) {
	for (uint8_t i = 0; i < radius * radius; ++i) {
		chunk_t *chunk = &state->chunks[i];
		chunk->blocks = pushArray(&state->chunksData, CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT, uint32_t);
		chunk->renderInfo.vertices = pushArray(&state->verticesData, CHUNK_RENDER_BUFFER_SIZE, GLfloat);
		chunk->renderInfo.indices = pushArray(&state->indicesData, CHUNK_INDICES_BUFFER_SIZE, GLuint);
	}
}

void gameUpdateAndRender(gameInput_t *input, gameMemory_t *memory, renderOutputArea_t *renderOutputArea) {
	static int64_t prevChunkX = 0, prevChunkZ = 0;
	if (!memory->isInitialized) {

		/// Memory layout in permanent storage ///
		/// BP - base pointer for memoryArena_t ///
		///////////////////////////////////////////////////////////////////////////////////////////////////////
		//             /         /         /            /                /                //                 //
		// gameState_t / chunk_t / chunk_t / chunk_t... / BP chunks data / BP render data // BP indices data //
		//             /         /         /            /                /                //                 //
		///////////////////////////////////////////////////////////////////////////////////////////////////////
		
		int worldRadius = 3;
		int64_t chunksDataOffset = sizeof(gameState_t) + sizeof(chunk_t) * worldRadius * worldRadius;
		int64_t chunksDataSize = sizeof(uint32_t) * CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT * worldRadius * worldRadius;

		int64_t renderDataOffset = chunksDataOffset + chunksDataSize;
		int64_t renderDataSize = sizeof(GLfloat) * CHUNK_RENDER_BUFFER_SIZE * worldRadius * worldRadius;

		int64_t indicesDataOffset = renderDataOffset + renderDataSize;
		int64_t indicesDataSize = sizeof(GLuint) * CHUNK_INDICES_BUFFER_SIZE * worldRadius * worldRadius;
		int t = sizeof(gameState_t);
		gameState = (gameState_t*)memory->permanentStorage;
		gameState->chunks = (chunk_t*)((uint8_t*)memory->permanentStorage + sizeof(gameState_t));
		initializeArena(&gameState->chunksData,   chunksDataSize,  (uint8_t*)memory->permanentStorage + chunksDataOffset);
		initializeArena(&gameState->verticesData, renderDataSize,  (uint8_t*)memory->permanentStorage + renderDataOffset);
		initializeArena(&gameState->indicesData,  indicesDataSize, (uint8_t*)memory->permanentStorage + indicesDataOffset);

		memory->isInitialized = true;
		
		perlinSeed(12041218833);
		createWorld(gameState, worldRadius);

		gameState->camera.position = glm::vec3(1.0f, -1.0f, -1.0f);
		gameState->camera.front = glm::vec3(0.0f, 0.0f, 1.0f);
		gameState->camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
		gameState->camera.yaw = -90.0f;

		renderGroup.projectionMatrix = glm::perspective(45.0f, (GLfloat)renderOutputArea->areaWidth / (GLfloat)renderOutputArea->areaHeight, 0.1f, 2500.0f);

		prevChunkX = getChunkCoord(gameState->camera.position.x);
		prevChunkZ = getChunkCoord(gameState->camera.position.z);

		renderGroup.viewMatrix = glm::lookAt(gameState->camera.position, gameState->camera.position + gameState->camera.front, gameState->camera.up);
		chunk_t *c1 = (chunk_t*)malloc(sizeof(chunk_t));
		createChunk(&gameState->chunks[0], 0, 0);

		chunks[0] = &gameState->chunks[0];
	}
	// input
	//update
	moveAndRotateCamera(input, &gameState->camera);
	renderGroup.viewMatrix = glm::lookAt(gameState->camera.position, gameState->camera.position + gameState->camera.front, gameState->camera.up);
	int64_t currentChunkX = getChunkCoord(gameState->camera.position.x);
	int64_t currentChunkZ = getChunkCoord(gameState->camera.position.z);
	if (currentChunkX != prevChunkX || currentChunkZ != prevChunkZ) {
		prevChunkX = currentChunkX;
		prevChunkZ = currentChunkZ;
		//checkDrawableChunks(gameState);
	}
	printf("%f, %f, %f \n", gameState->camera.position.x, gameState->camera.position.y, gameState->camera.position.z);
	// render
	renderChunks(&renderGroup, chunks, 1);
}


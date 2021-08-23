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


static void createWorld(gameState_t *state, uint8_t radius) {
	state->world.radius = radius;
	for (uint8_t i = 0; i < radius; ++i) {
		for (uint8_t j = 0; j < radius; ++j) {
			chunk_t *chunk = &state->chunks[i * radius + j];
			chunk->blocks = pushArray(&state->chunksData, CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT, uint32_t);
			chunk->renderInfo.vertices = pushArray(&state->verticesData, CHUNK_RENDER_BUFFER_SIZE, GLfloat);
			chunk->renderInfo.indices = pushArray(&state->indicesData, CHUNK_INDICES_BUFFER_SIZE, GLuint);
			createChunk(chunk, i * CHUNK_SIZE, j * CHUNK_SIZE);
			fillChunk(&state->world, chunk);
			insertChunk(&gameState->world.hashMap, chunk);
		}
	}
}

void updateWorldMesh(gameState_t *gameState) {
	for (int i = 0; i < 3 * 3; ++i) {
		if (gameState->chunks[i].renderInfo.isDirty) {
			fillRenderBuffer(&gameState->chunks[i].renderInfo);
		}
	}
}

static bool isPointInRect(int64_t rectX, int64_t rectZ, int64_t size, int64_t pointX, int64_t pointZ) {
	return(rectX <= pointX && pointX <= rectX + size && rectZ <= pointZ && pointZ <= rectZ + size);
}

static bool isChunkInWorldRect(int64_t centerX, int64_t centerZ, chunk_t *chunk) {
	int64_t bottomRightFrontX = chunk->x + CHUNK_SIZE;
	int64_t bottomRightFrontZ = chunk->z + CHUNK_SIZE;
	int64_t worldLeftX = centerX - ((3 - 1) / 2) * CHUNK_SIZE;
	int64_t worldLeftZ = centerZ - ((3 - 1) / 2) * CHUNK_SIZE;

	/// Need to check is left point or right point in drawing distance

	return(isPointInRect(worldLeftX, worldLeftZ, 3 * CHUNK_SIZE, chunk->x, chunk->z) && isPointInRect(worldLeftX, worldLeftZ, 3 * CHUNK_SIZE, bottomRightFrontX, bottomRightFrontZ));
}

static void updateDrawableChunks(gameState_t *gameState, int64_t prevX, int64_t prevZ, int64_t currentX, int64_t currentZ) {
	chunk_t *chunksToRewrite[3 * 3];
	int chunksToRewriteSize = 0;
	for (int i = 0; i < 3 * 3; ++i) {
		if (!isChunkInWorldRect(currentX, currentZ, &gameState->chunks[i])) {
			chunksToRewrite[chunksToRewriteSize] = removeChunk(&gameState->world.hashMap, gameState->chunks[i].x, gameState->chunks[i].z);
			chunksToRewriteSize++;
		}
	}
	
	int64_t worldLeftX = currentX - ((3 - 1) / 2) * CHUNK_SIZE;
	int64_t worldLeftZ = currentZ - ((3 - 1) / 2) * CHUNK_SIZE;
	for (int i = 0, chunksToRewriteIndex = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			if (!getChunk(&gameState->world.hashMap, worldLeftX + i * CHUNK_SIZE, worldLeftZ + j * CHUNK_SIZE)) {
				setChunkCoords(chunksToRewrite[chunksToRewriteIndex], worldLeftX + i * CHUNK_SIZE, worldLeftZ + j * CHUNK_SIZE);
				fillChunk(&gameState->world, chunksToRewrite[chunksToRewriteIndex]);
				insertChunk(&gameState->world.hashMap, chunksToRewrite[chunksToRewriteIndex]);
				chunksToRewriteIndex++;
			}
		}
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

		gameState = (gameState_t*)memory->permanentStorage;
		gameState->chunks = (chunk_t*)((uint8_t*)memory->permanentStorage + sizeof(gameState_t));
		initializeArena(&gameState->chunksData,   chunksDataSize,  (uint8_t*)memory->permanentStorage + chunksDataOffset);
		initializeArena(&gameState->verticesData, renderDataSize,  (uint8_t*)memory->permanentStorage + renderDataOffset);
		initializeArena(&gameState->indicesData,  indicesDataSize, (uint8_t*)memory->permanentStorage + indicesDataOffset);

		memory->isInitialized = true;
		
		perlinSeed(12041218833);
		createWorld(gameState, worldRadius);

		gameState->camera.position = glm::vec3(20.0f, 40.0f, 20.0f);
		gameState->camera.front = glm::vec3(0.0f, 0.0f, 1.0f);
		gameState->camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
		gameState->camera.yaw = -90.0f;

		renderGroup.projectionMatrix = glm::perspective(45.0f, (GLfloat)renderOutputArea->areaWidth / (GLfloat)renderOutputArea->areaHeight, 0.1f, 2500.0f);

		prevChunkX = getChunkCoord(gameState->camera.position.x);
		prevChunkZ = getChunkCoord(gameState->camera.position.z);

		renderGroup.viewMatrix = glm::lookAt(gameState->camera.position, gameState->camera.position + gameState->camera.front, gameState->camera.up);
	}
	// input
	//update
	// update camera
	moveAndRotateCamera(input, &gameState->camera);
	renderGroup.viewMatrix = glm::lookAt(gameState->camera.position, gameState->camera.position + gameState->camera.front, gameState->camera.up);
	// check chunks
	int64_t currentChunkX = getChunkCoord(gameState->camera.position.x);
	int64_t currentChunkZ = getChunkCoord(gameState->camera.position.z);
	if (currentChunkX != prevChunkX || currentChunkZ != prevChunkZ) {
		updateDrawableChunks(gameState, prevChunkX, prevChunkZ, currentChunkX, currentChunkZ);
		prevChunkX = currentChunkX;
		prevChunkZ = currentChunkZ;
	}
	updateWorldMesh(gameState);
	// render
	//printf("%f %f %f\n", gameState->camera.position.x, gameState->camera.position.y, gameState->camera.position.z);
	renderChunks(&renderGroup, gameState->chunks, 3 * 3);
}


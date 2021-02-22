#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../../vendor/glm/glm.hpp"
#include "../../vendor/glm/gtc/matrix_transform.hpp"
#include "../../vendor/glm/gtc/type_ptr.hpp"

struct gameButtonState_t {
	bool pressed;
};

struct mousePosition_t {
	double currentX;
	double currentY;
	double prevX;
	double prevY;
};

struct gameInput_t {
	mousePosition_t mousePosition;
	gameButtonState_t forward;
	gameButtonState_t back;
	gameButtonState_t left;
	gameButtonState_t right;
	gameButtonState_t leftMouseButton;
	gameButtonState_t rightMouseButton;
	double dt;
};

struct gameMemory_t {
	bool isInitialized;

	uint64_t permanentStorageSize;
	void *permanentStorage;
};

struct renderOutputArea_t {
	double areaWidth;
	double areaHeight;
};

void gameUpdateAndRender(gameInput_t *input, gameMemory_t *memory, renderOutputArea_t *renderOutputArea);

///// Internal 

#define CHUNK_SIZE 16
#define WORLD_HEIGHT 48

struct world_t;

struct camera_t {
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	double pitch;
	double yaw;
};

struct memoryArena_t {
	uint8_t *base;
	uint64_t size;
	uint64_t used;
};

struct gameState_t {
	memoryArena_t chunksData; // info about blocks
	memoryArena_t renderData; // info about all vertices
	memoryArena_t worldArena; // hash map
	camera_t camera;
	world_t *world;
};

enum quadFace {
	TOP,
	BOTTOM,
	LEFT,
	RIGHT,
	FRONT,
	BACK
};

static void moveAndRotateCamera(gameInput_t *input, camera_t *camera);
static void pushQuad(int64_t offsetX, int64_t offsetY, int64_t offsetZ, uint8_t type, quadFace face, memoryArena_t *renderArena);
static int64_t getChunkRenderBufferOffset();
static void createChunksSection(int64_t offsetX, int64_t offsetZ, gameState_t *gameState, bool rewriteChunks = false, int64_t rewriteX = 0, int64_t rewriteY = 0);
static void createWorldMesh(world_t *world);
static void initializeArena(memoryArena_t *arena, uint64_t size, uint8_t *base);
static void checkDrawableChunks(camera_t *camera, world_t *world);

#define pushStruct(arena, type) (type*)pushStruct_(arena, sizeof(type))
static void *pushStruct_(memoryArena_t *arena, uint64_t size) {
	assert((arena->used + size) <= arena->size);
	void *result = arena->base + arena->used;
	arena->used += size;
	return(result);
}

#define pushArray(arena, size, type) (type*)pushArray_(arena, size , sizeof(type))
static void *pushArray_(memoryArena_t *arena, uint64_t arraySize, uint64_t size) {
	assert((arena->used + size * arraySize) <= arena->size);
	void *result = arena->base + arena->used;
	arena->used += (size * arraySize);
	return(result);
}

static inline void popArena(memoryArena_t *arena) {
	arena->used = 0;
}

static void *reserveMemory(memoryArena_t *sourceArena, uint64_t arenaSize) {
	assert((sourceArena->used + arenaSize) <= sourceArena->size);
	void *result = sourceArena->base + sourceArena->used;
	sourceArena->used += arenaSize;
	return(result);
}
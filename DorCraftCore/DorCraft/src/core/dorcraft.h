#ifndef DORCRAFT_H
#define DORCRAFT_H

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

#include "dorcraftworld.h"

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
	memoryArena_t chunksData;
	memoryArena_t verticesData;
	memoryArena_t indicesData; 
	camera_t camera;
	world_t world;
	chunk_t *chunks;
};


#define pushStruct(arena, type) (type*)pushStruct_(arena, sizeof(type))
static void *pushStruct_(memoryArena_t *arena, uint64_t size) {
	assert((arena->used + size) <= arena->size);
	void *result = arena->base + arena->used;
	arena->used += size;
	return(result);
}

#define pushArray(arena, size, type) (type*)pushArray_(arena, size, sizeof(type))
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

static void initializeArena(memoryArena_t *arena, uint64_t size, uint8_t *base) {
	arena->base = base;
	arena->size = size;
	arena->used = 0;
}

#endif
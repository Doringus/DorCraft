#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

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
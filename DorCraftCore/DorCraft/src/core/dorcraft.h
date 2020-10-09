#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct gameButtonState_t {
	bool pressed;
};

struct gameInput_t {
	gameButtonState_t forward;
	gameButtonState_t back;
	gameButtonState_t left;
	gameButtonState_t right;
};

void gameUpdate(gameInput_t *input);
static void gameRender();
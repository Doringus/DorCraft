#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct gameButtonState_t {
	bool pressed;
};

struct gameInput {
	gameButtonState_t up;
	gameButtonState_t down;
	gameButtonState_t left;
	gameButtonState_t right;
};

static void gameUpdateAndRender(gameInput *input);
static void processGlfwButtonInput(GLFWwindow *window, int key, gameButtonState_t *state);
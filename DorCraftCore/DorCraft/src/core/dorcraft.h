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

void gameLoop(GLFWwindow *window);
static void gameUpdate(gameInput_t *input);
static void gameRender();
static void* readFile(char *filename);
static bool writeFile(char *filename, unsigned int memSize, void *memory);
#ifndef DORCRAFTPLATFORM_H
#define DORCRAFTPLATFORM_H

#include <stdint.h>
#include <gl/glew.h>
#include <GLFW/glfw3.h>

bool startGame(uint16_t windowWidth, uint16_t windowHeight, char *windowTitle);

struct windowInfo_t {
	char *windowTitle;
	uint16_t windowWidth;
	uint16_t windowHeight;
	GLFWwindow *window;
};

static windowInfo_t globalWindowInfo;

#endif
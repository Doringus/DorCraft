#include "dorcraftplatform.h"

#include <string.h>
#include <cstdlib>
#include "dorcraftutils.h"
#include "dorcraft.h"
#include "dorcraftogl.h"

static void processGlfwButtonInput(GLFWwindow* window, gameButtonState_t* newState, int key) {
	newState->pressed = glfwGetKey(window, key);
}

static void processGlfwMouseButtonInput(GLFWwindow* window, gameButtonState_t* newState, int key) {
	newState->pressed = glfwGetMouseButton(window, key);
}

bool startGame(uint16_t windowWidth, uint16_t windowHeight, char *windowTitle) {
	if (!glfwInit()) {
		ERROR_LOG("Cannot initialize glfw lib");
		return(false);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	globalWindowInfo.window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
	globalWindowInfo.windowHeight = windowHeight;
	globalWindowInfo.windowWidth = windowWidth;
	globalWindowInfo.windowTitle = windowTitle;
	if (!globalWindowInfo.window) {
		ERROR_LOG("Cannot create window");
		return(false);
	}
	glfwMakeContextCurrent(globalWindowInfo.window);
	glfwSetInputMode(globalWindowInfo.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	initOpenGl(windowWidth, windowHeight);

	gameInput_t input = {};
	input.mousePosition.currentX = (double)globalWindowInfo.windowWidth / 2.0;
	input.mousePosition.currentY = (double)globalWindowInfo.windowHeight / 2.0;
	glfwSetCursorPos(globalWindowInfo.window, input.mousePosition.currentX, input.mousePosition.currentY);

	gameMemory_t memory = {};
	memory.permanentStorageSize = GIGABYTES(1);
	memory.permanentStorage = VirtualAlloc(0, memory.permanentStorageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	renderOutputArea_t area = {};
	area.areaWidth = windowWidth;
	area.areaHeight = windowHeight;

	double dt = 1.0f / 60.0f;
	double beginTicks = glfwGetTimerValue();

	while (!glfwWindowShouldClose(globalWindowInfo.window)) {
		/* Poll events */
		glfwPollEvents();
		/* Input */
		input.dt = dt;
		processGlfwButtonInput(globalWindowInfo.window, &input.back, GLFW_KEY_S);
		processGlfwButtonInput(globalWindowInfo.window, &input.forward, GLFW_KEY_W);
		processGlfwButtonInput(globalWindowInfo.window, &input.left, GLFW_KEY_A);
		processGlfwButtonInput(globalWindowInfo.window, &input.right, GLFW_KEY_D);
		processGlfwMouseButtonInput(globalWindowInfo.window, &input.leftMouseButton, GLFW_MOUSE_BUTTON_LEFT);
		processGlfwMouseButtonInput(globalWindowInfo.window, &input.rightMouseButton, GLFW_MOUSE_BUTTON_RIGHT);
		/* Get mouse postiton*/
		glfwGetCursorPos(globalWindowInfo.window, &input.mousePosition.currentX, &input.mousePosition.currentY);
		/* Update game */
		gameUpdateAndRender(&input, &memory, &area);
		/* Change mouse prev position */
		input.mousePosition.prevX = input.mousePosition.currentX;
		input.mousePosition.prevY = input.mousePosition.currentY;
		/***********/
		glfwSwapBuffers(globalWindowInfo.window);
		/***********/
		double endTicks = glfwGetTimerValue();
		dt = (endTicks - beginTicks) / (double)glfwGetTimerFrequency();
		
		if (dt > 1.0f) {
			dt = 1.0f / 60.0f;
		}
		beginTicks = endTicks;
	}
	glfwTerminate();
}
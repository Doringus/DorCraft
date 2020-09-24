#include "dorcraft.h"

#include <stdio.h>

static void processGlfwButtonInput(GLFWwindow *window , gameButtonState_t* newState, int key) {
	newState->pressed = glfwGetKey(window, key);
}

void gameLoop(GLFWwindow *window) {
	while (!glfwWindowShouldClose(window)) {
		/* Input */
		gameInput_t input = {};
		processGlfwButtonInput(window, &input.back, GLFW_KEY_S);
		processGlfwButtonInput(window, &input.forward, GLFW_KEY_W);
		processGlfwButtonInput(window, &input.left, GLFW_KEY_A);
		processGlfwButtonInput(window, &input.right, GLFW_KEY_D);

		if (input.forward.pressed) {
			printf("Key `W` was pressed");
		}

		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
}

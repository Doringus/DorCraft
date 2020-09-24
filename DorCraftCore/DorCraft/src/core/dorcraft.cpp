#include "dorcraft.h"

#include <stdio.h>

static void processGlfwButtonInput(GLFWwindow *window , gameButtonState_t* newState, int key) {
	newState->pressed = glfwGetKey(window, key);
}

void gameLoop(GLFWwindow *window) {
	double timePerFrame = 1.0 / 60.0;
	double timer = glfwGetTime();
	double dt = 0.0;
	double lastTime = glfwGetTime();
	short updates = 0, frames = 0;
	while (!glfwWindowShouldClose(window)) {
		/* Input */
		gameInput_t input = {};
		processGlfwButtonInput(window, &input.back, GLFW_KEY_S);
		processGlfwButtonInput(window, &input.forward, GLFW_KEY_W);
		processGlfwButtonInput(window, &input.left, GLFW_KEY_A);
		processGlfwButtonInput(window, &input.right, GLFW_KEY_D);

		if (input.forward.pressed) {
			printf("Key `W` was pressed\n");
		}

		double currentTime = glfwGetTime();
		dt += (currentTime - lastTime) / timePerFrame;
		lastTime = currentTime;
		while (dt >= 1.0) {
			/* Update */
			/**********/
			updates++;
			dt--;
		}

		/* Render */

		/***********/
		frames++;
		if (currentTime - timer > 1.0) {
			timer = currentTime;
			printf("Updates %d, frames %d\n", updates, frames);
			updates = 0;
			frames = 0;
		}

		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
}

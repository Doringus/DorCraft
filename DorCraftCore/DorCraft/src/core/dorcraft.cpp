#include "dorcraft.h"

static void gameUpdateAndRender(gameInput *input) {

}

static void processGlfwButtonInput(GLFWwindow* window, int key, gameButtonState_t* state) {
	state->pressed = glfwGetKey(window, key);
}

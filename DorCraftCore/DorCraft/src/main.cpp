#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "core/dorcraft.h"

int main() {
	GLFWwindow* window;
	if(!glfwInit()) {
		return(-1);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return(-1);
	}
	
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK) {
		printf("Error initializing glew\n");
	}
	gameLoop(window);
	return(0);
}
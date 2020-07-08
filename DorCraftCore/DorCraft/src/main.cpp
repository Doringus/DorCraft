#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

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
		printf("Error initializing glew \n");
	}
	float lastTime = (float)glfwGetTime(), timer = lastTime;
	float deltaTime = 0.0f, currentTime = 0.0f;
	int frames = 0, updates = 0;
	while(!glfwWindowShouldClose(window)) {
		currentTime = (float)glfwGetTime();
		deltaTime += (currentTime - lastTime) / (1.0f / 60.0f);
		lastTime = currentTime;
		while (deltaTime > 1.0f) {
			//update
			++updates;
			--deltaTime;
		}
		//render
		++frames;
		if (glfwGetTime() - timer > 1.0f) {
			++timer;
			printf("FPS %i, updates %i\n", frames, updates);
			updates = 0;
			frames = 0;
		}

		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return(0);
}
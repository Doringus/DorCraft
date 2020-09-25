#include "dorcraft.h"

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static void processGlfwButtonInput(GLFWwindow *window , gameButtonState_t* newState, int key) {
	newState->pressed = glfwGetKey(window, key);
}


void gameLoop(GLFWwindow *window) {
	double timePerFrame = 1.0 / 60.0;
	double timer = glfwGetTime();
	double dt = 0.0;
	double lastTime = glfwGetTime();
	short updates = 0, frames = 0;
	
	char* test = (char*)readFile("src\\shaders\\basic.frag");
	printf("%s\n", test);

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

static void* readFile(char *filename) {
	void *result = 0;
	HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER fileSize;
		if (GetFileSizeEx(fileHandle, &fileSize)) {
			result = VirtualAlloc(0, (SIZE_T)fileSize.QuadPart , MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (result) {
				DWORD bytesRead;
				if (ReadFile(fileHandle, result, fileSize.QuadPart, &bytesRead, 0)
					&& fileSize.QuadPart == bytesRead) {
					// File read successfully
				}
				else {

				}
			}
			else {

			}
		}
		else {

		}
	}
	else {
		
	}
	return(result);
}

static bool writeFile(char* filename, unsigned int memSize, void* memory) {
	bool result = false;
	HANDLE fileHandle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE) {
		DWORD bytesWritten;
		if (WriteFile(fileHandle, memory, memSize, &bytesWritten, 0)) {
			result = (bytesWritten == memSize);
		}
		else {

		}
	}
	return result;
}
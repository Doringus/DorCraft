#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "core/dorcraftogl.h"

int main() {
// Opengl
#if 1
	initGlContext(640, 480, "Dorcraft");
	glGameLoop();
#endif
	return(0);
}
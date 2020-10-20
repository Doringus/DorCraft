//#include "core/dorcraftogl.h"
#include "core/dorcraftplatform.h"

int main() {
// Opengl
#if 0
	initGlContext(1000, 680, "Dorcraft");
	glGameLoop();
#endif
	startGame(1000, 680, "Dorcraft");
	return(0);
}

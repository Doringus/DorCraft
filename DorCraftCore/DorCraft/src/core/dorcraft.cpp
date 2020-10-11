#include "dorcraft.h"
#include "dorcraftutils.h"
#include "dorcraftogl.h"

void gameUpdate(gameInput_t* input) {
	if (input->forward.pressed) {
		printf("Key `W` was pressed\n");
	}
}


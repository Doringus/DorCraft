#include "dorcraft.h"
#include "dorcraftutils.h"
#include "dorcraftogl.h"

struct camera_t {
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	double pitch;
	double yaw;
};

static renderGroup_t renderGroup;
static camera_t camera;

static void moveAndRotateCamera(gameInput_t *input) {
	/* Move camera */
	if (input->forward.pressed) {
		camera.position += (GLfloat)(2.0f * input->dt) * camera.front;
	}
	if (input->back.pressed) {
		camera.position -= (GLfloat)(2.0f * input->dt) * camera.front;
	}
	if (input->right.pressed) {
		camera.position += glm::normalize(glm::cross(camera.front, camera.up)) * (GLfloat)(2.0f * input->dt);
	}
	if (input->left.pressed) {
		camera.position -= glm::normalize(glm::cross(camera.front, camera.up)) * (GLfloat)(2.0f * input->dt);
	}
	/* Rotate camera */
	double offsetX = input->mousePosition.currentX - input->mousePosition.prevX;
	double offsetY = input->mousePosition.currentY - input->mousePosition.prevY;
	offsetX *= 2.0f * input->dt;
	offsetY *= 2.0f * input->dt;
	camera.yaw += offsetX;
	camera.pitch -= offsetY;
	if (camera.pitch > 89.0f) {
		camera.pitch = 89.0f;
	}
	if (camera.pitch < -89.0f) {
		camera.pitch = -89.0f;
	}
	camera.front.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
	camera.front.y = sin(glm::radians(camera.pitch));
	camera.front.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
	camera.front = glm::normalize(camera.front);
}

void gameUpdateAndRender(gameInput_t *input, gameMemory_t *memory, renderOutputArea_t *renderOutputArea) {
	if (!memory->isInitialized) {
		GLfloat vertices[] = {
		-0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f
		};
		allocateRenderBuffer(vertices, 18);

		camera.position = glm::vec3(0.0f, 0.0f, 3.0f);
		camera.front = glm::vec3(0.0f, 0.0f, -1.0f);
		camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
		camera.yaw = -90.0f;

		renderGroup.modelMatrix = glm::mat4(1.0f);
		renderGroup.projectionMatrix = glm::perspective(45.0f, (GLfloat)renderOutputArea->areaWidth / (GLfloat)renderOutputArea->areaHeight, 0.1f, 2500.0f);

		memory->isInitialized = true;
	}
	moveAndRotateCamera(input);
	renderGroup.viewMatrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
	renderToOutput(&renderGroup);
}


#pragma once

#include <glad/glad.h>
#include "rendering/shader.h"
#include <glm/vec3.hpp>
#include "rendering/camera.h"

struct SkyBoxRenderer
{

	void create();

	Shader shader = {};

	GLuint u_invView = 0;
	GLuint u_invProj = 0;
	GLuint u_sunPos = 0;
	GLuint u_underWater = 0;
	GLuint u_waterColor = 0;

	glm::vec3 waterColor = (glm::vec3(24, 85, 217) / 255.f) * 0.6f;

	glm::vec3 sunPos = glm::normalize(glm::vec3(-1, 0.84, -1));

	void render(Camera camera, bool underWater);


};
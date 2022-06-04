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

	void render(Camera camera);


};
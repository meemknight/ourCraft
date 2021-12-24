#pragma once
#include "shader.h"


struct Renderer
{
	Shader defaultShader;
	GLint u_viewProjection = -1;


	void create();

	
};
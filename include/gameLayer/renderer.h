#pragma once
#include "shader.h"


struct Renderer
{
	Shader defaultShader;
	GLint u_viewProjection = -1;
	GLint u_position = -1;
	
	void create();

	GLuint vao = 0;
	GLuint vertexBuffer = 0;
	
};
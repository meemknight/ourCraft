#pragma once
#include "shader.h"

struct Renderer
{
	Shader defaultShader;
	GLint u_viewProjection = -1;
	GLint u_position = -1;
	GLint u_positionInt = -1;
	GLint u_positionFloat = -1;
	GLint u_texture = -1;
	
	void create();

	GLuint vao = 0;
	GLuint vertexBuffer = 0;
	
};

constexpr int mergeShorts(short a, short b)
{
	int rez = 0;
	((short*)&rez)[0] = a;
	((short*)&rez)[1] = b;
	return rez;
}
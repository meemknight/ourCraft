#pragma once
#include "shader.h"

struct Renderer
{
	Shader defaultShader;
	GLint u_viewProjection = -1;
	GLint u_typesCount = -1;
	GLint u_positionInt = -1;
	GLint u_positionFloat = -1;
	GLint u_texture = -1;
	GLuint u_atlasBlockIndex = GL_INVALID_INDEX;

	void create();

	GLuint vao = 0;
	GLuint vertexBuffer = 0;
	GLuint atlasBuffer = 0;
	
};

constexpr int mergeShorts(short a, short b)
{
	int rez = 0;
	((short*)&rez)[0] = a;
	((short*)&rez)[1] = b;
	return rez;
}
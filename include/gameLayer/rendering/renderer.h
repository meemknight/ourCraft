#pragma once
#include "rendering/shader.h"
#include <vector>
#include "rendering/camera.h"
#include "rendering/skyBoxRenderer.h"
#include <gl2d/gl2d.h>

struct Renderer
{
	Shader defaultShader;
	GLint u_viewProjection = -1;
	GLint u_typesCount = -1;
	GLint u_positionInt = -1;
	GLint u_positionFloat = -1;
	GLint u_texture = -1;
	GLint u_time = -1;
	GLuint u_atlasBlockIndex = GL_INVALID_INDEX;
	GLuint u_vertexData = GL_INVALID_INDEX;
	GLuint u_vertexUV = GL_INVALID_INDEX;

	void create();
	void updateDynamicBlocks();
	void render(std::vector<int> &data, Camera &c, gl2d::Texture &texture);

	SkyBoxRenderer skyBoxRenderer;

	GLuint vao = 0;
	GLuint vertexBuffer = 0;
	GLuint atlasBuffer = 0;
	GLuint vertexDataBuffer = 0;
	GLuint vertexUVBuffer = 0;
	
};

struct GyzmosRenderer
{

	struct CubeData
	{
		int x=0, y=0, z=0;
	};

	void create();
	Shader gyzmosCubeShader;
	GLint u_viewProjection = -1;
	GLint u_positionInt = -1;
	GLint u_positionFloat = -1;
	GLuint vao = 0;
	GLuint vertexDataBuffer = 0;
	GLuint blockPositionBuffer = 0;
	GLuint cubeIndices = 0;

	std::vector<CubeData> cubes;

	void drawCube(int x, int y, int z) { cubes.push_back({x, y, z}); };
	void drawCube(glm::ivec3 pos) { drawCube(pos.x, pos.y, pos.z); };
	void render(Camera &c, glm::ivec3 posInt, glm::vec3 posFloat);

};


constexpr int mergeShorts(short a, short b)
{
	int rez = 0;
	((short*)&rez)[0] = a;
	((short*)&rez)[1] = b;
	return rez;
}
#pragma once
#include "rendering/shader.h"
#include <vector>
#include "rendering/camera.h"
#include "rendering/skyBoxRenderer.h"
#include <gl2d/gl2d.h>

struct BlocksLoader;
struct ChunkSystem;
struct ProgramData;

struct Renderer
{

	struct FBO
	{
		void create(bool addColor, bool addDepth);

		void updateSize(int x, int y);

		glm::ivec2 size = {};
		GLuint fbo = 0;

		GLuint color = 0;
		GLuint depth = 0;

		void copyDepthFromMainFBO(int w, int h);

		void clearFBO();
	};

	struct DefaultShader
	{
		Shader shader;
		GLint u_viewProjection = -1;
		GLint u_typesCount = -1;
		GLint u_positionInt = -1;
		GLint u_positionFloat = -1;
		GLint u_texture = -1;
		GLint u_time = -1;
		GLint u_showLightLevels = -1;
		GLint u_skyLightIntensity = -1;
		GLint u_lightsCount = -1;
		GLint u_pointPosF = -1;
		GLint u_pointPosI = -1;
		GLint u_sunDirection = -1;
		GLuint u_vertexData = GL_INVALID_INDEX;
		GLuint u_vertexUV = GL_INVALID_INDEX;
		GLuint u_textureSamplerers = GL_INVALID_INDEX;
		GLuint u_normalsData = GL_INVALID_INDEX;
		GLuint u_lights = GL_INVALID_INDEX;

		GLint u_metallic = -1;
		GLint u_roughness = -1;
		GLint u_exposure = -1;
		GLint u_fogDistance = -1;
		GLint u_underWater = -1;
		GLint u_waterColor = -1;
	}defaultShader;

	struct ZpassShader
	{
		Shader shader;
		GLuint u_viewProjection;
		GLuint u_positionInt;
		GLuint u_positionFloat;
		GLuint u_vertexUV;
		GLuint u_vertexData = GL_INVALID_INDEX;
		GLuint u_textureSamplerers;
	}zpassShader;
	
	float metallic = 0;
	float roughness = 0.5;
	float exposure = 1.7;

	FBO fboCoppy;

	void create(BlocksLoader &blocksLoader);
	void updateDynamicBlocks();
	//void render(std::vector<int> &data, Camera &c, gl2d::Texture &texture);

	void renderFromBakedData(ChunkSystem &chunkSystem, Camera &c,
		ProgramData &programData, bool showLightLevels, int skyLightIntensity, glm::dvec3 pointPos,
		bool underWater, int screenX, int screenY);

	GLuint lightBuffer = 0;
	size_t lightsBufferCount = 0;

	SkyBoxRenderer skyBoxRenderer;

	GLuint vao = 0;
	GLuint vertexBuffer = 0;
	GLuint vertexDataBuffer = 0;
	GLuint vertexUVBuffer = 0;
	GLuint textureSamplerersBuffer = 0;
	
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

struct PointDebugRenderer
{

	void create();
	Shader pointDebugShader;
	GLint u_viewProjection = -1;
	GLint u_positionInt = -1;
	GLint u_positionFloat = -1;
	GLint u_blockPositionInt = -1;
	GLint u_blockPositionFloat = -1;

	void renderPoint(Camera &c, glm::dvec3 point);

	void renderCubePoint(Camera &c, glm::dvec3 point);

};

constexpr int mergeShorts(short a, short b)
{
	int rez = 0;
	((short*)&rez)[0] = a;
	((short*)&rez)[1] = b;
	return rez;
}

constexpr unsigned char merge4bits(unsigned char a, unsigned char b)
{
	unsigned char rez = b & 0b1111;

	a = a & 0b1111;
	a <<= 4;

	rez |= a;

	return rez;
}
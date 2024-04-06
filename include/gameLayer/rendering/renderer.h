#pragma once
#include "rendering/shader.h"
#include <vector>
#include "rendering/camera.h"
#include "rendering/skyBoxRenderer.h"
#include <gl2d/gl2d.h>

struct BlocksLoader;
struct ChunkSystem;
struct ProgramData;
struct SunShadow;
struct ClientEntityManager;

using uniform = GLint;

struct Renderer
{

	struct FBO
	{
		void create(GLint addColor, bool addDepth, GLint addSecondaryRenderTarger = 0);

		void updateSize(int x, int y);

		glm::ivec2 size = {};
		GLuint fbo = 0;
		GLuint fboOnlyFirstTarget = 0;
		GLuint fboOnlySecondTarget = 0;

		GLuint color = 0;
		GLuint secondaryColor = 0;
		GLuint depth = 0;

		GLint colorFormat = 0;
		GLint secondaryColorFormat = 0;

		void copyDepthFromMainFBO(int w, int h);
		void copyColorFromMainFBO(int w, int h);

		void copyDepthFromOtherFBO(GLuint other, int w, int h);
		void copyColorFromOtherFBO(GLuint other, int w, int h);
		void copyDepthAndColorFromOtherFBO(GLuint other, int w, int h);

		void writeAllToOtherFbo(GLuint other, int w, int h);

		void clearFBO();
	};

	struct DefaultShader
	{
		Shader shader;
		uniform u_viewProjection = -1;
		uniform u_typesCount = -1;
		uniform u_positionInt = -1;
		uniform u_positionFloat = -1;
		uniform u_texture = -1;
		uniform u_time = -1;
		uniform u_showLightLevels = -1;
		uniform u_skyLightIntensity = -1;
		uniform u_lightsCount = -1;
		uniform u_pointPosF = -1;
		uniform u_pointPosI = -1;
		uniform u_sunDirection = -1;
		GLuint u_vertexData = GL_INVALID_INDEX;
		GLuint u_vertexUV = GL_INVALID_INDEX;
		GLuint u_textureSamplerers = GL_INVALID_INDEX;
		GLuint u_normalsData = GL_INVALID_INDEX;
		GLuint u_lights = GL_INVALID_INDEX;
		uniform u_timeGrass = -1;
		uniform u_writeScreenSpacePositions = -1;
		uniform u_lastFrameColor = -1;
		uniform u_lastFramePositionViewSpace = -1;
		uniform u_cameraProjection = -1;
		uniform u_inverseView = -1;
		uniform u_view = -1;

		uniform u_metallic = -1;
		uniform u_roughness = -1;
		uniform u_underWater = -1;
		uniform u_waterColor = -1;
		uniform u_depthPeelwaterPass = -1;
		uniform u_hasPeelInformation = -1;
		uniform u_depthTexture = -1;
		uniform u_PeelTexture = -1;
		uniform u_dudv = -1;
		uniform u_dudvNormal = -1;
		uniform u_waterMove = -1;
		uniform u_near = -1;
		uniform u_far = -1;
		uniform u_caustics = -1;
		uniform u_inverseProjMat = -1;
		uniform u_lightSpaceMatrix = -1;
		uniform u_lightPos = -1;
		uniform u_sunShadowTexture = -1;
		uniform u_brdf = -1;
		uniform u_inverseViewProjMat = -1;
		GLuint u_shadingSettings = 0;
		GLuint shadingSettingsBuffer = 0;
		uniform u_lastViewProj = -1;

		struct ShadingSettings
		{
			glm::vec3 waterColor = (glm::vec3(9, 75, 126) / 255.f);
			int tonemapper = 0;
			glm::vec3 underWaterColor = (glm::vec3(0, 17, 25) / 255.f);
			float fogDistance = 10 * 16 / 2;
			float exposure = 1.7;
			float underwaterDarkenStrength = 0.94;
			float underwaterDarkenDistance = 29;
			float fogGradientUnderWater = 1.9;
			int shaders = true;
		}shadingSettings;


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
		GLuint u_renderOnlyWater;
		GLuint u_timeGrass;
	}zpassShader;
	
	struct EntityRenderer
	{

		struct
		{
			Shader shader;

			uniform u_entityPositionInt;
			uniform u_entityPositionFloat;
			uniform u_viewProjection;
			uniform u_cameraPositionInt;
			uniform u_cameraPositionFloat;
			uniform u_modelMatrix;
			uniform u_texture;

			GLuint vaoCube = 0;
			GLuint vertexBufferCube = 0;
			GLuint indexBufferCube = 0;

		}basicEntityshader;

		struct ItemEntityRenderData
		{
			glm::dvec3 position;
		};


		std::vector<ItemEntityRenderData> itemEntitiesToRender;

	}entityRenderer;

	float metallic = 0;
	float roughness = 0.5;

	bool unifiedGeometry = 1;
	bool sortChunks = 1;
	bool zprepass = 0;
	bool frustumCulling = 1;

	FBO fboMain;
	FBO fboCoppy;
	FBO fboLastFrame;
	FBO fboLastFramePositions;

	GLuint drawCommandsOpaqueBuffer;

	void create(BlocksLoader &blocksLoader);
	void reloadShaders();
	void updateDynamicBlocks();
	//void render(std::vector<int> &data, Camera &c, gl2d::Texture &texture);

	void renderFromBakedData(SunShadow &sunShadow, ChunkSystem &chunkSystem, Camera &c,
		ProgramData &programData, BlocksLoader &blocksLoader,
		ClientEntityManager &entityManager, bool showLightLevels, int skyLightIntensity, glm::dvec3 pointPos,
		bool underWater, int screenX, int screenY, float deltaTime);
	
	void renderShadow(SunShadow &sunShadow,
		ChunkSystem &chunkSystem, Camera &c, ProgramData &programData);



	float waterTimer = 0;

	GLuint lightBuffer = 0;
	size_t lightsBufferCount = 0;

	SkyBoxRenderer skyBoxRenderer;
	SkyBoxLoaderAndDrawer skyBoxLoaderAndDrawer;
	SkyBox defaultSkyBox;
	gl2d::Texture sunTexture;
	gl2d::Texture brdfTexture;

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


	struct LinesData
	{
		glm::vec3 a = {};
		glm::vec3 b = {};
	};

	Shader gyzmosLineShader;
	GLint u_gyzmosLineShaderViewProjection = -1;


	void create();
	Shader gyzmosCubeShader;
	GLint u_viewProjection = -1;
	GLint u_positionInt = -1;
	GLint u_positionFloat = -1;
	GLuint vao = 0;
	GLuint vertexDataBuffer = 0;
	GLuint blockPositionBuffer = 0;
	GLuint cubeIndices = 0;

	GLuint vaoLines = 0;
	GLuint vertexDataBufferLines = 0;

	std::vector<CubeData> cubes;
	std::vector<LinesData> lines;

	void drawCube(int x, int y, int z) { cubes.push_back({x, y, z}); };
	void drawCube(glm::ivec3 pos) { drawCube(pos.x, pos.y, pos.z); };

	//todo not working at far distances rn
	void drawLine(glm::dvec3 a, glm::dvec3 b) { lines.push_back({a, b}); }
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

constexpr unsigned short mergeChars(unsigned char a, unsigned char b)
{
	unsigned short rez = b;

	unsigned short secondA = a;
	secondA <<= 8;

	rez |= secondA;

	return rez;
}
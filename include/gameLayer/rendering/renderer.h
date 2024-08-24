#pragma once
#include "rendering/shader.h"
#include <vector>
#include "rendering/camera.h"
#include "rendering/skyBoxRenderer.h"
#include <gl2d/gl2d.h>
#include "blocks.h"

struct BlocksLoader;
struct ChunkSystem;
struct ProgramData;
struct SunShadow;
struct ClientEntityManager;
struct ModelsManager;
struct BoneTransform;

using uniform = GLint;

struct Renderer
{

	struct FBO
	{
		void create(GLint addColor, bool addDepth, GLint addSecondaryRenderTarget = 0,
			GLint addThirdRenderTarget = 0);

		void updateSize(int x, int y);

		glm::ivec2 size = {};
		GLuint fbo = 0;
		GLuint fboOnlyFirstTarget = 0;
		GLuint fboOnlySecondTarget = 0;
		GLuint fboOnlyThirdTarget = 0;

		GLuint color = 0;
		GLuint secondaryColor = 0;
		GLuint thirdColor = 0;
		GLuint depth = 0;

		GLint colorFormat = 0;
		GLint secondaryColorFormat = 0;
		GLint thirdColorFormat = 0;

		void copyDepthFromMainFBO(int w, int h);
		void copyColorFromMainFBO(int w, int h);

		void copyDepthToMainFbo(int w, int h);
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
		uniform u_sunLightColor = -1;
		uniform u_ambientColor = -1;
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
		uniform u_skyTexture = -1;
		uniform u_ao = -1;
		uniform u_inverseViewProjMat = -1;
		GLuint u_shadingSettings = 0;
		GLuint shadingSettingsBuffer = 0;
		uniform u_lastViewProj = -1;

		struct ShadingSettings
		{
			glm::vec3 waterColor = (glm::vec3(6, 27, 43) / 255.f);
			int tonemapper = 0;
			glm::vec3 underWaterColor = (glm::vec3(0, 17, 25) / 255.f);
			float fogDistance = 10 * 16 / 2; //this is controlled by chunk size
			float exposure = 1.6;
			float underwaterDarkenStrength = 0.94;
			float underwaterDarkenDistance = 29;
			float fogGradientUnderWater = 1.9;
			int shaders = true;
			float fogCloseGradient = 0;
			int shadows = 0;
		}shadingSettings;


	}defaultShader;

	struct HBAOShader
	{
		Shader shader;
		uniform u_gPosition;
		uniform u_gNormal;
		uniform u_view;
		uniform u_projection;
	}hbaoShader;

	struct ApplyHBAOShader
	{
		Shader shader;
		uniform u_hbao;
		uniform u_currentViewSpace;
		GLuint u_shadingSettings = 0;

	}applyHBAOShader;

	struct WarpShader
	{
		Shader shader;
		uniform u_color = 0;
		uniform u_time = 0;
		uniform u_currentViewSpace = 0;
		uniform u_underwaterColor = 0;
	}warpShader;

	struct ApplyToneMapper
	{
		Shader shader;
		uniform u_color = 0;
		uniform u_tonemapper = 0;
		uniform u_exposure = 0;

	}applyToneMapper;

	//It is the same as the z pre pass shader
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
			uniform u_view;
			uniform u_lightValue;

			GLuint vaoCube = 0;
			GLuint vertexBufferCube = 0;
			GLuint indexBufferCube = 0;
		}blockEntityshader;

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
			uniform u_view;
			uniform u_lightValue;

		}itemEntityShader;


		struct ItemEntityRenderData
		{
			glm::dvec3 position;
		};

		//todo remove
		std::vector<ItemEntityRenderData> itemEntitiesToRender;


		struct BasicEntityShader
		{
			Shader shader;

			uniform u_viewProjection;
			uniform u_modelMatrix;
			uniform u_cameraPositionInt;
			uniform u_cameraPositionFloat;
			uniform u_view;
			uniform u_bonesPerModel;
			uniform u_exposure;

			GLuint u_skinningMatrix = GL_INVALID_INDEX;
			GLuint u_entityTextureSamplerers = GL_INVALID_INDEX;
			GLuint u_perEntityData = GL_INVALID_INDEX;
				
		}basicEntityShader;
		

	}entityRenderer;

	//for block cracks, it is the same as the z pre pass shader
	struct DecalShader
	{
		Shader shader;
		GLuint u_viewProjection;
		GLuint u_positionInt;
		GLuint u_positionFloat;
		GLuint u_vertexUV;
		GLuint u_vertexData = GL_INVALID_INDEX;
		GLuint u_textureSamplerers;
		GLuint u_renderOnlyWater;
		GLuint u_zBias;
		GLuint u_timeGrass;
		GLuint u_crackPosition;
		
		GLuint vao = 0;
		GLuint geometry = 0;
		GLuint index = 0;



	}decalShader;


	float metallic = 0;
	float roughness = 0.5;

	bool unifiedGeometry = 0; //big gpu buffer
	bool sortChunks = 1;
	bool zprepass = 1;
	bool renderTransparent = 1;
	bool frustumCulling = 1;
	bool ssao = 1;
	int waterRefraction = 1;

	FBO fboHBAO;
	FBO fboMain;
	FBO fboSkyBox;
	FBO fboCoppy;
	FBO fboLastFrame;
	FBO fboLastFramePositions;

	GLuint drawCommandsOpaqueBuffer;

	void recreateBlocksTexturesBuffer(BlocksLoader &blocksLoader);

	void create();
	void reloadShaders();
	//void render(std::vector<int> &data, Camera &c, gl2d::Texture &texture);

	void renderFromBakedData(SunShadow &sunShadow, ChunkSystem &chunkSystem, Camera &c,
		ProgramData &programData, BlocksLoader &blocksLoader,
		ClientEntityManager &entityManager, ModelsManager &modelsManager,
		bool showLightLevels, glm::dvec3 pointPos,
		bool underWater, int screenX, int screenY, float deltaTime, float dayTime,
		GLuint64 currentSkinBindlessTexture, bool &playerClicked, float playerRunning,
		BoneTransform &playerHand);
	
	void renderDecal(glm::ivec3 position, Camera &c, Block b, ProgramData &programData, 
		float crack);

	void renderEntities(float deltaTime,
		Camera &c,
		ModelsManager &modelsManager,
		BlocksLoader &blocksLoader, ClientEntityManager &entityManager,
		glm::mat4 &vp, glm::mat4 &projection, glm::mat4 &viewMatrix, glm::vec3 posFloat, glm::ivec3 posInt,
		float exposure, ChunkSystem &chunkSystem, int skyLightIntensity,
		GLuint64 currentSkinBindlessTexture,
		bool &playerClicked, float playerRunning, BoneTransform &playerHand
		);

	//todo implement this to optimize rendering
	void renderPlayersHand(float deltaTime, ModelsManager &modelsManager,
		BlocksLoader &blocksLoader, ClientEntityManager &entityManager,
		glm::mat4 &vp, glm::mat4 &viewMatrix, glm::vec3 posFloat, glm::ivec3 posInt);

	void renderShadow(SunShadow &sunShadow,
		ChunkSystem &chunkSystem, Camera &c, ProgramData &programData, glm::vec3 sunPos);


	float waterTimer = 0;

	GLuint lightBuffer = 0;
	size_t lightsBufferCount = 0;
	
	glm::vec3 sunPos = glm::normalize(glm::vec3(-1, 0.84, -1));//todo change

	GLuint vao = 0;
	GLuint vertexBuffer = 0;
	GLuint vertexDataBuffer = 0;
	GLuint vertexUVBuffer = 0;
	GLuint textureSamplerersBuffer = 0;
	
	GLuint vaoQuad = 0;
	GLuint vertexBufferQuad = 0;

	GLuint skinningMatrixSSBO = 0;
	GLuint perEntityDataSSBO = 0;
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
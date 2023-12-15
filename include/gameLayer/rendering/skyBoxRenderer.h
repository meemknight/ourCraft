#pragma once

#include <glad/glad.h>
#include "rendering/shader.h"
#include <glm/vec3.hpp>
#include "rendering/camera.h"
#include <gl2d/gl2d.h>

//todo big refactor
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

struct SkyBox
{
	GLuint texture = 0;				//environment cubemap
	GLuint convolutedTexture = 0;	//convoluted environment (used for difuse iradiance)
	GLuint preFilteredMap = 0;		//multiple mipmaps used for speclar 
	glm::vec3 color = {1,1,1};
	void clearTextures();
};

struct SkyBoxLoaderAndDrawer
{
	GLuint vertexArray = 0;
	GLuint vertexBuffer = 0;
	GLuint captureFBO;

	void createGpuData();

	struct
	{
		Shader shader;
		GLuint modelViewUniformLocation;
		GLuint u_sunPos;

	}normalSkyBox;

	struct
	{
		Shader shader;
		GLuint u_equirectangularMap;
		GLuint modelViewUniformLocation;

	}hdrtoCubeMap;

	struct
	{
		Shader shader;
		GLuint u_environmentMap;
		GLuint u_sampleQuality;
		GLuint modelViewUniformLocation;

	}convolute;

	struct
	{
		Shader shader;
		GLuint u_environmentMap;
		GLuint u_roughness;
		GLuint u_sampleCount;
		GLuint modelViewUniformLocation;

	}preFilterSpecular;


	enum CrossskyBoxFormats
	{
		BottomOfTheCrossRight,
		BottomOfTheCrossDown,
		BottomOfTheCrossLeft,
	};

	
	void loadTexture(const char *name, SkyBox &skyBox, int format = 0);
	void loadHDRtexture(const char *name, SkyBox &skyBox, GLuint frameBuffer);
	

	//, GLuint frameBuffer is the default fbo
	void createConvolutedAndPrefilteredTextureData(SkyBox &skyBox,
		float sampleQuality = 0.025, unsigned int specularSamples = 1024);

	void drawBefore(const glm::mat4 &viewProjMat, SkyBox &skyBox, gl2d::Texture &sunTexture,
		glm::vec3 sunPos);


	void clear();
};


/*

"right.jpg",
"left.jpg",
"top.jpg",
"bottom.jpg",
"front.jpg",
"back.jpg"

*/


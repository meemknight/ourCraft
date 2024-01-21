#include "rendering/renderer.h"
#include <ctime>
#include "blocks.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/transform.hpp>
#include <blocksLoader.h>
#include <chunkSystem.h>
#include <gamePlayLogic.h>
#include <iostream>
#include <rendering/sunShadow.h>
#include <platformTools.h>

#define GET_UNIFORM(s, n) n = s.getUniform(#n);
#define GET_UNIFORM2(s, n) s. n = s.shader.getUniform(#n);


//data format:

// short orientation
// short texture index
// int x
// int y
// int z

float vertexData[] = {
	//front
	0.5, 0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, 0.5, 0.5,

	//back
	-0.5, -0.5, -0.5,
	-0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, -0.5, -0.5,
	-0.5, -0.5, -0.5,

	//top
	-0.5, 0.5, -0.5,
	-0.5, 0.5, 0.5,
	0.5, 0.5, 0.5,
	0.5, 0.5, 0.5,
	0.5, 0.5, -0.5,
	-0.5, 0.5, -0.5,

	//bottom
	0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, -0.5,
	-0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,
	0.5, -0.5, 0.5,

	//left
	-0.5, -0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, 0.5, -0.5,
	-0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,
	-0.5, -0.5, 0.5,

	//right
	0.5, 0.5, -0.5,
	0.5, 0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, -0.5, -0.5,
	0.5, 0.5, -0.5,

	//grass
	0.5, 0.5, 0.5,
	-0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,
	-0.5, -0.5, -0.5,
	0.5, -0.5, 0.5,
	0.5, 0.5, 0.5,

	-0.5, -0.5, -0.5,
	-0.5, 0.5, -0.5,
	0.5, 0.5, 0.5,
	0.5, 0.5, 0.5,
	0.5, -0.5, 0.5,
	-0.5, -0.5, -0.5,

	-0.5, 0.5, 0.5,
	0.5, 0.5, -0.5,
	0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,
	-0.5, -0.5, 0.5,
	-0.5, 0.5, 0.5,

	-0.5, -0.5, 0.5,
	0.5, -0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	-0.5, 0.5, 0.5,
	-0.5, -0.5, 0.5,
	

	//moving leaves
	//front
	0.5, 0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, 0.5, 0.5,

	//back
	-0.5, -0.5, -0.5,
	-0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, -0.5, -0.5,
	-0.5, -0.5, -0.5,

	//top
	-0.5, 0.5, -0.5,
	-0.5, 0.5, 0.5,
	0.5, 0.5, 0.5,
	0.5, 0.5, 0.5,
	0.5, 0.5, -0.5,
	-0.5, 0.5, -0.5,

	//bottom
	0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, -0.5,
	-0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,
	0.5, -0.5, 0.5,

	//left
	-0.5, -0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, 0.5, -0.5,
	-0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,
	-0.5, -0.5, 0.5,

	//right
	0.5, 0.5, -0.5,
	0.5, 0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, -0.5, -0.5,
	0.5, 0.5, -0.5,
};

float vertexUV[] = {
	//front
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,

	//back
	0, 0,
	0, 1,
	1, 1,
	1, 1,
	1, 0,
	0, 0,

	//bottom
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,

	//top
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,

	//left
	1, 0,
	1, 1,
	0, 1,
	0, 1,
	0, 0,
	1, 0,

	//right
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,


	//grass
	//front
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,

	0, 0,
	0, 1,
	1, 1,
	1, 1,
	1, 0,
	0, 0,

	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,

	1, 0,
	0, 0,
	0, 1,
	0, 1,
	1, 1,
	1, 0,


	//leaves////////////////////////
	//front
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,

	//back
	0, 0,
	0, 1,
	1, 1,
	1, 1,
	1, 0,
	0, 0,

	//bottom
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,

	//top
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,

	//left
	1, 0,
	1, 1,
	0, 1,
	0, 1,
	0, 0,
	1, 0,

	//right
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1
};


void Renderer::create(BlocksLoader &blocksLoader)
{

	fboCoppy.create(true, true);
	fboMain.create(true, true, GL_RGB16F);
	fboLastFrame.create(true, false);
	fboLastFramePositions.create(GL_RGB16F, false);

	skyBoxRenderer.create();
	skyBoxLoaderAndDrawer.createGpuData();
	//skyBoxLoaderAndDrawer.loadTexture(RESOURCES_PATH "sky/skybox.png", defaultSkyBox);
	skyBoxLoaderAndDrawer.loadTexture(RESOURCES_PATH "sky/nightsky.png", defaultSkyBox);
	//skyBoxLoaderAndDrawer.loadTexture(RESOURCES_PATH "sky/twilightsky.png", defaultSkyBox);
	sunTexture.loadFromFile(RESOURCES_PATH "sky/sun.png", false, false);

	defaultShader.shader.loadShaderProgramFromFile(RESOURCES_PATH "defaultShader.vert", RESOURCES_PATH "defaultShader.frag");
	defaultShader.shader.bind();

	GET_UNIFORM2(defaultShader, u_viewProjection);
	GET_UNIFORM2(defaultShader, u_typesCount);
	GET_UNIFORM2(defaultShader, u_positionInt);
	GET_UNIFORM2(defaultShader, u_positionFloat);
	GET_UNIFORM2(defaultShader, u_texture);
	GET_UNIFORM2(defaultShader, u_time);
	GET_UNIFORM2(defaultShader, u_showLightLevels);
	GET_UNIFORM2(defaultShader, u_skyLightIntensity);
	GET_UNIFORM2(defaultShader, u_lightsCount);
	GET_UNIFORM2(defaultShader, u_pointPosF);
	GET_UNIFORM2(defaultShader, u_pointPosI);
	GET_UNIFORM2(defaultShader, u_sunDirection);
	GET_UNIFORM2(defaultShader, u_metallic);
	GET_UNIFORM2(defaultShader, u_roughness);
	GET_UNIFORM2(defaultShader, u_exposure);
	GET_UNIFORM2(defaultShader, u_fogDistance);
	GET_UNIFORM2(defaultShader, u_underWater);
	GET_UNIFORM2(defaultShader, u_waterColor);
	GET_UNIFORM2(defaultShader, u_depthPeelwaterPass);
	GET_UNIFORM2(defaultShader, u_depthTexture);
	GET_UNIFORM2(defaultShader, u_hasPeelInformation);
	GET_UNIFORM2(defaultShader, u_PeelTexture);
	GET_UNIFORM2(defaultShader, u_dudv);
	GET_UNIFORM2(defaultShader, u_dudvNormal);
	GET_UNIFORM2(defaultShader, u_waterMove);
	GET_UNIFORM2(defaultShader, u_near);
	GET_UNIFORM2(defaultShader, u_far);
	GET_UNIFORM2(defaultShader, u_caustics);
	GET_UNIFORM2(defaultShader, u_inverseProjMat);
	GET_UNIFORM2(defaultShader, u_lightSpaceMatrix);
	GET_UNIFORM2(defaultShader, u_lightPos);
	GET_UNIFORM2(defaultShader, u_sunShadowTexture);
	GET_UNIFORM2(defaultShader, u_timeGrass);
	GET_UNIFORM2(defaultShader, u_writeScreenSpacePositions);
	GET_UNIFORM2(defaultShader, u_lastFrameColor);
	GET_UNIFORM2(defaultShader, u_lastFramePositionViewSpace);
	GET_UNIFORM2(defaultShader, u_cameraProjection);
	GET_UNIFORM2(defaultShader, u_inverseView);
	GET_UNIFORM2(defaultShader, u_view);
	GET_UNIFORM2(defaultShader, u_tonemapper);
	
	
	defaultShader.u_vertexData = getStorageBlockIndex(defaultShader.shader.id, "u_vertexData");
	glShaderStorageBlockBinding(defaultShader.shader.id, defaultShader.u_vertexData, 1);
	glGenBuffers(1, &vertexDataBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexDataBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(vertexData), vertexData, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vertexDataBuffer);

	defaultShader.u_vertexUV = getStorageBlockIndex(defaultShader.shader.id, "u_vertexUV");
	glShaderStorageBlockBinding(defaultShader.shader.id, defaultShader.u_vertexUV, 2);
	glGenBuffers(1, &vertexUVBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexUVBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(vertexUV), vertexUV, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vertexUVBuffer);

	defaultShader.u_textureSamplerers = getStorageBlockIndex(defaultShader.shader.id, "u_textureSamplerers");
	glShaderStorageBlockBinding(defaultShader.shader.id, defaultShader.u_textureSamplerers, 3);
	glGenBuffers(1, &textureSamplerersBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureSamplerersBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint64) * blocksLoader.gpuIds.size(), blocksLoader.gpuIds.data(), 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, textureSamplerersBuffer);
	

	
	glGenBuffers(1, &lightBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	defaultShader.u_lights = getStorageBlockIndex(defaultShader.shader.id, "u_lights");
	glShaderStorageBlockBinding(defaultShader.shader.id, defaultShader.u_lights, 4);


	
#pragma region zpass
	{
		zpassShader.shader.loadShaderProgramFromFile(RESOURCES_PATH "zpass.vert", RESOURCES_PATH "zpass.frag");
		zpassShader.shader.bind();

		GET_UNIFORM2(zpassShader, u_viewProjection);
		GET_UNIFORM2(zpassShader, u_positionInt);
		GET_UNIFORM2(zpassShader, u_positionFloat);
		GET_UNIFORM2(zpassShader, u_renderOnlyWater);
		GET_UNIFORM2(zpassShader, u_timeGrass);

		zpassShader.u_vertexData = getStorageBlockIndex(zpassShader.shader.id, "u_vertexData");
		glShaderStorageBlockBinding(zpassShader.shader.id, zpassShader.u_vertexData, 1);

		zpassShader.u_vertexUV = getStorageBlockIndex(zpassShader.shader.id, "u_vertexUV");
		glShaderStorageBlockBinding(zpassShader.shader.id, zpassShader.u_vertexUV, 2);
		
		zpassShader.u_textureSamplerers = getStorageBlockIndex(zpassShader.shader.id, "u_textureSamplerers");
		glShaderStorageBlockBinding(zpassShader.shader.id, zpassShader.u_textureSamplerers, 3);

	}
#pragma endregion

	//todo remove?
	glCreateBuffers(1, &vertexBuffer);
	//glNamedBufferData(vertexBuffer, sizeof(data), data, GL_DYNAMIC_DRAW);

	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
		
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

		glEnableVertexAttribArray(0);
		glVertexAttribIPointer(0, 1, GL_SHORT, 5 * sizeof(int), 0);
		glVertexAttribDivisor(0, 1);

		glEnableVertexAttribArray(1);
		glVertexAttribIPointer(1, 1, GL_SHORT, 5 * sizeof(int), (void *)(1 * sizeof(short)));
		glVertexAttribDivisor(1, 1);

		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 3, GL_INT, 5 * sizeof(int), (void *)(1 * sizeof(int)));
		glVertexAttribDivisor(2, 1);
		
		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 1, GL_INT, 5 * sizeof(int), (void *)(4 * sizeof(int)));
		glVertexAttribDivisor(3, 1);

		glEnableVertexAttribArray(4);
		glVertexAttribIPointer(4, 1, GL_INT, 6 * sizeof(int), (void *)(5 * sizeof(int)));
		glVertexAttribDivisor(4, 1);

	glBindVertexArray(0);

}

void Renderer::updateDynamicBlocks()
{
	float time = std::clock();

	glm::vec3 topFrontLeftGrass = {-0.5f, 0.5f, 0.5f};
	glm::vec3 topFrontRightGrass = {0.5f, 0.5f, 0.5f};
	glm::vec3 topBackLeftGrass = {-0.5f, 0.5f, -0.5f};
	glm::vec3 topBackRightGrass = {0.5f, 0.5f, -0.5f};
	glm::vec3 bottomFrontLeftGrass = {-0.5f, -0.5f, 0.5f};
	glm::vec3 bottomFrontRightGrass = {0.5f, -0.5f, 0.5f};
	glm::vec3 bottomBackLeftGrass = {-0.5f, -0.5f, -0.5f};
	glm::vec3 bottomBackRightGrass = {0.5f, -0.5f, -0.5f};

	glm::vec3 topFrontLeft = {};
	glm::vec3 topFrontRight = {};
	glm::vec3 topBackLeft = {};
	glm::vec3 topBackRight = {};
	glm::vec3 bottomFrontLeft = {};
	glm::vec3 bottomFrontRight = {};
	glm::vec3 bottomBackLeft = {};
	glm::vec3 bottomBackRight = {};

	glm::vec3 *topFaces[4] = {&topFrontLeft, &topFrontRight, &topBackLeft, &topBackRight};
	glm::vec3* bottomFaces[4] = {&bottomFrontLeft, &bottomFrontRight, &bottomBackLeft, &bottomBackRight};

	glm::vec3 *topFacesGrass[4] = {&topFrontLeftGrass, &topFrontRightGrass, &topBackLeftGrass, &topBackRightGrass};
	glm::vec3 *bottomFacesGrass[4] = {&bottomFrontLeftGrass, &bottomFrontRightGrass, &bottomBackLeftGrass, &bottomBackRightGrass};
	
	float prelucratedTime = time / 100000.f;

	prelucratedTime = std::sin(prelucratedTime) * 3.14159 * 10;

	float s = std::sin(prelucratedTime);
	float c = std::cos(prelucratedTime);
	
	glm::vec2 offsetVector = {1, 0};
	offsetVector = {c * offsetVector.x - s * offsetVector.y, s * offsetVector.x + c * offsetVector.y};
	offsetVector = glm::normalize(offsetVector) * 0.06f * std::abs(cos(prelucratedTime * 2.f));
	//offsetVector = glm::normalize(offsetVector) * 0.07f * sin(prelucratedTime * 20.f);
	
	for (int i = 0; i < 4; i++)
	{
		topFaces[i]->x += offsetVector.x;
		topFaces[i]->z += offsetVector.y;
	
		bottomFaces[i]->x -= offsetVector.x;
		bottomFaces[i]->z -= offsetVector.y;


		//topFacesGrass[i]->x += offsetVector.x;
		//topFacesGrass[i]->z += offsetVector.y;

		//bottomFacesGrass[i]->x -= offsetVector.x;
		//bottomFacesGrass[i]->z -= offsetVector.y;
	}
	//sin((worldPos.x * DIRECTIONX + worldPos.z * DIRECTIONZ - time * SPEED) * FREQUENCY) * AMPLITUDE

	float newData[] =
	{
		//grass
		topFrontRightGrass.x, topFrontRightGrass.y, topFrontRightGrass.z,
		topBackLeftGrass.x,topBackLeftGrass.y,topBackLeftGrass.z,
		-0.5, -0.5, -0.5,
		-0.5, -0.5, -0.5,
		0.5, -0.5, 0.5,
		topFrontRightGrass.x, topFrontRightGrass.y, topFrontRightGrass.z,

		-0.5, -0.5, -0.5,
		topBackLeftGrass.x,topBackLeftGrass.y,topBackLeftGrass.z,
		topFrontRightGrass.x, topFrontRightGrass.y, topFrontRightGrass.z,
		topFrontRightGrass.x, topFrontRightGrass.y, topFrontRightGrass.z,
		0.5, -0.5, 0.5,
		-0.5, -0.5, -0.5,

		topFrontLeftGrass.x,topFrontLeftGrass.y,topFrontLeftGrass.z,
		topBackRightGrass.x,topBackRightGrass.y,topBackRightGrass.z,
		0.5, -0.5, -0.5,
		0.5, -0.5, -0.5,
		-0.5, -0.5, 0.5,
		topFrontLeftGrass.x,topFrontLeftGrass.y,topFrontLeftGrass.z,

		
		-0.5, -0.5, 0.5,
		0.5, -0.5, -0.5,
		topBackRightGrass.x,topBackRightGrass.y,topBackRightGrass.z,
		topBackRightGrass.x,topBackRightGrass.y,topBackRightGrass.z,
		topFrontLeftGrass.x,topFrontLeftGrass.y,topFrontLeftGrass.z,
		-0.5, -0.5, 0.5,

		//-0.5, -0.5, 0.5,
		//0.5, -0.5, -0.5,
		//0.5, 0.5, -0.5,
		//0.5, 0.5, -0.5,
		//-0.5, 0.5, 0.5,
		//-0.5, -0.5, 0.5,


		//leaves
		//front
		topFrontRight.x, topFrontRight.y, topFrontRight.z,
		topFrontLeft.x, topFrontLeft.y, topFrontLeft.z,
		bottomFrontLeft.x, bottomFrontLeft.y, bottomFrontLeft.z,
		bottomFrontLeft.x, bottomFrontLeft.y, bottomFrontLeft.z,
		bottomFrontRight.x, bottomFrontRight.y, bottomFrontRight.z,
		topFrontRight.x, topFrontRight.y, topFrontRight.z,

		//back
		bottomBackLeft.x, bottomBackLeft.y, bottomBackLeft.z,
		topBackLeft.x, topBackLeft.y, topBackLeft.z,
		topBackRight.x, topBackRight.y, topBackRight.z,
		topBackRight.x, topBackRight.y, topBackRight.z,
		bottomBackRight.x, bottomBackRight.y, bottomBackRight.z,
		bottomBackLeft.x, bottomBackLeft.y, bottomBackLeft.z,

		//top
		topBackLeft.x, topBackLeft.y, topBackLeft.z,
		topFrontLeft.x, topFrontLeft.y, topFrontLeft.z,
		topFrontRight.x, topFrontRight.y, topFrontRight.z,
		topFrontRight.x, topFrontRight.y, topFrontRight.z,
		topBackRight.x, topBackRight.y, topBackRight.z,
		topBackLeft.x, topBackLeft.y, topBackLeft.z,

		//bottom
		bottomFrontRight.x, bottomFrontRight.y, bottomFrontRight.z,
		bottomFrontLeft.x, bottomFrontLeft.y, bottomFrontLeft.z,
		bottomBackLeft.x, bottomBackLeft.y, bottomBackLeft.z,
		bottomBackLeft.x, bottomBackLeft.y, bottomBackLeft.z,
		bottomBackRight.x, bottomBackRight.y, bottomBackRight.z,
		bottomFrontRight.x, bottomFrontRight.y, bottomFrontRight.z,

		//left
		bottomFrontLeft.x, bottomFrontLeft.y, bottomFrontLeft.z,
		topFrontLeft.x, topFrontLeft.y, topFrontLeft.z,
		topBackLeft.x, topBackLeft.y, topBackLeft.z,
		topBackLeft.x, topBackLeft.y, topBackLeft.z,
		bottomBackLeft.x, bottomBackLeft.y, bottomBackLeft.z,
		bottomFrontLeft.x, bottomFrontLeft.y, bottomFrontLeft.z,

		//right
		topBackRight.x, topBackRight.y, topBackRight.z,
		topFrontRight.x, topFrontRight.y, topFrontRight.z,
		bottomFrontRight.x, bottomFrontRight.y, bottomFrontRight.z,
		bottomFrontRight.x, bottomFrontRight.y, bottomFrontRight.z,
		bottomBackRight.x, bottomBackRight.y, bottomBackRight.z,
		topBackRight.x, topBackRight.y, topBackRight.z,

	};

	glNamedBufferSubData(vertexDataBuffer, sizeof(vertexData) - sizeof(newData), sizeof(newData), newData);


}

void Renderer::renderFromBakedData(SunShadow &sunShadow, ChunkSystem &chunkSystem, Camera &c, ProgramData &programData
	, bool showLightLevels, int skyLightIntensity, glm::dvec3 pointPos, bool underWater, int screenX, int screenY, 
	float deltaTime)
{
	glViewport(0, 0, screenX, screenY);
	
	fboCoppy.updateSize(screenX, screenY);
	fboCoppy.clearFBO();

	fboMain.updateSize(screenX, screenY);
	fboMain.clearFBO();

	fboLastFrame.updateSize(screenX, screenY);
	fboLastFramePositions.updateSize(screenX, screenY);

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = {};
	glm::ivec3 blockPosition = from3DPointToBlock(c.position);
	c.decomposePosition(posFloat, posInt);

	auto vp = c.getProjectionMatrix() * glm::lookAt({0,0,0}, c.viewDirection, c.up);
	auto chunkVectorCopy = chunkSystem.loadedChunks;

	float timeGrass = std::clock() / 1000.f;


#pragma region render sky box 0

	glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fboOnlyFirstTarget);

	programData.renderer.skyBoxLoaderAndDrawer.drawBefore(c.getProjectionMatrix() * c.getViewMatrix(),
		programData.renderer.defaultSkyBox, programData.renderer.sunTexture,
		programData.renderer.skyBoxRenderer.sunPos);

#pragma endregion


#pragma region setup uniforms and stuff
	{
		zpassShader.shader.bind();
		glUniformMatrix4fv(zpassShader.u_viewProjection, 1, GL_FALSE, &vp[0][0]);
		glUniform3fv(zpassShader.u_positionFloat, 1, &posFloat[0]);
		glUniform3iv(zpassShader.u_positionInt, 1, &posInt[0]);
		glUniform1i(zpassShader.u_renderOnlyWater, 0);
		glUniform1f(zpassShader.u_timeGrass, timeGrass);


		programData.texture.bind(0);
		programData.numbersTexture.bind(1);

		defaultShader.shader.bind();
		glUniformMatrix4fv(defaultShader.u_viewProjection, 1, GL_FALSE, &vp[0][0]);
		glUniform3fv(defaultShader.u_positionFloat, 1, &posFloat[0]);
		glUniform3iv(defaultShader.u_positionInt, 1, &posInt[0]);
		glUniform1i(defaultShader.u_typesCount, BlocksCount);	//remove
		glUniform1f(defaultShader.u_time, std::clock() / 400.f);
		glUniform1i(defaultShader.u_showLightLevels, showLightLevels);
		glUniform1i(defaultShader.u_skyLightIntensity, skyLightIntensity);
		glUniform3fv(defaultShader.u_sunDirection, 1, &skyBoxRenderer.sunPos[0]);
		glUniform1f(defaultShader.u_metallic, metallic);
		glUniform1f(defaultShader.u_roughness, roughness);
		glUniform1f(defaultShader.u_exposure, exposure);
		glUniform1f(defaultShader.u_fogDistance, (chunkSystem.squareSize / 2.f) * CHUNK_SIZE - CHUNK_SIZE);
		glUniform1i(defaultShader.u_underWater, underWater);
		glUniform3fv(defaultShader.u_waterColor, 1, &skyBoxRenderer.waterColor[0]);
		glUniform1i(defaultShader.u_depthPeelwaterPass, 0);
		glUniform1i(defaultShader.u_hasPeelInformation, 0);
		glUniform1f(defaultShader.u_waterMove, waterTimer);
		glUniform1f(defaultShader.u_near, c.closePlane);
		glUniform1f(defaultShader.u_far, c.farPlane);
		glUniformMatrix4fv(defaultShader.u_inverseProjMat, 1, 0,
		&glm::inverse(c.getProjectionMatrix())[0][0]);
		glUniformMatrix4fv(defaultShader.u_lightSpaceMatrix, 1, 0,
			&sunShadow.lightSpaceMatrix[0][0]);
		glUniform3iv(defaultShader.u_lightPos, 1, &sunShadow.lightSpacePosition[0]);
		glUniform1i(defaultShader.u_writeScreenSpacePositions, 1);//todo remove

		programData.dudv.bind(4);
		glUniform1i(defaultShader.u_dudv, 4);

		programData.dudvNormal.bind(5);
		glUniform1i(defaultShader.u_dudvNormal, 5);

		programData.causticsTexture.bind(6);
		glUniform1i(defaultShader.u_caustics, 6);

		glActiveTexture(GL_TEXTURE0 + 7);
		glBindTexture(GL_TEXTURE_2D, sunShadow.shadowMap.depth);
		glUniform1i(defaultShader.u_sunShadowTexture, 7);
		glUniform1f(defaultShader.u_timeGrass, timeGrass);

		glActiveTexture(GL_TEXTURE0 + 8);
		glBindTexture(GL_TEXTURE_2D, fboLastFrame.color);
		glUniform1i(defaultShader.u_lastFrameColor, 8);

		glActiveTexture(GL_TEXTURE0 + 9);
		glBindTexture(GL_TEXTURE_2D, fboLastFramePositions.color);
		glUniform1i(defaultShader.u_lastFramePositionViewSpace, 9);

		glUniformMatrix4fv(defaultShader.u_cameraProjection, 1, GL_FALSE, glm::value_ptr(c.getProjectionMatrix()));

		glUniformMatrix4fv(defaultShader.u_inverseView, 1, GL_FALSE, 
			glm::value_ptr( glm::inverse(c.getViewMatrix())) );

		glUniformMatrix4fv(defaultShader.u_view, 1, GL_FALSE,
			glm::value_ptr(c.getViewMatrix()));

		glUniform1i(defaultShader.u_tonemapper, tonemapper);
		

		waterTimer += deltaTime * 0.09;
		if (waterTimer > 20)
		{
			waterTimer -= 20;
		}

		//waterTimer += deltaTime * 0.4;
		//if (waterTimer > 1)
		//{
		//	waterTimer -= 1;
		//}
		

		{
			glm::ivec3 i;
			glm::vec3 f;
			decomposePosition(pointPos, f, i);

			glUniform3fv(defaultShader.u_pointPosF, 1, &f[0]);
			glUniform3iv(defaultShader.u_pointPosI, 1, &i[0]);
		}

	#pragma region lights
		{

			if (chunkSystem.shouldUpdateLights)
			{
				chunkSystem.shouldUpdateLights = 0;

				//std::cout << "Updated lights\n";

				lightsBufferCount = 0;

				for (auto c : chunkSystem.loadedChunks)
				{
					if (c)
					{
						lightsBufferCount += c->lightsElementCountSize;
					}
				}

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
				glBufferData(GL_SHADER_STORAGE_BUFFER, lightsBufferCount * sizeof(glm::ivec4), NULL, GL_STREAM_COPY);

				glBindBuffer(GL_COPY_WRITE_BUFFER, lightBuffer);

				size_t offset = 0;
				for (auto c : chunkSystem.loadedChunks)
				{

					if (c)
					{
						glBindBuffer(GL_COPY_READ_BUFFER, c->lightsBuffer);

						// Copy data from the first existing SSBO to the new SSBO
						glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset,
							c->lightsElementCountSize * sizeof(glm::ivec4));

						offset += c->lightsElementCountSize * sizeof(glm::ivec4);
					}
				}

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, lightBuffer);
			};

			glUniform1i(defaultShader.u_lightsCount, lightsBufferCount);

			//auto c = chunkSystem.getChunkSafeFromBlockPos(posInt.x, posInt.z);
			//
			//if (c)
			//{
			//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, c->lightsBuffer);
			//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, c->lightsBuffer);
			//	glUniform1i(u_lightsCount, c->lightsElementCountSize);
			//}
		}
	#pragma endregion

		//sort chunks
		{
			std::sort(chunkVectorCopy.begin(), chunkVectorCopy.end(),
				[x = divideChunk(blockPosition.x), z = divideChunk(blockPosition.z)](Chunk *b, Chunk *a)
			{
				if (a == nullptr) { return false; }
				if (b == nullptr) { return true; }

				int ax = a->data.x - x;
				int az = a->data.z - z;

				int bx = b->data.x - x;
				int bz = b->data.z - z;

				unsigned long reza = ax * ax + az * az;
				unsigned long rezb = bx * bx + bz * bz;

				return reza < rezb;
			}
			);
		}
	}
#pragma endregion

	auto renderStaticGeometry = [&]()
	{
		//todo??????????
		//int s = chunkVectorCopy.size();
		//for (int i = s - 1; s >= 0; s--)
		//{
		//	auto chunk = chunkVectorCopy[s];
		//	if (chunk)
		//	{
		//		if (!chunk->dontDrawYet)
		//		{
		//			int facesCount = chunk->elementCountSize;
		//			if (facesCount)
		//			{
		//				glBindVertexArray(chunk->vao);
		//				glDrawElementsInstanced(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0, facesCount);
		//			}
		//		}
		//	}
		//}

		for (auto &chunk : chunkVectorCopy)
		{
			if (chunk)
			{
				if (!chunk->dontDrawYet)
				{
					int facesCount = chunk->elementCountSize;
					if (facesCount)
					{
						glBindVertexArray(chunk->vao);
						glDrawElementsInstanced(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0, facesCount);
					}
				}
			}
		}
	};

	auto renderTransparentGeometry = [&]()
	{
		for (auto &chunk : chunkVectorCopy)
		{
			if (chunk)
			{
				if (!chunk->dontDrawYet)
				{
					int facesCount = chunk->transparentElementCountSize;
					if (facesCount)
					{
						glBindVertexArray(chunk->transparentVao);
						glDrawElementsInstanced(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0, facesCount);
					}
				}
			}
		}
	};

#pragma region depth pre pass 1
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);
		glColorMask(0, 0, 0, 0);
		zpassShader.shader.bind();
		renderStaticGeometry();

	}
#pragma endregion


#pragma region solid pass 2
	glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
	glColorMask(1, 1, 1, 1);
	glDepthFunc(GL_EQUAL);
	defaultShader.shader.bind();
	glDisable(GL_BLEND);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	renderStaticGeometry();
#pragma endregion

#pragma region copy depth 3
	fboCoppy.copyDepthFromOtherFBO(fboMain.fbo, screenX, screenY);
#pragma endregion

#pragma region render only water geometry to depth 4
	glBindFramebuffer(GL_FRAMEBUFFER, fboCoppy.fbo);
	glDepthFunc(GL_LESS);
	zpassShader.shader.bind();
	glUniform1i(zpassShader.u_renderOnlyWater, 1);
	glEnablei(GL_BLEND, 0);
	glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisablei(GL_BLEND, 1);
	glColorMask(0, 0, 0, 0);

	renderTransparentGeometry();
#pragma endregion

#pragma region render with depth peel first part of the transparent of the geometry 5
	glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
	defaultShader.shader.bind();

	glEnablei(GL_BLEND, 0);
	glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisablei(GL_BLEND, 1);

	glColorMask(1, 1, 1, 1);
	glUniform1i(defaultShader.u_depthPeelwaterPass, true);

	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, fboCoppy.depth);
	glUniform1i(defaultShader.u_depthTexture, 2);

	glDepthFunc(GL_LESS);
	//glDisable(GL_CULL_FACE); //todo change
	//todo disable ssr for this step?
	renderTransparentGeometry();
#pragma endregion

#pragma region copy color buffer and last depth for later use 6
	fboCoppy.copyDepthAndColorFromOtherFBO(fboMain.fbo, screenX, screenY);
#pragma endregion

#pragma region render transparent geometry last phaze 7
	glBindFramebuffer(GL_FRAMEBUFFER, fboMain.fbo);
	defaultShader.shader.bind();

	glEnablei(GL_BLEND, 0);
	glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisablei(GL_BLEND, 1);

	glColorMask(1, 1, 1, 1);
	glUniform1i(defaultShader.u_depthPeelwaterPass, 0);
	glUniform1i(defaultShader.u_hasPeelInformation, 1);
	
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, fboCoppy.color);
	glUniform1i(defaultShader.u_PeelTexture, 3);

	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, fboCoppy.depth);
	glUniform1i(defaultShader.u_depthTexture, 2);


	glDepthFunc(GL_LESS);
	//glDisable(GL_CULL_FACE); //todo change
	renderTransparentGeometry();
#pragma endregion

#pragma region copy to main fbo 8

	fboMain.writeAllToOtherFbo(0, screenX, screenY);
	fboLastFrame.copyColorFromOtherFBO(fboMain.fboOnlyFirstTarget, screenX, screenY);
	fboLastFramePositions.copyColorFromOtherFBO(fboMain.fboOnlySecondTarget, screenX, screenY);

#pragma endregion


	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glBindVertexArray(0);
}

//https://www.youtube.com/watch?v=lUo7s-i9Gy4&ab_channel=OREONENGINE
void computeFrustumDimensions(
	glm::vec3 position, glm::vec3 viewDirection,
	float fovRadians, float aspectRatio, float nearPlane, float farPlane,
	glm::vec2 &nearDimensions, glm::vec2 &farDimensions, glm::vec3 &centerNear, glm::vec3 &centerFar)
{
	float tanFov2 = tan(fovRadians) * 2;

	nearDimensions.y = tanFov2 * nearPlane;			//hNear
	nearDimensions.x = nearDimensions.y * aspectRatio;	//wNear

	farDimensions.y = tanFov2 * farPlane;			//hNear
	farDimensions.x = farDimensions.y * aspectRatio;	//wNear

	centerNear = position + viewDirection * farPlane;
	centerFar = position + viewDirection * farPlane;

}

void generateTangentSpace(glm::vec3 v, glm::vec3 &outUp, glm::vec3 &outRight)
{
	glm::vec3 up(0, 1, 0);
	if (v == up)
	{
		outRight = glm::vec3(1, 0, 0);
	}
	else
	{
		outRight = normalize(glm::cross(v, up));
	}
	outUp = normalize(cross(outRight, v));
}

//https://www.youtube.com/watch?v=lUo7s-i9Gy4&ab_channel=OREONENGINE
void computeFrustumSplitCorners(glm::vec3 directionVector,
	glm::vec2 nearDimensions, glm::vec2 farDimensions, glm::vec3 centerNear, glm::vec3 centerFar,
	glm::vec3 &nearTopLeft, glm::vec3 &nearTopRight, glm::vec3 &nearBottomLeft, glm::vec3 &nearBottomRight,
	glm::vec3 &farTopLeft, glm::vec3 &farTopRight, glm::vec3 &farBottomLeft, glm::vec3 &farBottomRight)
{

	glm::vec3 rVector = {};
	glm::vec3 upVectpr = {};

	generateTangentSpace(directionVector, upVectpr, rVector);

	nearTopLeft = centerNear + upVectpr * nearDimensions.y / 2.f - rVector * nearDimensions.x / 2.f;
	nearTopRight = centerNear + upVectpr * nearDimensions.y / 2.f + rVector * nearDimensions.x / 2.f;
	nearBottomLeft = centerNear - upVectpr * nearDimensions.y / 2.f - rVector * nearDimensions.x / 2.f;
	nearBottomRight = centerNear - upVectpr * nearDimensions.y / 2.f + rVector * nearDimensions.x / 2.f;

	farTopLeft = centerNear + upVectpr * farDimensions.y / 2.f - rVector * farDimensions.x / 2.f;
	farTopRight = centerNear + upVectpr * farDimensions.y / 2.f + rVector * farDimensions.x / 2.f;
	farBottomLeft = centerNear - upVectpr * farDimensions.y / 2.f - rVector * farDimensions.x / 2.f;
	farBottomRight = centerNear - upVectpr * farDimensions.y / 2.f + rVector * farDimensions.x / 2.f;

}

glm::mat4 lookAtSafe(glm::vec3 const &eye, glm::vec3 const &center, glm::vec3 const &upVec)
{
	glm::vec3 up = glm::normalize(upVec);

	glm::vec3 f;
	glm::vec3 s;
	glm::vec3 u;

	f = (normalize(center - eye));
	if (f == up || f == -up)
	{
		s = glm::vec3(up.z, up.x, up.y);
		u = (cross(s, f));

	}
	else
	{
		s = (normalize(cross(f, up)));
		u = (cross(s, f));
	}

	glm::mat4 Result(1);
	Result[0][0] = s.x;
	Result[1][0] = s.y;
	Result[2][0] = s.z;
	Result[0][1] = u.x;
	Result[1][1] = u.y;
	Result[2][1] = u.z;
	Result[0][2] = -f.x;
	Result[1][2] = -f.y;
	Result[2][2] = -f.z;
	Result[3][0] = -dot(s, eye);
	Result[3][1] = -dot(u, eye);
	Result[3][2] = dot(f, eye);
	return Result;
}

glm::mat4 calculateLightProjectionMatrix(Camera &camera, glm::vec3 lightDir,
	float nearPlane, float farPlane,
	float zOffset)
{
	glm::vec3 direction = lightDir;
	glm::vec3 eye = camera.position - glm::dvec3(direction);
	glm::vec3 center = eye + direction;

	glm::mat4 lightView = lookAtSafe(eye, center, {0.f,1.f,0.f});//todo option to add a custom vector direction

	glm::vec3 rVector = {};
	glm::vec3 upVectpr = {};
	generateTangentSpace(lightDir, upVectpr, rVector);

	glm::vec2 nearDimensions{};
	glm::vec2 farDimensions{};
	glm::vec3 centerNear{};
	glm::vec3 centerFar{};

	computeFrustumDimensions(camera.position, camera.viewDirection, camera.fovRadians, camera.aspectRatio,
		nearPlane, farPlane, nearDimensions, farDimensions, centerNear, centerFar);

	glm::vec3 nearTopLeft{};
	glm::vec3 nearTopRight{};
	glm::vec3 nearBottomLeft{};
	glm::vec3 nearBottomRight{};
	glm::vec3 farTopLeft{};
	glm::vec3 farTopRight{};
	glm::vec3 farBottomLeft{};
	glm::vec3 farBottomRight{};

	computeFrustumSplitCorners(camera.viewDirection, nearDimensions, farDimensions, centerNear, centerFar,
		nearTopLeft,
		nearTopRight,
		nearBottomLeft,
		nearBottomRight,
		farTopLeft,
		farTopRight,
		farBottomLeft,
		farBottomRight
	);


	glm::vec3 corners[] =
	{
		nearTopLeft,
		nearTopRight,
		nearBottomLeft,
		nearBottomRight,
		farTopLeft,
		farTopRight,
		farBottomLeft,
		farBottomRight,
	};

	float longestDiagonal = glm::distance(nearTopLeft, farBottomRight);

	glm::vec3 minVal{};
	glm::vec3 maxVal{};

	for (int i = 0; i < 8; i++)
	{
		glm::vec4 corner(corners[i], 1);

		glm::vec4 lightViewCorner = lightView * corner;

		if (i == 0)
		{
			minVal = lightViewCorner;
			maxVal = lightViewCorner;
		}
		else
		{
			if (lightViewCorner.x < minVal.x) { minVal.x = lightViewCorner.x; }
			if (lightViewCorner.y < minVal.y) { minVal.y = lightViewCorner.y; }
			if (lightViewCorner.z < minVal.z) { minVal.z = lightViewCorner.z; }

			if (lightViewCorner.x > maxVal.x) { maxVal.x = lightViewCorner.x; }
			if (lightViewCorner.y > maxVal.y) { maxVal.y = lightViewCorner.y; }
			if (lightViewCorner.z > maxVal.z) { maxVal.z = lightViewCorner.z; }

		}

	}

	//keep them square and the same size:
	//https://www.youtube.com/watch?v=u0pk1LyLKYQ&t=99s&ab_channel=WesleyLaFerriere
	if (1)
	{
		float firstSize = maxVal.x - minVal.x;
		float secondSize = maxVal.y - minVal.y;
		float thirdSize = maxVal.z - minVal.z;

		{
			float ratio = longestDiagonal / firstSize;

			glm::vec2 newVecValues = {minVal.x, maxVal.x};
			float dimension = firstSize;
			float dimensionOver2 = dimension / 2.f;

			newVecValues -= glm::vec2(minVal.x + dimensionOver2, minVal.x + dimensionOver2);
			newVecValues *= ratio;
			newVecValues += glm::vec2(minVal.x + dimensionOver2, minVal.x + dimensionOver2);

			minVal.x = newVecValues.x;
			maxVal.x = newVecValues.y;
		}

		{
			float ratio = longestDiagonal / secondSize;

			glm::vec2 newVecValues = {minVal.y, maxVal.y};
			float dimension = secondSize;
			float dimensionOver2 = dimension / 2.f;

			newVecValues -= glm::vec2(minVal.y + dimensionOver2, minVal.y + dimensionOver2);
			newVecValues *= ratio;
			newVecValues += glm::vec2(minVal.y + dimensionOver2, minVal.y + dimensionOver2);

			minVal.y = newVecValues.x;
			maxVal.y = newVecValues.y;
		}

		{//todo this size probably can be far-close
			float ratio = longestDiagonal / thirdSize;

			glm::vec2 newVecValues = {minVal.z, maxVal.z};
			float dimension = thirdSize;
			float dimensionOver2 = dimension / 2.f;

			newVecValues -= glm::vec2(minVal.z + dimensionOver2, minVal.z + dimensionOver2);
			newVecValues *= ratio;
			newVecValues += glm::vec2(minVal.z + dimensionOver2, minVal.z + dimensionOver2);

			minVal.z = newVecValues.x;
			maxVal.z = newVecValues.y;
		}

	}

	float near_plane = minVal.z - zOffset;
	float far_plane = maxVal.z;


	glm::vec2 ortoMin = {minVal.x, minVal.y};
	glm::vec2 ortoMax = {maxVal.x, maxVal.y};

	//remove shadow flicker
	if (1)
	{
		glm::vec2 shadowMapSize(2048, 2048);
		glm::vec2 worldUnitsPerTexel = (ortoMax - ortoMin) / shadowMapSize;

		ortoMin /= worldUnitsPerTexel;
		ortoMin = glm::floor(ortoMin);
		ortoMin *= worldUnitsPerTexel;

		ortoMax /= worldUnitsPerTexel;
		ortoMax = glm::floor(ortoMax);
		ortoMax *= worldUnitsPerTexel;

		float zWorldUnitsPerTexel = (far_plane - near_plane) / 2048;

		near_plane /= zWorldUnitsPerTexel;
		far_plane /= zWorldUnitsPerTexel;
		near_plane = glm::floor(near_plane);
		far_plane = glm::floor(far_plane);
		near_plane *= zWorldUnitsPerTexel;
		far_plane *= zWorldUnitsPerTexel;

	}

	glm::mat4 lightProjection = glm::ortho(ortoMin.x, ortoMax.x, ortoMin.y, ortoMax.y, near_plane, far_plane);

	return lightProjection * lightView;

};


void Renderer::renderShadow(SunShadow &sunShadow,
	ChunkSystem &chunkSystem, Camera &c, ProgramData &programData)
{
	glEnable(GL_DEPTH_TEST);
	glColorMask(0, 0, 0, 0);

	glViewport(0, 0, 2048, 2048);
	glBindFramebuffer(GL_FRAMEBUFFER, sunShadow.shadowMap.fbo);
	glClear(GL_DEPTH_BUFFER_BIT);


	glm::ivec3 newPos = c.position;

	{
		newPos.y = 120;

		glm::vec3 moveDir = skyBoxRenderer.sunPos;

		float l = glm::length(moveDir);
		if (l > 0.0001)
		{
			moveDir /= l;
			newPos += moveDir * (float)CHUNK_SIZE * 12.f;
		}
	}

	//glm::vec3 posFloat = {};
	//glm::ivec3 posInt = {};
	//c.decomposePosition(posFloat, posInt);

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = newPos;
	
	glm::ivec3 playerPos = c.position;

	glm::vec3 vectorToPlayer = glm::normalize(glm::vec3(playerPos - newPos));

	//posInt += glm::vec3(0, 0, 10);

	float projectSize = 50;

	float near_plane = 1.0f, far_plane = 260.f;
	glm::mat4 lightProjection = glm::ortho(-projectSize, projectSize, -projectSize, projectSize,
		near_plane, far_plane);
	//auto mvp = lightProjection * glm::lookAt({},
	//	-skyBoxRenderer.sunPos, glm::vec3(0, 1, 0));

	auto mvp = lightProjection * glm::lookAt({},
		vectorToPlayer, glm::vec3(0, 1, 0));

	

	//auto mat = calculateLightProjectionMatrix(c, -skyBoxRenderer.sunPos, 1, 260, 25);

	sunShadow.lightSpaceMatrix = mvp;
	sunShadow.lightSpacePosition = posInt;
	
	//sunShadow.lightSpaceMatrix = mat;
	//sunShadow.lightSpacePosition = {};

#pragma region setup uniforms and stuff
	{
		zpassShader.shader.bind();
		glUniformMatrix4fv(zpassShader.u_viewProjection, 1, GL_FALSE, &sunShadow.lightSpaceMatrix[0][0]);
		glUniform3fv(zpassShader.u_positionFloat, 1, &posFloat[0]);
		glUniform3iv(zpassShader.u_positionInt, 1, &sunShadow.lightSpacePosition[0]);
		glUniform1i(zpassShader.u_renderOnlyWater, 0);
	}
#pragma endregion

	for (auto &chunk : chunkSystem.loadedChunks)
	{
		if (chunk)
		{
			if (!chunk->dontDrawYet)
			{
				int facesCount = chunk->elementCountSize;
				if (facesCount)
				{
					glBindVertexArray(chunk->vao);
					glDrawElementsInstanced(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0, facesCount);
				}
			}
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glColorMask(1, 1, 1, 1);

}

float cubePositions[] = {
		-0.5f, +0.5f, +0.5f, // 0
		+0.5f, +0.5f, +0.5f, // 1
		+0.5f, +0.5f, -0.5f, // 2
		-0.5f, +0.5f, -0.5f, // 3
		-0.5f, +0.5f, -0.5f, // 4
		+0.5f, +0.5f, -0.5f, // 5
		+0.5f, -0.5f, -0.5f, // 6
		-0.5f, -0.5f, -0.5f, // 7
		+0.5f, +0.5f, -0.5f, // 8
		+0.5f, +0.5f, +0.5f, // 9
		+0.5f, -0.5f, +0.5f, // 10
		+0.5f, -0.5f, -0.5f, // 11
		-0.5f, +0.5f, +0.5f, // 12
		-0.5f, +0.5f, -0.5f, // 13
		-0.5f, -0.5f, -0.5f, // 14
		-0.5f, -0.5f, +0.5f, // 15
		+0.5f, +0.5f, +0.5f, // 16
		-0.5f, +0.5f, +0.5f, // 17
		-0.5f, -0.5f, +0.5f, // 18
		+0.5f, -0.5f, +0.5f, // 19
		+0.5f, -0.5f, -0.5f, // 20
		-0.5f, -0.5f, -0.5f, // 21
		-0.5f, -0.5f, +0.5f, // 22
		+0.5f, -0.5f, +0.5f, // 23
};

unsigned int cubeIndicesData[] = {
	0,   1,  2,  0,  2,  3, // Top
	4,   5,  6,  4,  6,  7, // Back
	8,   9, 10,  8, 10, 11, // Right
	12, 13, 14, 12, 14, 15, // Left
	16, 17, 18, 16, 18, 19, // Front
	20, 22, 21, 20, 23, 22, // Bottom
};


void GyzmosRenderer::create()
{
	gyzmosCubeShader.loadShaderProgramFromFile(RESOURCES_PATH "gyzmosCubeShader.vert",
		RESOURCES_PATH "gyzmosCubeShader.frag");

	GET_UNIFORM(gyzmosCubeShader, u_viewProjection);
	GET_UNIFORM(gyzmosCubeShader, u_positionInt);
	GET_UNIFORM(gyzmosCubeShader, u_positionFloat);
	

	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glCreateBuffers(1, &vertexDataBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexDataBuffer);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(cubePositions), cubePositions, 0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribDivisor(0, 0);

	glCreateBuffers(1, &blockPositionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, blockPositionBuffer);
	glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_STREAM_DRAW);
	glVertexAttribIPointer(1, 3, GL_INT, 0, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);

	glCreateBuffers(1, &cubeIndices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndices);
	glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndicesData), (void*)cubeIndicesData, 0);

	glBindVertexArray(0);
}


void GyzmosRenderer::render(Camera &c, glm::ivec3 posInt, glm::vec3 posFloat)
{

	if (cubes.empty()) { return; }

	glNamedBufferData(blockPositionBuffer, cubes.size() * sizeof(CubeData), cubes.data(), GL_STREAM_DRAW);
	
	gyzmosCubeShader.bind();
	
	glDepthFunc(GL_LEQUAL);
	glBindVertexArray(vao);
	
	auto mvp = c.getProjectionMatrix() * glm::lookAt({0,0,0}, c.viewDirection, c.up);

	glUniformMatrix4fv(u_viewProjection, 1, GL_FALSE, &mvp[0][0]);
	glUniform3fv(u_positionFloat, 1, &posFloat[0]);
	glUniform3iv(u_positionInt, 1, &posInt[0]);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, cubes.size());
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindVertexArray(0);
	glDepthFunc(GL_LESS);

	cubes.clear();
}



void PointDebugRenderer::create()
{
	pointDebugShader.loadShaderProgramFromFile(RESOURCES_PATH "pointDebugShader.vert",
		RESOURCES_PATH "pointDebugShader.frag");

	GET_UNIFORM(pointDebugShader, u_viewProjection);
	GET_UNIFORM(pointDebugShader, u_positionInt);
	GET_UNIFORM(pointDebugShader, u_positionFloat);
	GET_UNIFORM(pointDebugShader, u_blockPositionInt);
	GET_UNIFORM(pointDebugShader, u_blockPositionFloat);


	
}

void PointDebugRenderer::renderPoint(Camera &c, glm::dvec3 point)
{
	pointDebugShader.bind();

	auto mvp = c.getProjectionMatrix() * glm::lookAt({0,0,0}, c.viewDirection, c.up);

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = {};
	c.decomposePosition(posFloat, posInt);

	glUniformMatrix4fv(u_viewProjection, 1, GL_FALSE, &mvp[0][0]);
	glUniform3fv(u_positionFloat, 1, &posFloat[0]);
	glUniform3iv(u_positionInt, 1, &posInt[0]);

	glm::vec3 posFloatBlock = {};
	glm::ivec3 posIntBlock = {};
	decomposePosition(point, posFloatBlock, posIntBlock);

	glUniform3fv(u_blockPositionFloat, 1, &posFloatBlock[0]);
	glUniform3iv(u_blockPositionInt, 1, &posIntBlock[0]);

	glPointSize(15);
	glDrawArrays(GL_POINTS, 0, 1);

}

void PointDebugRenderer::renderCubePoint(Camera &c, glm::dvec3 point)
{
	renderPoint(c, point);
	
	renderPoint(c, point + glm::dvec3(0.5,0.5,-0.5));
	renderPoint(c, point + glm::dvec3(0.5,0.5,0.5));
	renderPoint(c, point + glm::dvec3(-0.5,0.5,-0.5));
	renderPoint(c, point + glm::dvec3(-0.5,0.5,0.5));

	renderPoint(c, point + glm::dvec3(0.5, -0.5, -0.5));
	renderPoint(c, point + glm::dvec3(0.5, -0.5, 0.5));
	renderPoint(c, point + glm::dvec3(-0.5, -0.5, -0.5));
	renderPoint(c, point + glm::dvec3(-0.5, -0.5, 0.5));

}


#undef GET_UNIFORM

void Renderer::FBO::create(GLint addColor, bool addDepth, GLint addSecondaryRenderTarger)
{
	if (addColor == 1) { addColor = GL_RGBA8; }
	if (addSecondaryRenderTarger == 1) { addSecondaryRenderTarger = GL_RGBA8; }

	colorFormat = addColor;
	secondaryColorFormat = addSecondaryRenderTarger;


	permaAssert(!(addColor == 0 && addSecondaryRenderTarger == 1));


	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);


	if (addColor)
	{
		glGenTextures(1, &color);
		glBindTexture(GL_TEXTURE_2D, color);
		glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, 1, 1
			, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
	}

	if (addSecondaryRenderTarger)
	{
		glGenTextures(1, &secondaryColor);
		glBindTexture(GL_TEXTURE_2D, secondaryColor);
		// Set the width and height of your texture (e.g., 1024x1024)
		glTexImage2D(GL_TEXTURE_2D, 0, secondaryColorFormat, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Attach the secondary color texture to the framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, secondaryColor, 0);

		unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
		glDrawBuffers(2, attachments);
	}

	

	if (addDepth)
	{
		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 1, 1,
			0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

		if (!addColor)
		{
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
	}

	// Check for framebuffer completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer Error!!!\n";
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (addSecondaryRenderTarger)
	{
		glGenFramebuffers(1, &fboOnlyFirstTarget);
		glBindFramebuffer(GL_FRAMEBUFFER, fboOnlyFirstTarget);

		if (addColor)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
		}

		if (addDepth)
		{
			if (!addColor)
			{
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
		}

		// Check for framebuffer completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Framebuffer Error!!!\n";
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	if (addSecondaryRenderTarger)
	{
		glGenFramebuffers(1, &fboOnlySecondTarget);
		glBindFramebuffer(GL_FRAMEBUFFER, fboOnlySecondTarget);

		if (addColor)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, secondaryColor, 0);
		}

		if (addDepth)
		{
			if (!addColor)
			{
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
		}

		// Check for framebuffer completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Framebuffer Error!!!\n";
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void Renderer::FBO::updateSize(int x, int y)
{
	if (size.x != x || size.y != y)
	{
		if (color)
		{
			glBindTexture(GL_TEXTURE_2D, color);
			glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, x, y,
				0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		}

		if (secondaryColor)
		{
			glBindTexture(GL_TEXTURE_2D, secondaryColor);
			glTexImage2D(GL_TEXTURE_2D, 0, secondaryColorFormat, x, y,
				0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		}
		
		if (depth)
		{
			glBindTexture(GL_TEXTURE_2D, depth);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, x, y,
				0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}

		size = glm::ivec2(x, y);
	}
}

void Renderer::FBO::copyDepthFromMainFBO(int w, int h)
{
	copyDepthFromOtherFBO(0, w, h);
}

void Renderer::FBO::copyColorFromMainFBO(int w, int h)
{
	copyColorFromOtherFBO(0, w, h);
}

void Renderer::FBO::copyDepthFromOtherFBO(GLuint other, int w, int h)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, other);
	glBindTexture(GL_TEXTURE_2D, depth);

	glBlitFramebuffer(
		0, 0, w, h,  // Source rectangle (usually the whole screen)
		0, 0, w, h,  // Destination rectangle (the size of your texture)
		GL_DEPTH_BUFFER_BIT, GL_NEAREST// You can adjust the filter mode as needed
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::FBO::copyColorFromOtherFBO(GLuint other, int w, int h)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, other);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

	glBlitFramebuffer(
		0, 0, w, h,  // Source rectangle (usually the whole screen)
		0, 0, w, h,  // Destination rectangle (the size of your texture)
		GL_COLOR_BUFFER_BIT, GL_NEAREST// You can adjust the filter mode as needed
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::FBO::copyDepthAndColorFromOtherFBO(GLuint other, int w, int h)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, other);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

	glBlitFramebuffer(
		0, 0, w, h,  // Source rectangle (usually the whole screen)
		0, 0, w, h,  // Destination rectangle (the size of your texture)
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST// You can adjust the filter mode as needed
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::FBO::writeAllToOtherFbo(GLuint other, int w, int h)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, other);

	glBlitFramebuffer(
		0, 0, w, h,  // Source rectangle (usually the whole screen)
		0, 0, w, h,  // Destination rectangle (the size of your texture)
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST// You can adjust the filter mode as needed
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::FBO::clearFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);


	if (depth && color)
	{
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	}
	else if (depth)
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	else if (color)
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	if (secondaryColor)
	{
		const float clearColor2[] = {0.0f, 0.0f, 0.0f, 0.0f};
		glClearBufferfv(GL_COLOR, 1, clearColor2);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

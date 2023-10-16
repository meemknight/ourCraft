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

#define GET_UNIFORM(s, n) n = s.getUniform(#n);


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

	skyBoxRenderer.create();

	defaultShader.loadShaderProgramFromFile(RESOURCES_PATH "defaultShader.vert", RESOURCES_PATH "defaultShader.frag");
	defaultShader.bind();

	GET_UNIFORM(defaultShader, u_viewProjection);
	GET_UNIFORM(defaultShader, u_typesCount);
	GET_UNIFORM(defaultShader, u_positionInt);
	GET_UNIFORM(defaultShader, u_positionFloat);
	GET_UNIFORM(defaultShader, u_texture);
	GET_UNIFORM(defaultShader, u_time);
	GET_UNIFORM(defaultShader, u_showLightLevels);
	GET_UNIFORM(defaultShader, u_skyLightIntensity);
	GET_UNIFORM(defaultShader, u_lightsCount);
	GET_UNIFORM(defaultShader, u_pointPosF);
	GET_UNIFORM(defaultShader, u_pointPosI);

	
	u_vertexData = getStorageBlockIndex(defaultShader.id, "u_vertexData");
	glShaderStorageBlockBinding(defaultShader.id, u_vertexData, 1);
	glGenBuffers(1, &vertexDataBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexDataBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(vertexData), vertexData, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vertexDataBuffer);

	u_vertexUV = getStorageBlockIndex(defaultShader.id, "u_vertexUV");
	glShaderStorageBlockBinding(defaultShader.id, u_vertexUV, 2);
	glGenBuffers(1, &vertexUVBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexUVBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(vertexUV), vertexUV, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vertexUVBuffer);

	u_textureSamplerers = getStorageBlockIndex(defaultShader.id, "u_textureSamplerers");
	glShaderStorageBlockBinding(defaultShader.id, u_textureSamplerers, 3);
	glGenBuffers(1, &textureSamplerersBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureSamplerersBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint64) * blocksLoader.gpuIds.size(), blocksLoader.gpuIds.data(), 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, textureSamplerersBuffer);

	//normals
	{



	}


	u_lights = getStorageBlockIndex(defaultShader.id, "u_lights");
	glShaderStorageBlockBinding(defaultShader.id, u_lights, 4);
	//glGenBuffers(1, &textureSamplerersBuffer);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureSamplerersBuffer);
	//glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint64) * blocksLoader.gpuIds.size(), blocksLoader.gpuIds.data(), 0);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, textureSamplerersBuffer);

	


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
	

	for (int i = 0; i < 4; i++)
	{
		topFaces[i]->x += offsetVector.x;
		topFaces[i]->z += offsetVector.y;
	
		bottomFaces[i]->x -= offsetVector.x;
		bottomFaces[i]->z -= offsetVector.y;


		topFacesGrass[i]->x += offsetVector.x;
		topFacesGrass[i]->z += offsetVector.y;

		bottomFacesGrass[i]->x -= offsetVector.x;
		bottomFacesGrass[i]->z -= offsetVector.y;
	}


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

void Renderer::renderFromBakedData(ChunkSystem &chunkSystem, Camera &c, ProgramData &programData
	, bool showLightLevels, int skyLightIntensity, glm::dvec3 pointPos)
{

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = {};
	glm::ivec3 blockPosition = from3DPointToBlock(c.position);
	c.decomposePosition(posFloat, posInt);


	programData.texture.bind(0);
	programData.numbersTexture.bind(1);
	defaultShader.bind();

#pragma region lights
	{
		auto c = chunkSystem.getChunkSafeFromBlockPos(posInt.x, posInt.z);

		if (c)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, c->lightsBuffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, c->lightsBuffer);
			glUniform1i(u_lightsCount, c->lightsElementCountSize);
		}
	}
#pragma endregion




	auto mvp = c.getProjectionMatrix() * glm::lookAt({0,0,0}, c.viewDirection, c.up);

	glUniformMatrix4fv(u_viewProjection, 1, GL_FALSE, &mvp[0][0]);
	glUniform3fv(u_positionFloat, 1, &posFloat[0]);
	glUniform3iv(u_positionInt, 1, &posInt[0]);
	glUniform1i(u_typesCount, BlocksCount);	//remove
	glUniform1f(u_time, std::clock() / 400.f);
	glUniform1i(u_showLightLevels, showLightLevels);
	glUniform1i(u_skyLightIntensity, skyLightIntensity);
	
	{
		glm::ivec3 i;
		glm::vec3 f;
		decomposePosition(pointPos, f, i);

		glUniform3fv(u_pointPosF, 1, &f[0]);
		glUniform3iv(u_pointPosI, 1, &i[0]);
	}


	glDisable(GL_BLEND);

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
					glDrawArraysInstanced(GL_TRIANGLES, 0, 6, facesCount);
				}
			}
		}
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	auto chunkVectorCopy = chunkSystem.loadedChunks;

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
					glDrawArraysInstanced(GL_TRIANGLES, 0, 6, facesCount);
				}
			}
		}
	}


	glBindVertexArray(0);




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

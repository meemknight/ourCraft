#include "rendering/renderer.h"
#include <ctime>
#include "blocks.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/transform.hpp>

#define GET_UNIFORM(s, n) n = s.getUniform(#n);


//data format:

// short orientation
// short type
// int x
// int y
// int z

int atlasData[] = {
	//front
	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 3,//sand_stone
	4, 11,//snow_dirt
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	3, 13, //block shelf
	5, 8, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	//back
	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 3,//sand_stone
	4, 11,//snow_dirt
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	3, 13, //block shelf
	5, 8, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	//top
	0, 0,
	0, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 4,//sand_stone
	2, 11,// snow_grass
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	4, 15, //block shelf
	5, 14, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	//bottom
	0, 0,
	2, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	5, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 2,//sand_stone
	2, 15, // snow_grass
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	4, 15, //block shelf
	5, 14, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	//left
	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 3,//sand_stone
	4, 11,//snow_dirt
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	3, 13, //block shelf
	5, 8, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	//right
	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 3,//sand_stone
	4, 11,//snow_dirt
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	3, 13, //block shelf
	5, 8, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	///// moving grass
	0, 0,
	0, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 4,//sand_stone
	2, 11,// snow_grass
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	4, 15, //block shelf
	5, 14, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	0, 0,
	2, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	5, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 2,//sand_stone
	2, 15, // snow_grass
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	4, 15, //block shelf
	5, 14, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 3,//sand_stone
	4, 11,//snow_dirt
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	3, 13, //block shelf
	5, 8, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 3,//sand_stone
	4, 11,//snow_dirt
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	3, 13, //block shelf
	5, 8, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose


	///// moving blocks
	//front
	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 3,//sand_stone
	4, 11,//snow_dirt
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	3, 13, //block shelf
	5, 8, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	//back
	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 3,//sand_stone
	4, 11,//snow_dirt
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	3, 13, //block shelf
	5, 8, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	//top
	0, 0,
	0, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 4,//sand_stone
	2, 11,// snow_grass
	5, 12,//leaves

	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	4, 15, //block shelf
	5, 14, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	//bottom
	0, 0,
	2, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	5, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 2,//sand_stone
	2, 15, // snow_grass
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	4, 15, //block shelf
	5, 14, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	//left
	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 3,//sand_stone
	4, 11,//snow_dirt
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	3, 13, //block shelf
	5, 8, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	//right
	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 3,//sand_stone
	4, 11,//snow_dirt
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	3, 13, //block shelf
	5, 8, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

};

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

void Renderer::create()
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

	u_atlasBlockIndex = getStorageBlockIndex(defaultShader.id, "u_atlasPositions");
	glShaderStorageBlockBinding(defaultShader.id, u_atlasBlockIndex, 0);
	glGenBuffers(1, &atlasBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, atlasBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(atlasData), atlasData, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, atlasBuffer);
	

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


	glCreateBuffers(1, &vertexBuffer);
	//glNamedBufferData(vertexBuffer, sizeof(data), data, GL_DYNAMIC_DRAW);

	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
		
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

		//glEnableVertexAttribArray(0);
		//glVertexAttribIPointer(0, 1, GL_SHORT, 5*sizeof(int), 0);								//short
		//glVertexAttribDivisor(0, 1);
		//
		//glEnableVertexAttribArray(1);
		//glVertexAttribIPointer(1, 1, GL_SHORT, 5 * sizeof(int), (void*)(1 * sizeof(short)));	//short
		//glVertexAttribDivisor(1, 1);
		//
		//glEnableVertexAttribArray(2);
		//glVertexAttribIPointer(2, 1, GL_INT, 5 * sizeof(int), (void *)(2 * sizeof(short)));		//int
		//glVertexAttribDivisor(2, 1);
		//
		//glEnableVertexAttribArray(3);
		//glVertexAttribIPointer(3, 3, GL_INT, 5 * sizeof(int), (void*)(4 * sizeof(short)));
		//glVertexAttribDivisor(3, 1);

		glEnableVertexAttribArray(0);
		glVertexAttribIPointer(0, 1, GL_SHORT, 4 * sizeof(int), 0);
		glVertexAttribDivisor(0, 1);

		glEnableVertexAttribArray(1);
		glVertexAttribIPointer(1, 1, GL_SHORT, 4 * sizeof(int), (void *)(1 * sizeof(short)));
		glVertexAttribDivisor(1, 1);

		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 3, GL_INT, 4 * sizeof(int), (void *)(1 * sizeof(int)));
		glVertexAttribDivisor(2, 1);

	glBindVertexArray(0);

}

void Renderer::updateDynamicBlocks()
{
	float time = std::clock();

	glm::vec3 topFrontLeft = {-0.5f, 0.5f, 0.5f};
	glm::vec3 topFrontRight = {0.5f, 0.5f, 0.5f};
	glm::vec3 topBackLeft = {-0.5f, 0.5f, -0.5f};
	glm::vec3 topBackRight = {0.5f, 0.5f, -0.5f};
	glm::vec3 bottomFrontLeft = {-0.5f, -0.5f, 0.5f};
	glm::vec3 bottomFrontRight = {0.5f, -0.5f, 0.5f};
	glm::vec3 bottomBackLeft = {-0.5f, -0.5f, -0.5f};
	glm::vec3 bottomBackRight = {0.5f, -0.5f, -0.5f};

	glm::vec3 *topFaces[4] = {&topFrontLeft, &topFrontRight, &topBackLeft, &topBackRight};
	glm::vec3* bottomFaces[4] = {&bottomFrontLeft, &bottomFrontRight, &bottomBackLeft, &bottomBackRight};
	
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
		//topFaces[i]->y ;
		topFaces[i]->z += offsetVector.y;
	
		bottomFaces[i]->x -= offsetVector.x;
		bottomFaces[i]->z -= offsetVector.y;
	}


	float newData[] =
	{
		//grass
		topFrontRight.x, topFrontRight.y, topFrontRight.z,
		topBackLeft.x,topBackLeft.y,topBackLeft.z,
		-0.5, -0.5, -0.5,
		-0.5, -0.5, -0.5,
		0.5, -0.5, 0.5,
		topFrontRight.x, topFrontRight.y, topFrontRight.z,

		-0.5, -0.5, -0.5,
		topBackLeft.x,topBackLeft.y,topBackLeft.z,
		topFrontRight.x, topFrontRight.y, topFrontRight.z,
		topFrontRight.x, topFrontRight.y, topFrontRight.z,
		0.5, -0.5, 0.5,
		-0.5, -0.5, -0.5,

		topFrontLeft.x,topFrontLeft.y,topFrontLeft.z,
		topBackRight.x,topBackRight.y,topBackRight.z,
		0.5, -0.5, -0.5,
		0.5, -0.5, -0.5,
		-0.5, -0.5, 0.5,
		topFrontLeft.x,topFrontLeft.y,topFrontLeft.z,

		
		-0.5, -0.5, 0.5,
		0.5, -0.5, -0.5,
		topBackRight.x,topBackRight.y,topBackRight.z,
		topBackRight.x,topBackRight.y,topBackRight.z,
		topFrontLeft.x,topFrontLeft.y,topFrontLeft.z,
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

void Renderer::render(std::vector<int> &data, Camera &c, gl2d::Texture &texture)
{

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = {};
	c.decomposePosition(posFloat, posInt);

	glNamedBufferData(vertexBuffer, sizeof(int) * data.size(), data.data(), GL_STREAM_DRAW);
	int facesCount = data.size() / 4;

	glBindVertexArray(vao);
	texture.bind(0);

	defaultShader.bind();

	auto mvp = c.getProjectionMatrix() * glm::lookAt({0,0,0}, c.viewDirection, c.up);

	glUniformMatrix4fv(u_viewProjection, 1, GL_FALSE, &mvp[0][0]);

	glUniform3fv(u_positionFloat, 1, &posFloat[0]);
	glUniform3iv(u_positionInt, 1, &posInt[0]);
	glUniform1i(u_typesCount, BlocksCount);
	glUniform1f(u_time, std::clock() / 400.f);

	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, facesCount);

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

#include "renderer.h"

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
	3, 14 //gravel
};

void Renderer::create()
{
	defaultShader.loadShaderProgramFromFile(RESOURCES_PATH "defaultShader.vert", RESOURCES_PATH "defaultShader.frag");
	defaultShader.bind();

	GET_UNIFORM(defaultShader, u_viewProjection);
	GET_UNIFORM(defaultShader, u_typesCount);
	GET_UNIFORM(defaultShader, u_positionInt);
	GET_UNIFORM(defaultShader, u_positionFloat);
	GET_UNIFORM(defaultShader, u_texture);

	u_atlasBlockIndex = getStorageBlockIndex(defaultShader.id, "u_atlasPositions");
	glShaderStorageBlockBinding(defaultShader.id, u_atlasBlockIndex, 0);
	glGenBuffers(1, &atlasBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, atlasBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(atlasData), atlasData, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, atlasBuffer);


	glCreateBuffers(1, &vertexBuffer);
	//glNamedBufferData(vertexBuffer, sizeof(data), data, GL_DYNAMIC_DRAW);


	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
		
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

		glEnableVertexAttribArray(0);
		glVertexAttribIPointer(0, 1, GL_SHORT, 4*sizeof(int), 0);
		glVertexAttribDivisor(0, 1);

		glEnableVertexAttribArray(1);
		glVertexAttribIPointer(1, 1, GL_SHORT, 4 * sizeof(int), (void*)(1 * sizeof(short)));
		glVertexAttribDivisor(1, 1);

		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 3, GL_INT, 4 * sizeof(int), (void*)(1 * sizeof(int)));
		glVertexAttribDivisor(2, 1);

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


#undef GET_UNIFORM

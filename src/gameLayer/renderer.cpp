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
	2, 1,//sand
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
	GET_UNIFORM(defaultShader, u_position);
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


#undef GET_UNIFORM
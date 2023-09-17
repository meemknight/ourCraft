#include <blocksLoader.h>
#include "blocks.h"

const char *texturesNames[] = {
	
	"dirt",				//1
	"grass_block_top",	//2 
	"grass_block_side",	//3
	"ice",				//4
	"oak_log",			//5
	"oak_log_top",		//6
	"oak_planks",		//7
	"cobblestone",		//8
	"gold_block",		//9
	"bricks",			//10
	"sand",				//11
	"sandstone",		//12
	"sandstone_bottom",	//13
	"sandstone_top",	//14
	"grass_block_snow",	//15
	"snow",				//16
	"dark_oak_leaves",	//17
	"gold_ore",			//18
	"coal_ore",			//19
	"stone_bricks",		//20
	"iron_ore",			//21
	"diamond_ore",		//22
	"bookshelf",		//23
	"birch_log",		//24
	"birch_log_top",	//25
	"gravel",			//26
	"grass", //herbs	//27
	"rose",				//28
	"stone",			//29
};

//front
//back
//top
//bottom
//left
//right

uint16_t blocksLookupTable[] = {

	//air
	0,0,0,0,0,0,

	//grass
	0,3,2,1,3,3,

	// dirt
	1,1,1,1,1,1,

	//stone
	29,29,29,29,29,29,

	//ice
	4,4,4,4,4,4,

	//log
	5,5,6,6,5,5,

	//wooden_plank
	7,7,7,7,7,7,

	//cobblestone
	8,8,8,8,8,8,

	//gold_block
	9,9,9,9,9,9,

	//bricks
	10,10,10,10,10,10,

	//sand
	11,11,11,11,11,11,

	//sand_stone
	12,12,14,13,12,12,

	//snow_dirt
	15,15,16,1,15,15,

	//leaves 
	17,17,17,17,17,17,

	// gold ore
	18,18,18,18,18,18,

	// coal ore
	19,19,19,19,19,19,

	//stone brick
	20,20,20,20,20,20,

	// iron ore
	21,21,21,21,21,21,

	// diamond ore
	22,22,22,22,22,22,

	//boock shelf
	23,23,7,7,23,23,

	//birch wood
	24,24,25,25,24,24,

	//gravel
	26,26,26,26,26,26,

	//herbs
	27,27,27,27,27,27,

	//rose
	28,28,28,28,28,28,

};


void BlocksLoader::loadAllTextures()
{
	size_t count = sizeof(texturesNames) / sizeof(char *);
	
	gpuIds.reserve(count + 1);
	texturesIds.reserve(count+ 1);

	{
		unsigned char data[16] = {};
		data[0] = 146;
		data[1] = 52;
		data[2] = 235;
		data[3] = 255;

		data[4] = 0;
		data[5] = 0;
		data[6] = 0;
		data[7] = 255;

		data[8] = 146;
		data[9] = 52;
		data[10] = 235;
		data[11] = 255;

		data[12] = 0;
		data[13] = 0;
		data[14] = 0;
		data[15] = 255;

		gl2d::Texture t;
		//t.createFromBuffer((char*)data, 2, 2, true, false);
		t.create1PxSquare();

		auto handle = glGetTextureHandleARB(t.id);
		glMakeTextureHandleResidentARB(handle);
		gpuIds.push_back(t.id);
		texturesIds.push_back(handle);
	}

	

	std::string path;

	for (int i = 0; i < count; i++)
	{
		gl2d::Texture t;
		path.clear();

		path += RESOURCES_PATH;
		path += "pbr/block/";
		path += texturesNames[i];
		path += ".png";

		t.loadFromFile(path.c_str(), true, false);
		t.bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.f);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 4.f);

		glGenerateMipmap(GL_TEXTURE_2D);

		texturesIds.push_back(t.id);

		auto handle = glGetTextureHandleARB(t.id);
		glMakeTextureHandleResidentARB(handle);

		gpuIds.push_back(handle);
	}


}

uint16_t getGpuIdIndexForBlock(short type, int face)
{
	return blocksLookupTable[type * 6 + face];
}

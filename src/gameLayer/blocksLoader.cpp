#include <blocksLoader.h>
#include "blocks.h"
#include <fstream>
#include <iostream>

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
	"water",			//30
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
	3,3,2,1,3,3,

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

	//water
	30,30,30,30,30,30,

};

void fixAlpha(unsigned char *buffer, int w, int h)
{
	auto sample = [&](int x, int y) -> glm::ivec4 
	{
		glm::ivec4 rez;

		rez.r = buffer[(x + y * w) * 4 + 0];
		rez.g = buffer[(x + y * w) * 4 + 1];
		rez.b = buffer[(x + y * w) * 4 + 2];
		rez.a = buffer[(x + y * w) * 4 + 3];

		return rez;
	};

	auto set = [&](int x, int y, glm::ivec3 c)
	{
		buffer[(x + y * w) * 4 + 0] = c.r;
		buffer[(x + y * w) * 4 + 1] = c.g;
		buffer[(x + y * w) * 4 + 2] = c.b;
	};

	auto sampleA = [&](int x, int y) -> unsigned char
	{
		return buffer[(x + y * w) * 4 + 3];
	};

	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
		{

			if (sampleA(x, y) == 0)
			{

				if (x < w - 1)
				{
					auto c = sample(x + 1, y);
					if (c.a > 0)
					{
						set(x, y, c);
						continue;
					}
				}

				if (y < h - 1)
				{
					auto c = sample(x, y + 1);
					if (c.a > 0)
					{
						set(x, y, c);
						continue;
					}
				}

				if (x < w - 1 && y < h - 1)
				{
					auto c = sample(x + 1, y + 1);
					if (c.a > 0)
					{
						set(x, y, c);
						continue;
					}
				}

				if (x > 0 && y < h - 1)
				{
					auto c = sample(x - 1, y + 1);
					if (c.a > 0)
					{
						set(x, y, c);
						continue;
					}
				}

				if (x > 0)
				{
					auto c = sample(x - 1, y);
					if (c.a > 0)
					{
						set(x, y, c);
						continue;
					}
				}

				if (y > 0)
				{
					auto c = sample(x, y - 1);
					if (c.a > 0)
					{
						set(x, y, c);
						continue;
					}
				}

				if (x > 0 && y > 0)
				{
					auto c = sample(x - 1, y - 1);
					if (c.a > 0)
					{
						set(x, y, c);
						continue;
					}
				}

				if (x < w - 1 && y > 0)
				{
					auto c = sample(x + 1, y - 1);
					if (c.a > 0)
					{
						set(x, y, c);
						continue;
					}
				}

			}

		}
}

void createFromFileDataWithAplhaFixing(gl2d::Texture &t, const unsigned char *image_file_data, 
	const size_t image_file_size
	, bool pixelated, bool useMipMaps)
{
	stbi_set_flip_vertically_on_load(true);

	int width = 0;
	int height = 0;
	int channels = 0;

	unsigned char *decodedImage = stbi_load_from_memory(image_file_data, (int)image_file_size, &width, &height, &channels, 4);

	for (int i = 0; i < 128; i++)
	{
		fixAlpha(decodedImage, width, height);
	}
	

	t.createFromBuffer((const char *)decodedImage, width, height, pixelated, useMipMaps);

	//Replace stbi allocators
	free((void *)decodedImage);
}

void loadFromFileWithAplhaFixing(gl2d::Texture &t, const char *fileName, bool pixelated, bool useMipMaps)
{
	std::ifstream file(fileName, std::ios::binary);

	if (!file.is_open())
	{
		char c[300] = {0};
		strcat(c, "error openning: ");
		strcat(c + strlen(c), fileName);
		std::cout << c;
		//errorFunc(c);//todo propper error function
		return;
	}

	int fileSize = 0;
	file.seekg(0, std::ios::end);
	fileSize = (int)file.tellg();
	file.seekg(0, std::ios::beg);
	unsigned char *fileData = new unsigned char[fileSize];
	file.read((char *)fileData, fileSize);
	file.close();

	createFromFileDataWithAplhaFixing(t, fileData, fileSize, pixelated, useMipMaps);

	delete[] fileData;

}

void BlocksLoader::loadAllTextures()
{
	size_t count = sizeof(texturesNames) / sizeof(char *);
	
	gpuIds.reserve(count + 1);
	texturesIds.reserve(count+ 1);

	{
		unsigned char data[16] = {};

		{
			int i = 0;
			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 255;

			data[i++] = 146;
			data[i++] = 52;
			data[i++] = 235;
			data[i++] = 255;

			data[i++] = 146;
			data[i++] = 52;
			data[i++] = 235;
			data[i++] = 255;

			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 255;
		}
		


		gl2d::Texture t;
		t.createFromBuffer((char*)data, 2, 2, true, false);
	
		texturesIds.push_back(t.id);
		auto handle = glGetTextureHandleARB(t.id);
		glMakeTextureHandleResidentARB(handle);
		gpuIds.push_back(handle);
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

		loadFromFileWithAplhaFixing(t, path.c_str(), true, false);
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

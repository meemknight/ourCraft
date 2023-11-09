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
	"jungle_log",		//31
	"jungle_log_top",	//32
	"leaves_jungle",	//33
	"palm_log",			//34
	"palm_log_top",		//35
	"palm_leaves",		//36
	"cactus_bud",		//37
	"dead_bush",		//38
	"jungle_planks",	//39
	"clay",				//40
	"hardened_clay",	//41
	"mud",				//42
	"packed_mud",		//43
	"mud_bricks",		//44
	"controll1",		//45
	"controll2",		//46
	"controll3",		//47
	"controll4",		//48
	"leaves_birch",		//49
	"spruce_leaves",	//50
	"spruce_leaves_red",//51
	"spruce_log",		//52
	"spruce_log_top",	//53
	"glowstone",		//54
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

	//jungle_logs
	31,31,32,32,31,31,

	//jungle_leaves
	33,33,33,33,33,33,

	//palm_log
	34,34,35,35,34,34,

	//palm_leaves
	36,36,36,36,36,36,

	//cactus bud
	37,37,37,37,37,37,

	//dead bush
	38,38,38,38,38,38,

	//jungle_planks
	39,39,39,39,39,39,

	//clay
	40,40,40,40,40,40,

	//hardened_clay
	41, 41, 41, 41, 41, 41,

	//mud
	42, 42, 42, 42, 42, 42,

	//packed_mud
	43, 43, 43, 43, 43, 43,
	
	//mud_bricks
	44, 44, 44, 44, 44, 44,

	//control1
	45, 45, 45, 45, 45, 45,

	//control2
	46, 46, 46, 46, 46, 46,

	//control3
	47, 47, 47, 47, 47, 47,

	//control4
	48, 48, 48, 48, 48, 48,

	//snow_block
	16,16,16,16,16,16,

	//birch_leaves
	49, 49, 49, 49, 49, 49,

	//spruce_log
	52, 52, 53, 53, 52, 52,

	//spruce_leaves
	50, 50, 50, 50, 50, 50,
	
	//spruce_leaves_red
	51, 51, 51, 51, 51, 51,

	//glowstone
	54, 54, 54, 54, 54, 54,
};

bool fixAlpha(unsigned char *buffer, int w, int h)
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

	auto notBlack = [&](int x, int y)
	{
		auto c = sample(x, y);

		return c.r != 0 && c.g != 0 && c.b != 0;
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
				set(x, y, {0,0,0});
			}
		}

	bool changed = 0;
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
		{

			if (sampleA(x, y) == 0)
			{

				if (x < w - 1)
				{
					auto c = sample(x + 1, y);
					if (glm::ivec3(c) != glm::ivec3{0,0,0})
					{
						set(x, y, c);
						changed = true;
						continue;
					}
				}

				if (y < h - 1)
				{
					auto c = sample(x, y + 1);
					if (glm::ivec3(c) != glm::ivec3{0,0,0})
					{
						set(x, y, c);
						changed = true;
						continue;
					}
				}

				if (x < w - 1 && y < h - 1)
				{
					auto c = sample(x + 1, y + 1);
					if (glm::ivec3(c) != glm::ivec3{0,0,0})
					{
						set(x, y, c);
						changed = true;
						continue;
					}
				}

				if (x > 0 && y < h - 1)
				{
					auto c = sample(x - 1, y + 1);
					if (glm::ivec3(c) != glm::ivec3{0,0,0})
					{
						set(x, y, c);
						changed = true;
						continue;
					}
				}

				if (x > 0)
				{
					auto c = sample(x - 1, y);
					if (glm::ivec3(c) != glm::ivec3{0,0,0})
					{
						set(x, y, c);
						changed = true;
						continue;
					}
				}

				if (y > 0)
				{
					auto c = sample(x, y - 1);
					if (glm::ivec3(c) != glm::ivec3{0,0,0})
					{
						set(x, y, c);
						changed = true;
						continue;
					}
				}

				if (x > 0 && y > 0)
				{
					auto c = sample(x - 1, y - 1);
					if (glm::ivec3(c) != glm::ivec3{0,0,0})
					{
						set(x, y, c);
						changed = true;
						continue;
					}
				}

				if (x < w - 1 && y > 0)
				{
					auto c = sample(x + 1, y - 1);
					if (glm::ivec3(c) != glm::ivec3{0,0,0})
					{
						set(x, y, c);
						changed = true;
						continue;
					}
				}

			}

		}

	return changed;
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

	for (int i = 0; i < 64; i++)
	{
		fixAlpha(decodedImage, width, height);
	}
	

	t.createFromBuffer((const char *)decodedImage, width, height, pixelated, useMipMaps);

	//Replace stbi allocators
	free((void *)decodedImage);
}

bool loadFromFileWithAplhaFixing(gl2d::Texture &t, const char *fileName, bool pixelated, bool useMipMaps)
{
	std::ifstream file(fileName, std::ios::binary);

	if (!file.is_open())
	{
		char c[300] = {0};
		strcat(c, "error openning: ");
		strcat(c + strlen(c), fileName);
		std::cout << c << "\n";
		//errorFunc(c);//todo propper error function
		return 0;
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

	return 1;
}

//textureloader texture loader
void BlocksLoader::loadAllTextures()
{
	size_t count = sizeof(texturesNames) / sizeof(char *);
	
	gpuIds.reserve(count + 1);
	texturesIds.reserve(count+ 1);

	//default texture
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

	//default normal
	{
		unsigned char data[4] = {};

		{
			int i = 0;
			data[i++] = 127;
			data[i++] = 127;
			data[i++] = 255;
			data[i++] = 255;
		}

		gl2d::Texture t;
		t.createFromBuffer((char *)data, 1, 1, true, false);

		texturesIds.push_back(t.id);
		auto handle = glGetTextureHandleARB(t.id);
		glMakeTextureHandleResidentARB(handle);
		gpuIds.push_back(handle);
	}

	//default material
	{
		unsigned char data[4] = {};

		{
			int i = 0;
			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 255;
		}

		gl2d::Texture t;
		t.createFromBuffer((char *)data, 1, 1, true, false);
		texturesIds.push_back(t.id);
		auto handle = glGetTextureHandleARB(t.id);
		glMakeTextureHandleResidentARB(handle);
		
		gpuIds.push_back(handle);
	}
	
	std::string path;
	std::string path2;

	auto addTexture = [&](std::string path) -> bool
	{

		gl2d::Texture t;
		
		if (!loadFromFileWithAplhaFixing(t, path.c_str(), true, false)) { return 0; }
		t.bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 6.f);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 6.f);

		glGenerateMipmap(GL_TEXTURE_2D);

		texturesIds.push_back(t.id);

		auto handle = glGetTextureHandleARB(t.id);
		glMakeTextureHandleResidentARB(handle);

		gpuIds.push_back(handle);

		return 1;
	};

	for (int i = 0; i < count; i++)
	{
		
		path.clear();
		path2.clear();

		path += RESOURCES_PATH;
		path2 += RESOURCES_PATH;
		
		path2 += "pbr/block/";
		//path += "pbr3/";
		path += "pbr/block/"; //original
		
		path += texturesNames[i];
		path2 += texturesNames[i];

		if (!addTexture(path + ".png"))
		{
			if (!addTexture(path2 + ".png"))
			{
				texturesIds.push_back(texturesIds[0]);
				gpuIds.push_back(gpuIds[0]);
			}
		}
		
		if(!addTexture(path + "_n.png"))
		{
			if (!addTexture(path2 + "_n.png"))
			{
				texturesIds.push_back(texturesIds[1]);
				gpuIds.push_back(gpuIds[1]);
			}
		}

		if (!addTexture(path + "_s.png"))
		{
			if (!addTexture(path2 + "_s.png"))
			{
				texturesIds.push_back(texturesIds[2]);
				gpuIds.push_back(gpuIds[2]);
			}
		}
	}
	

}

uint16_t getGpuIdIndexForBlock(short type, int face)
{
	return blocksLookupTable[type * 6 + face] * 3;
}

#include <blocksLoader.h>
#include "blocks.h"
#include <fstream>
#include <iostream>
#include <errorReporting.h>
#include <gameplay/items.h>
#include <platformTools.h>

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
	"glass",			//55
	"",					//56 test texture
	"torch",			//57
	"crafting_table_front",	//58
	"crafting_table_side",	//59
	"crafting_table_top",	//60
	"gravel-dirt-bloc",		//61
	"birch_planks",		//62
	"black_stained_glass",	//63
	"gray_stained_glass",	//64
	"light_gray_stained_glass",	//65
	"white_stained_glass",	//66
	"brown_stained_glass",	//67
	"red_stained_glass",	//68
	"orange_stained_glass",	//69
	"yellow_stained_glass",	//70
	"lime_stained_glass",	//71
	"green_stained_glass",	//72
	"cyan_stained_glass",	//73
	"light_blue_stained_glass",	//74
	"blue_stained_glass",	//75
	"purple_stained_glass",	//76
	"magenta_stained_glass",//77
	"pink_stained_glass",	//78
	"white_wool",		//79

};

//front
//back
//top
//bottom
//leftz
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
	45, 46, 46, 46, 47, 48,

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

	//glass
	55, 55, 55, 55, 55, 55,

	//test texture
	56, 56, 56, 56, 56, 56,

	//torch
	57, 57, 57, 57, 57, 57,

	//crafting table
	58, 58, 60, 7, 59, 59, 

	//white_wool
	61,61,61,61,61,61,

	//birch_planks
	62,62,62,62,62,62,

	//glasses
	63, 63, 63, 63, 63, 63,
	64, 64, 64, 64, 64, 64,
	65, 65, 65, 65, 65, 65,
	66, 66, 66, 66, 66, 66,
	67, 67, 67, 67, 67, 67,
	68, 68, 68, 68, 68, 68,
	69, 69, 69, 69, 69, 69,
	70, 70, 70, 70, 70, 70,
	71, 71, 71, 71, 71, 71,
	72, 72, 72, 72, 72, 72,
	73, 73, 73, 73, 73, 73,
	74, 74, 74, 74, 74, 74,
	75, 75, 75, 75, 75, 75,
	76, 76, 76, 76, 76, 76,
	77, 77, 77, 77, 77, 77,
	78, 78, 78, 78, 78, 78,

	//whool
	79,79,79,79,79,79,

	//wooden_stairs
	7, 7, 7, 7, 7, 7,

	//wooden_slab
	7, 7, 7, 7, 7, 7,

};

void fixAlphaForNormals(unsigned char *buffer, int w, int h)
{
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
			if (sampleA(x, y) <= 0)
			{
				set(x, y, {127,127,255});
			}
		}
}

bool fixAlpha(unsigned char *buffer, int w, int h, bool firstTime)
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

	auto setA = [&](int x, int y, unsigned a)
	{
		buffer[(x + y * w) * 4 + 3] = a;
	};

	auto sampleA = [&](int x, int y) -> unsigned char
	{
		return buffer[(x + y * w) * 4 + 3];
	};

	if(firstTime)
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
		{
			if (sampleA(x, y) <= 0)
			{
				set(x, y, {0,0,0});
				//setA(x, y, 255);
			}
		}

	bool changed = 0;
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
		{
			auto s = sample(x, y);
			if ( s.a == 0 && (glm::ivec3(s) == glm::ivec3{0,0,0}) )
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

	//preview
	//if (!changed)
	//{
	//	for (int y = 0; y < h; y++)
	//		for (int x = 0; x < w; x++)
	//		{
	//			if (sampleA(x, y) <= 0)
	//			{
	//				setA(x, y, 255);
	//			}
	//		}	
	//}

	return changed;
}

void createFromFileDataWithAplhaFixing(gl2d::Texture &t, const unsigned char *image_file_data, 
	const size_t image_file_size
	, bool pixelated, bool useMipMaps, bool isNormalMap)
{
	stbi_set_flip_vertically_on_load(true);
	
	int width = 0;
	int height = 0;
	int channels = 0;

	unsigned char *decodedImage = stbi_load_from_memory(image_file_data, (int)image_file_size, &width, &height, &channels, 4);

	if (isNormalMap)
	{
		fixAlphaForNormals(decodedImage, width, height);
	}
	else
	{
		for (int i = 0; i < 256; i++)
		{
			if (!fixAlpha(decodedImage, width, height, i == 0))
			{
				break;
			}
		}

	}

	t.createFromBuffer((const char *)decodedImage, width, height, pixelated, useMipMaps);
	

	//Replace stbi allocators
	free((void *)decodedImage);
}

bool loadFromFileWithAplhaFixing(gl2d::Texture &t, 
	const char *fileName, bool pixelated, bool useMipMaps, bool isNormalMap)
{
	std::ifstream file(fileName, std::ios::binary);

	if (!file.is_open())
	{
		char c[300] = {0};
		strcat(c, "error openning: ");
		strcat(c + strlen(c), fileName);
		reportError(c);
		return 0;
	}

	int fileSize = 0;
	file.seekg(0, std::ios::end);
	fileSize = (int)file.tellg();
	file.seekg(0, std::ios::beg);
	unsigned char *fileData = new unsigned char[fileSize];
	file.read((char *)fileData, fileSize);
	file.close();

	createFromFileDataWithAplhaFixing(t, fileData, fileSize, pixelated, useMipMaps, isNormalMap);

	delete[] fileData;

	return 1;
}

//https://stackoverflow.com/questions/15095909/from-rgb-to-hsv-in-opengl-glsl
glm::vec3 rgbToHSV(glm::vec3 c)
{
	glm::vec4 K = glm::vec4(0.0f, -1.0f / 3.0f, 2.0f / 3.0f, -1.0f);
	glm::vec4 p = glm::mix(glm::vec4(c.b, c.g, K.w, K.z), glm::vec4(c.g, c.b, K.x, K.y), glm::step(c.b, c.g));
	glm::vec4 q = glm::mix(glm::vec4(glm::vec3(p.x, p.y, p.w), c.r), glm::vec4(c.r, p.y, p.z, p.x), glm::step(p.x, c.r));

	float d = q.x - glm::min(q.w, q.y);
	float e = 1.0e-10;
	return glm::vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

glm::vec3 hsv2rgb(glm::vec3 c)
{
	glm::vec4 K = glm::vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	glm::vec3 p = abs(glm::fract(glm::vec3(c.x, c.x, c.x) + glm::vec3(K.x, K.y, K.z)) 
		* 6.0f - glm::vec3(K.w, K.w, K.w));
	return c.z * glm::mix(glm::vec3(K.x, K.x, K.x), glm::clamp(p - glm::vec3(K.x, K.x, K.x), 0.0f, 1.0f), c.y);
}

//textureloader texture loader
void BlocksLoader::loadAllTextures(std::string filePath)
{
	
	if (!backgroundTexture.id)
	{
		std::string path;
		path = filePath + "blocks/";
		path += texturesNames[0];
		path += ".png";

		backgroundTexture.loadFromFile(path.c_str(), true, true);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	
	
	bool appendMode = gpuIds.empty();

	size_t count = sizeof(texturesNames) / sizeof(char *);
	 
	if (appendMode)
	{
		gpuIds.reserve(count + 1);
		texturesIds.reserve(count + 1);
	};


	if (appendMode)
	{
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
			t.createFromBuffer((char *)data, 2, 2, true, false);

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
	};

	std::string path;

	auto setGpuIds = [&](int blockIndex) 
	{
		auto handle = glGetTextureHandleARB(texturesIds[blockIndex + 0]);
		glMakeTextureHandleResidentARB(handle);
		gpuIds[blockIndex + 0] = handle;

		handle = glGetTextureHandleARB(texturesIds[blockIndex + 1]);
		glMakeTextureHandleResidentARB(handle);
		gpuIds[blockIndex + 1] = handle;

		handle = glGetTextureHandleARB(texturesIds[blockIndex + 2]);
		glMakeTextureHandleResidentARB(handle);
		gpuIds[blockIndex + 2] = handle;
	};

	auto addTexture = [&](int index, std::string path, bool isNormalMap = 0) -> bool
	{

		gl2d::Texture t;
		
		if (!loadFromFileWithAplhaFixing(t, path.c_str(), true, false, isNormalMap)) { return 0; }
		t.bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		//todo compare
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.f);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 4.f);

		glGenerateMipmap(GL_TEXTURE_2D);



		auto handle = glGetTextureHandleARB(t.id);
		glMakeTextureHandleResidentARB(handle);


		if (appendMode)
		{
			texturesIds.push_back(t.id);
			gpuIds.push_back(handle);
		}
		else
		{
			texturesIds[index] = t.id;
			gpuIds[index] = handle;
		}

		return 1;
	};

	for (int i = 0; i < count; i++)
	{
		
		if (!appendMode && texturesIds[(i+1)*3] != texturesIds[0])
		{
			continue;
		}

		path = filePath + "blocks/";
		path += texturesNames[i];

		
		if (!texturesNames[i][0] || !addTexture((i+1) * 3, path + ".png"))
		{
			if (appendMode)
			{
				texturesIds.push_back(texturesIds[0]);
				gpuIds.push_back(gpuIds[0]);
			};
		}
		
		if (0) //no normals
		{
			if (appendMode)
			{
				texturesIds.push_back(texturesIds[1]);
				gpuIds.push_back(gpuIds[1]);
			}
		}
		else
		{
			if (!texturesNames[i][0] || !addTexture((i+1) * 3 + 1, path + "_n.png", true))
			{
				if (appendMode)
				{
					texturesIds.push_back(texturesIds[1]);
					gpuIds.push_back(gpuIds[1]);
				};
			}
		}

		if (!texturesNames[i][0] || !addTexture((i+1) * 3 + 2, path + "_s.png"))
		{
			if (appendMode)
			{
				texturesIds.push_back(texturesIds[2]);
				gpuIds.push_back(gpuIds[2]);
			}
		}


	}
	
	auto copyFromOtherTexture = [&](int desinationIndex, int sourceIndex)
	{
		texturesIds[desinationIndex + 0] = texturesIds[sourceIndex + 0];
		texturesIds[desinationIndex + 1] = texturesIds[sourceIndex + 1];
		texturesIds[desinationIndex + 2] = texturesIds[sourceIndex + 2];
	};

	auto applyModifications = [&](std::vector<unsigned char> &data,
		glm::ivec2 size, glm::vec3(*func)(glm::vec3))
	{
		for (int i = 0; i < size.x * size.y; i++)
		{
			glm::vec3 color = {};

			color.r = data[i * 4 + 0] / 255.f;
			color.g = data[i * 4 + 1] / 255.f;
			color.b = data[i * 4 + 2] / 255.f;

			color = func(color);

			data[i * 4 + 0] = color.r * 255;
			data[i * 4 + 1] = color.g * 255;
			data[i * 4 + 2] = color.b * 255;
		}
	};

	auto copyFromOtherTextureAndApplyModifications = [&](int desinationIndex, int sourceIndex,
		glm::vec3(*func)(glm::vec3))
	{
		copyFromOtherTexture(desinationIndex, sourceIndex);

		gl2d::Texture t; t.id = texturesIds[desinationIndex + 0];
		glm::ivec2 size = {};
		auto data = t.readTextureData(0, &size);

		applyModifications(data, size, func);

		t.createFromBuffer((char *)data.data(), size.x, size.y, true, true);
		texturesIds[desinationIndex + 0] = t.id;

		t.bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.f);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 4.f);
		glGenerateMipmap(GL_TEXTURE_2D);

		setGpuIds(desinationIndex);
	};

	//generate other textures
	{

		//test texture

		if(gpuIds[getGpuIdIndexForBlock(BlockTypes::diamond_ore, 0)] != gpuIds[0])
		copyFromOtherTextureAndApplyModifications(56 * 3, 
			getGpuIdIndexForBlock(BlockTypes::diamond_ore, 0),
			[](glm::vec3 in) ->glm::vec3
		{
			
			//in = glm::pow(in, glm::vec3(1.5f, 1.5f, 0.5f));

			//in = glm::pow(in, glm::vec3(0.5f));


			in = rgbToHSV(in);
			
			//if (in.g > 0.2)
			//{
			//	in.g = 0;
			//	in.b = std::sqrt(in.b);
			//}
			//else
			//{
			//	in.b = std::pow(in.b, 1.5);
			//}

			in.x += 0.3;
			in.x = glm::fract(in.x);
			
			in = hsv2rgb(in);
			
			return in;
		});
	}

	//generate normal maps
	{

		for (int i = 1; i < texturesIds.size() / 3; i++)
		{

			if (texturesIds[i * 3 + 1] == texturesIds[1] && texturesIds[i*3] != texturesIds[0])
			{
				//no normal map!, but the block is loaded

				gl2d::Texture t; t.id = texturesIds[i*3 + 0];
				glm::ivec2 size = {};
				auto data = t.readTextureData(0, &size);

				std::vector<float> dataGrayScale;
				dataGrayScale.resize(data.size() / 4);

				for (int i = 0; i < data.size() / 4; i++)
				{
					glm::vec3 color = {};
					color.r = data[i * 4 + 0] / 255.f;
					color.g = data[i * 4 + 1] / 255.f;
					color.b = data[i * 4 + 2] / 255.f;
					float luminosity = glm::dot(color, {0.21,0.71,0.07});
					dataGrayScale[i] = luminosity;
				}

				constexpr int magnify = 1;
				data.clear();
				data.resize(dataGrayScale.size() * 4 * magnify * magnify);

				float pixelSize = (1.f / size.x) * 2.2f;
				//the last constant represents the height of the normal map result, hence the strength, smaller constant stronger normal

				auto sample = [&](float u, float v)
				{
					int i = u * size.x;
					int j = v * size.y;
						
					return dataGrayScale[i + j * size.x];
				};

				for (int y = 0; y < size.y * magnify; y++)
					for (int x = 0; x < size.x * magnify; x++)
					{
						glm::vec3 color = {};

						//int iRight = std::min((x + 1), size.x - 1) + y * size.x;
						//int iDown = x + std::min(y + 1, size.y - 1) * size.x;
						//
						//float h = dataGrayScale[i];
						//float hRight = dataGrayScale[iRight];
						//float hDown = dataGrayScale[iDown];

						float h = sample((float)x / (size.x * magnify), (float)y / (size.y * magnify));
						float hRight = sample((float)std::min(x+1, size.x*magnify-1) / (float)(size.x * magnify), (float)y / (size.y * magnify));
						float hDown = sample((float)x / (size.x * magnify), (float)std::min(y + 1, size.y * magnify - 1) / (float)(size.y * magnify));

						glm::vec3 vertex(0, 0, h);
						glm::vec3 vertexR(pixelSize, 0, hRight);
						glm::vec3 vertexD(0, -pixelSize, hDown);

						glm::vec3 R = glm::normalize(vertexR - vertex);
						glm::vec3 D = glm::normalize(vertexD - vertex);

						glm::vec3 N = glm::normalize(glm::cross(D, R));

						//from [-1 1] to [0 255]

						//N = {0,1,0};

						N += glm::vec3(1.f);
						N /= 2.f;
						color = N;

						int i = x + y * (size.x * magnify);

						data[i * 4 + 0] = color.r * 255;
						data[i * 4 + 1] = color.g * 255;
						data[i * 4 + 2] = color.b * 255;
						data[i * 4 + 3] = 255;

					}

				t.createFromBuffer((char *)data.data(), size.x * magnify, size.y * magnify, true, true);
				texturesIds[i * 3 + 1] = t.id;

				t.bind();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 6.f);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 4.f);
				glGenerateMipmap(GL_TEXTURE_2D);

				auto handle = glGetTextureHandleARB(texturesIds[i*3 + 1]);
				glMakeTextureHandleResidentARB(handle);
				gpuIds[i * 3 + 1] = handle;

			}

		}

	}



	//load items
	for (int i = ItemsStartPoint; i < lastItem; i++)
	{

		if (!appendMode && texturesIdsItems[i - ItemsStartPoint] != texturesIds[0])
		{ continue; }

		const char *itemName = getItemTextureName(i);

		if (itemName == std::string(""))
		{
			if (appendMode)
			{
				texturesIdsItems.push_back(texturesIds[0]);
				gpuIdsItems.push_back(gpuIds[0]);
			};
		}
		else
		{
			path = filePath;
			path += "items/";
			path += itemName;

			gl2d::Texture t;

			if (!loadFromFileWithAplhaFixing(t, path.c_str(), true, false, false))
			{
				if (appendMode)
				{
					texturesIdsItems.push_back(texturesIds[0]);
					gpuIdsItems.push_back(gpuIds[0]);
				};
			}
			else
			{
				t.bind();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

				glGenerateMipmap(GL_TEXTURE_2D);


				auto handle = glGetTextureHandleARB(t.id);
				glMakeTextureHandleResidentARB(handle);

				if (appendMode)
				{
					texturesIdsItems.push_back(t.id);
					gpuIdsItems.push_back(handle);
				}
				else
				{
					texturesIdsItems[i - ItemsStartPoint] = t.id;
					gpuIdsItems[i - ItemsStartPoint] = handle;
				}
			}
		}

		
	}


	if (!spawnEgg.id)
		spawnEgg.loadFromFile((filePath + "items/spawn_egg.png").c_str(), true, false);

	if (!spawnEggOverlay.id)
		spawnEggOverlay.loadFromFile((filePath + "items/spawn_egg_overlay.png").c_str(), true, false);

	glm::ivec2 spawnEggSize = {};
	auto spawnEggData = spawnEgg.readTextureData(0, &spawnEggSize);

	glm::ivec2 spawnEggOverSize = {};
	auto spawnEggOverData = spawnEggOverlay.readTextureData(0, &spawnEggOverSize);


	auto createSpawnEggTexture = [&](int index, glm::vec3 baseColod, glm::vec3 topColor)
	{
		index -= ItemsStartPoint;

		auto newBuffer = spawnEggData;
		
		for (int i = 0; i < newBuffer.size(); i+=4)
		{

			unsigned char r = newBuffer[i + 0];
			unsigned char g = newBuffer[i + 1];
			unsigned char b = newBuffer[i + 2];

			unsigned char r2 = spawnEggOverData[i + 0];
			unsigned char g2 = spawnEggOverData[i + 1];
			unsigned char b2 = spawnEggOverData[i + 2];
			unsigned char a2 = spawnEggOverData[i + 3];

			glm::vec3 color1(r, g, b); color1 /= 255.f;
			glm::vec3 color2(r2, g2, b2); color2 /= 255.f;

			glm::vec3 finalColor = {};
			if (a2)
			{
				finalColor = color2 * topColor;
			}
			else
			{
				finalColor = color1 * baseColod;
			}

			finalColor = glm::min(finalColor, glm::vec3(1.f));

			newBuffer[i + 0] = finalColor.r * 255;
			newBuffer[i + 1] = finalColor.g * 255;
			newBuffer[i + 2] = finalColor.b * 255;

		}

		gl2d::Texture newTexture;
		newTexture.createFromBuffer((const char *)newBuffer.data(),
			spawnEggSize.x, spawnEggSize.y, true, false);

		texturesIdsItems[index] = newTexture.id;
		auto handle = glGetTextureHandleARB(newTexture.id);
		glMakeTextureHandleResidentARB(handle);
		gpuIdsItems[index] = handle;


	};

		
	if (spawnEggSize == spawnEggOverSize && spawnEggSize.x != 0 && spawnEggSize.y != 0)
	{
		if (texturesIdsItems[zombieSpawnEgg - ItemsStartPoint] == texturesIds[0])
		{
			createSpawnEggTexture(zombieSpawnEgg, glm::vec3{2, 161, 160} / 255.f, glm::vec3{107, 137, 89} / 255.f);
			createSpawnEggTexture(pigSpawnEgg, glm::vec3(230, 151, 167) / 255.f, glm::vec3(148, 92, 95) / 255.f);
			createSpawnEggTexture(catSpawnEgg, glm::vec3(230, 230, 230) / 255.f, glm::vec3(10, 10, 10) / 255.f);


		};
	}
	else
	{
		spawnEgg.cleanup();
		spawnEggOverlay.cleanup();
		//todo error
		//std::cout << "err\n";
	}

}

void BlocksLoader::setupAllColors()
{
	blocksColors.resize(BlockTypes::BlocksCount);

	for (int i = 0; i < BlockTypes::BlocksCount; i++)
	{
		
		int idIndex = getGpuIdIndexForBlock(i, 2);

		gl2d::Texture t;
		t.id = texturesIds[idIndex];

		int mipmap = 0;
		//glBindTexture(GL_TEXTURE_2D, t.id);
		//glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, &mipmap);
		//mipmap += 1;
		//


		glm::ivec2 size = {};
		auto data = t.readTextureData(mipmap, &size);

		glm::vec3 color = {};
		int divizor = 0;

		for (int y = 0; y < size.y; y++)
			for (int x = 0; x < size.x; x++)
			{
				glm::vec4 c = {};
				c.r = data[(x + y * size.x) * 4 + 0] / 255.f;
				c.g = data[(x + y * size.x) * 4 + 1] / 255.f;
				c.b = data[(x + y * size.x) * 4 + 2] / 255.f;
				c.a = data[(x + y * size.x) * 4 + 3] / 255.f;

				if (c.a > 0.1)
				{
					color += glm::vec3(c);
					divizor++;
				}

			}

		if (divizor)
		{
			color /= divizor;
		}

		blocksColors[i] = color;
	}
}

void BlocksLoader::clearAllTextures()
{
	blocksColors.clear();

	spawnEgg.cleanup();
	
	spawnEggOverlay.cleanup();

	for (int i = 1; i < texturesIds.size()/3; i++)
	{
		if (texturesIds[i * 3] != texturesIds[0])
		{
			glDeleteTextures(1, &texturesIds[i * 3]);
		}

		if (texturesIds[i * 3 + 1] != texturesIds[1])
		{
			glDeleteTextures(1, &texturesIds[i * 3 +1]);
		}

		if (texturesIds[i * 3 + 2] != texturesIds[2])
		{
			glDeleteTextures(1, &texturesIds[i * 3 + 2]);
		}
	}

	for (auto &t : texturesIdsItems)
	{
		if (t != texturesIds[0])
		{
			glDeleteTextures(1, &t);
		}
	}

	glDeleteTextures(3, &texturesIds[0]);
	texturesIds.clear();
	gpuIds.clear();

	texturesIdsItems.clear();
	gpuIdsItems.clear();

	backgroundTexture.cleanup();
}

void BlocksLoader::loadAllItemsGeometry()
{
	if (itemsGeometry.size())
	{
		for (int i = 1; i < itemsGeometry.size(); i++)
		{
			if (itemsGeometry[0].vao != itemsGeometry[i].vao)
			{
				itemsGeometry[i].clear();
			}
		}
		itemsGeometry[0].clear();
		itemsGeometry.clear();
	}

	std::vector<float> finalData;
	

	auto loadItem = [&](gl2d::Texture &t)
	{
		permaAssertComment(t.id, "recieved no texture id in loadItem in blocks loader");

		glm::ivec2 size = {};
		auto textureData = t.readTextureData(0, &size);

		finalData.clear();
		finalData.reserve(100);

		ItemGeometry result = {};

		glGenVertexArrays(1, &result.vao);
		glBindVertexArray(result.vao);
		glGenBuffers(1, &result.buffer);
		glBindBuffer(GL_ARRAY_BUFFER, result.buffer);

		float thick = 1.f / 8;
		//float thick = 2;

		float dataStart[] = {
			1, 1, thick/2.f,	0, 0, 1,	1, 1,
			-1, 1, thick/2.f,	0, 0, 1,	0, 1,
			-1, -1, thick/2.f,	0, 0, 1,	0, 0,

			-1, -1, thick/2.f,	0, 0, 1,	0, 0,
			1, -1, thick/2.f,	0, 0, 1,	1, 0,
			1, 1, thick/2.f,	0, 0, 1,	1, 1,

			-1, 1, -thick/2.f,	0, 0, -1,	0, 1,
			1, 1, -thick/2.f,	0, 0, -1,	1, 1,
			-1, -1, -thick/2.f,	0, 0, -1,	0, 0,

			1, -1, -thick/2.f,	0, 0, -1,	1, 0,
			-1, -1, -thick/2.f,	0, 0, -1,	0, 0,
			1, 1, -thick/2.f,	0, 0, -1,	1, 1,
		};
		
		for (int i = 0; i < sizeof(dataStart) / sizeof(float); i++)
		{
			finalData.push_back(dataStart[i]);
		}

		auto unsafeGet = [&](int i, int j)
		{
			j = (size.y) - j - 1;
			glm::vec4 rez = {};
			rez.r = textureData[(i + j * size.x) * 4 + 0];
			rez.g = textureData[(i + j * size.x) * 4 + 1];
			rez.b = textureData[(i + j * size.x) * 4 + 2];
			rez.a = textureData[(i + j * size.x) * 4 + 3];
			return rez;
		};

		auto unsafeGet2 = [&](int i, int j)
		{
			i = (size.x) - i - 1;
			j = (size.y) - j - 1;
			glm::vec4 rez = {};
			rez.r = textureData[(i + j * size.x) * 4 + 0];
			rez.g = textureData[(i + j * size.x) * 4 + 1];
			rez.b = textureData[(i + j * size.x) * 4 + 2];
			rez.a = textureData[(i + j * size.x) * 4 + 3];
			return rez;
		};

		//top face
		for (int y = 0; y < size.y; y++)
		{
			for (int x = 0; x < size.x; x++)
			{
				auto color = unsafeGet(x, y);
				if (color.a > 1)
				{
					if (y != 0)
					{
						auto color = unsafeGet(x, y - 1);
						if (color.a > 1) { continue; }
					}

					int start = x;
					int end = x;

					for (x = x+1; x < size.x; x++)
					{
						auto color = unsafeGet(x, y);
						if (color.a <= 1) { break; }

						if (y != 0)
						{
							auto color = unsafeGet(x, y-1);
							if (color.a > 1) { break; }
						}

						end = x;
					}
					end++;

					float height = 1.f + (-2.f) * ((float)y / size.y);

					float textureBotton = (1 + (-1.f) * (((float)y + 1) / size.y));
					float textureTop = (1 + (-1.f) * ((float)y / size.y));

					float xStart = -1 + 2 * (start / (float)size.x);
					float xEnd = -1 + 2 * (end / (float)size.x);

					float textureLeft = 0 + 1 * (start / (float)size.x);
					float textureRight = 0 + 1 * (end / (float)size.x);

					float dataTop[] = {
						xEnd, height, thick / 2.f,	0, 1, 0,	textureRight, textureBotton,
						xEnd, height, -thick / 2.f,0, 1, 0,		textureRight, textureTop,
						xStart, height, -thick / 2.f,0, 1, 0,	textureLeft, textureTop,

						xStart, height, -thick / 2.f,0, 1, 0,	textureLeft, textureTop,
						xStart, height, thick / 2.f,0, 1, 0,	textureLeft, textureBotton,
						xEnd, height, thick / 2.f,	0, 1, 0,	textureRight, textureBotton,
					};

					for (int i = 0; i < sizeof(dataTop) / sizeof(float); i++)
					{
						finalData.push_back(dataTop[i]);
					}

				}

			}

		}


		//bottom face
		for (int y = 0; y < size.y; y++)
		{
			for (int x = 0; x < size.x; x++)
			{

				auto color = unsafeGet(x, y);
				if (color.a > 1)
				{
					if (y != size.y - 1)
					{
						auto color = unsafeGet(x, y + 1);
						if (color.a > 1) { continue; }
					}

					
					int start = x;
					int end = x;

					for (x = x + 1; x < size.x; x++)
					{
						auto color = unsafeGet(x, y);
						if (color.a <= 1) { break; }

						if (y != size.y-1)
						{
							auto color = unsafeGet(x, y + 1);
							if (color.a > 1) { break; }
						}

						end = x;
					}
					end++;

					float height = 1.f + (-2.f) * ((float)(y+1.f) / size.y);

					float textureBotton = (1 + (-1.f) * (((float)y + 1) / size.y));
					float textureTop = (1 + (-1.f) * ((float)y / size.y));

					float xStart = -1 + 2 * (start / (float)size.x);
					float xEnd = -1 + 2 * (end / (float)size.x);

					float textureLeft = 0 + 1 * (start / (float)size.x);
					float textureRight = 0 + 1 * (end / (float)size.x);

					float dataTop[] = {
						xEnd, height, thick / 2.f,	0, 1, 0,	textureRight, textureBotton,
						xStart, height, -thick / 2.f,0, 1, 0,	textureLeft, textureTop,
						xEnd, height, -thick / 2.f,0, 1, 0,		textureRight, textureTop,

						xStart, height, -thick / 2.f,0, 1, 0,	textureLeft, textureTop,
						xEnd, height, thick / 2.f,	0, 1, 0,	textureRight, textureBotton,
						xStart, height, thick / 2.f,0, 1, 0,	textureLeft, textureBotton,

					};

					for (int i = 0; i < sizeof(dataTop) / sizeof(float); i++)
					{
						finalData.push_back(dataTop[i]);
					}

				}

			}

		}
		
		//left face
		for (int x = 0; x < size.x; x++)
		{
			for (int y = 0; y < size.y; y++)
			{

				auto color = unsafeGet(x, y);
				if (color.a > 1)
				{
					if (x != 0)
					{
						auto color = unsafeGet(x-1, y);
						if (color.a > 1) { continue; }
					}

					int start = y;
					int end = y;

					for (y = y + 1; y < size.y; y++)
					{
						auto color = unsafeGet(x, y);
						if (color.a <= 1) { break; }

						if (x != 0)
						{
							auto color = unsafeGet(x-1, y);
							if (color.a > 1) { break; }
						}

						end = y;
					}
					end++;

					float slide = -1.f + (2.f) * ((float)x / size.x);

					float textureLeft = 1-(1 + (-1.f) * (((float)x) / size.x));
					float textureRight = 1-(1 + (-1.f) * (((float)x + 1) / size.x));

					float yStart = (-1 + 2 * (start / (float)size.y))*-1;
					float yEnd = (-1 + 2 * (end / (float)size.y))*-1;

					float textureTop = 1 - 1 * (start / (float)size.y);
					float textureBottom = 1 - 1 * (end / (float)size.y);

					float dataTop[] = {
						slide, yEnd, thick / 2.f,	0, 1, 0,	textureRight,textureBottom,
						slide, yStart, -thick / 2.f,0, 1, 0,	textureLeft, textureTop,
						slide, yEnd, -thick / 2.f,0, 1, 0,		textureLeft, textureBottom,

						slide, yStart, -thick / 2.f,0, 1, 0,	textureLeft, textureTop,
						slide, yEnd, thick / 2.f,	0, 1, 0,	textureRight, textureBottom,
						slide, yStart, thick / 2.f,0, 1, 0,	    textureRight, textureTop,

					};

					for (int i = 0; i < sizeof(dataTop) / sizeof(float); i++)
					{
						finalData.push_back(dataTop[i]);
					}

				}

			}

		}


		//right face
		for (int x = 0; x < size.x; x++)
		{
			for (int y = 0; y < size.y; y++)
			{

				auto color = unsafeGet(x, y);
				if (color.a > 1)
				{
					if (x+1 < size.x)
					{
						auto color = unsafeGet(x + 1, y);
						if (color.a > 1) { continue; }
					}

					int start = y;
					int end = y;

					for (y = y + 1; y < size.y; y++)
					{
						auto color = unsafeGet(x, y);
						if (color.a <= 1) { break; }

						if (x+1 < size.x)
						{
							auto color = unsafeGet(x + 1, y);
							if (color.a > 1) { break; }
						}

						end = y;
					}
					end++;

					float slide = -1.f + (2.f) * ((float)(x+1.f) / size.x);

					float textureLeft = 1 - (1 + (-1.f) * (((float)x) / size.x));
					float textureRight = 1 - (1 + (-1.f) * (((float)x + 1) / size.x));

					float yStart = (-1 + 2 * (start / (float)size.y)) * -1;
					float yEnd = (-1 + 2 * (end / (float)size.y)) * -1;

					float textureTop = 1 - 1 * (start / (float)size.y);
					float textureBottom = 1 - 1 * (end / (float)size.y);

					float dataTop[] = {
						slide, yEnd, thick / 2.f,	0, 1, 0,	textureRight,textureBottom,
						slide, yEnd, -thick / 2.f,0, 1, 0,		textureLeft, textureBottom,
						slide, yStart, -thick / 2.f,0, 1, 0,	textureLeft, textureTop,

						slide, yStart, -thick / 2.f,0, 1, 0,	textureLeft, textureTop,
						slide, yStart, thick / 2.f,0, 1, 0,	    textureRight, textureTop,
						slide, yEnd, thick / 2.f,	0, 1, 0,	textureRight, textureBottom,

					};

					for (int i = 0; i < sizeof(dataTop) / sizeof(float); i++)
					{
						finalData.push_back(dataTop[i]);
					}

				}

			}

		}



		glBufferStorage(GL_ARRAY_BUFFER, finalData.size() * sizeof(float),
			finalData.data(), 0);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));

		result.count = finalData.size() / 8;

		itemsGeometry.push_back(result);
	};


	gl2d::Texture defaultTexture;
	defaultTexture.id = texturesIds[0];
	loadItem(defaultTexture);

	for (int i = ItemTypes::stick; i < ItemTypes::lastItem; i++)
	{

		if (texturesIdsItems[i - ItemTypes::stick] == texturesIds[0])
		{
			itemsGeometry.push_back(itemsGeometry[0]);
		}
		else
		{
			gl2d::Texture t;
			t.id = texturesIdsItems[i - ItemTypes::stick];
			loadItem(t);
		}
	}
	glBindVertexArray(0);

}

uint16_t getGpuIdIndexForBlock(short type, int face)
{
	return blocksLookupTable[type * 6 + face] * 3;
}


void BlocksLoader::ItemGeometry::clear()
{
	glDeleteBuffers(1, &buffer);
	glDeleteVertexArrays(1, &vao);
	*this = {};
}

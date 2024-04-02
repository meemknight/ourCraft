#include <blocksLoader.h>
#include "blocks.h"
#include <fstream>
#include <iostream>
#include <errorReporting.h>

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

	//glass
	55, 55, 55, 55, 55, 55,

	//test texture
	56, 56, 56, 56, 56, 56,
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

	auto addTexture = [&](std::string path, bool isNormalMap = 0) -> bool
	{

		gl2d::Texture t;
		
		if (!loadFromFileWithAplhaFixing(t, path.c_str(), true, false, isNormalMap)) { return 0; }
		t.bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		//todo compare
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
		
		path2 += "pbr/block/"; //default
		
		//path += "pbr2/";		//texture pack
		path += "pbr/block/"; //original
		
		path += texturesNames[i];
		path2 += texturesNames[i];

		if (!texturesNames[i] || !addTexture(path + ".png"))
		{
			if (!texturesNames[i] || !addTexture(path2 + ".png"))
			{
				texturesIds.push_back(texturesIds[0]);
				gpuIds.push_back(gpuIds[0]);
			}
		}
		
		if (0) //no normals
		{
			texturesIds.push_back(texturesIds[1]);
			gpuIds.push_back(gpuIds[1]);
		}
		else
		{
			if (!texturesNames[i] || !addTexture(path + "_n.png", true))
			{
				if (!texturesNames[i] || !addTexture(path2 + "_n.png", true))
				{
					texturesIds.push_back(texturesIds[1]);
					gpuIds.push_back(gpuIds[1]);
				}
			}
		}

		

		if (!texturesNames[i] || !addTexture(path + "_s.png"))
		{
			if (!texturesNames[i] || !addTexture(path2 + "_s.png"))
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

		setGpuIds(desinationIndex);
	};

	//generate other textures
	{
		//test texture
		copyFromOtherTextureAndApplyModifications(56 * 3, 
			getGpuIdIndexForBlock(BlockTypes::wooden_plank, 0),
			[](glm::vec3 in) ->glm::vec3
		{
			in = glm::pow(in, glm::vec3(2.f));
			
			//in = rgbToHSV(in);
			//in.x += 0.0;
			//in.x = glm::fract(in.x);
			//in = hsv2rgb(in);
			
			return in;
		});
		


	}



}

uint16_t getGpuIdIndexForBlock(short type, int face)
{
	return blocksLookupTable[type * 6 + face] * 3;
}

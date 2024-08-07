#pragma once
#include <gl2d/gl2d.h>

struct BlocksLoader
{


	std::vector<GLuint64> gpuIds;
	std::vector<GLuint> texturesIds;

	void loadAllTextures(std::string path);
	void clearAllTextures();

	gl2d::Texture spawnEgg;
	gl2d::Texture spawnEggOverlay;

	std::vector<GLuint64> gpuIdsItems;
	std::vector<GLuint> texturesIdsItems;
	

};

uint16_t getGpuIdIndexForBlock(short type, int face);

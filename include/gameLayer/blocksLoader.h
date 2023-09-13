#pragma once
#include <gl2d/gl2d.h>

struct BlocksLoader
{


	std::vector<GLuint64> gpuIds;
	std::vector<GLuint> texturesIds;

	void loadAllTextures();

};

uint16_t getGpuIdIndexForBlock(short type, int face);

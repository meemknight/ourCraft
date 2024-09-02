#pragma once
#include <gl2d/gl2d.h>

struct BlocksLoader
{

	std::vector<GLuint64> gpuIds;
	std::vector<GLuint> texturesIds;

	void loadAllTextures(std::string path);
	void clearAllTextures();

	void loadAllItemsGeometry();

	gl2d::Texture spawnEgg;
	gl2d::Texture spawnEggOverlay;

	std::vector<GLuint64> gpuIdsItems;
	std::vector<GLuint> texturesIdsItems;
	
	struct ItemGeometry
	{
		GLuint vao = 0;
		GLuint buffer = 0;
		unsigned int count = 0;

		void clear();
	};

	std::vector<ItemGeometry> itemsGeometry;

	gl2d::Texture backgroundTexture;

};

uint16_t getGpuIdIndexForBlock(short type, int face);

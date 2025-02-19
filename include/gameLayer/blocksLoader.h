#pragma once
#include <gl2d/gl2d.h>

struct BlocksLoader
{

	std::vector<GLuint64> gpuIds;
	std::vector<GLuint> texturesIds;

	//load all blocks loadallblocks
	void loadAllTextures(std::string path, bool reportErrors);
	void setupAllColors();
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

	std::vector<gl2d::Texture> blockUiTextures;

	std::vector<glm::vec3> blocksColors;

};

uint16_t getGpuIdIndexForBlock(short type, int face);

#define SNOW_GRASS_TEXTURE_INDEX 82
#define YELLOW_GRASS_TEXTURE_INDEX 97
#define BRICKS_VARIATION_TEXTURE_INDEX 99
#define BLUE_BRICKS_VARIATION_TEXTURE_INDEX 100


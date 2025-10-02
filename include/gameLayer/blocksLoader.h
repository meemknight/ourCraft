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
	

	struct ItemGeometry2D
	{
		GLuint vao = 0;
		GLuint buffer = 0;
		unsigned int count = 0;

		void clear();
	};

	struct ItemGeometry3D
	{
		GLuint vao = 0;
		GLuint buffer = 0;
		GLuint indexBuffer = 0;
		unsigned int count = 0;

		GLuint64 gpuId = 0;
		gl2d::Texture texturesId;

		void clear();
	};


	struct FullItemGeometryData
	{
		ItemGeometry2D model2D;

		ItemGeometry3D model3D;

	};


	std::vector<FullItemGeometryData> itemsGeometry;

	gl2d::Texture backgroundTexture;

	std::vector<gl2d::Texture> blockUiTextures;

	std::vector<glm::vec3> blocksColors;

	void clearItemsGeometry();

};

bool loadFromFileAndAddPadding(gl2d::Texture &t, const char *path);


uint16_t getGpuIdIndexForBlock(short type, int face);

#define SNOW_GRASS_TEXTURE_INDEX 82
#define YELLOW_GRASS_TEXTURE_INDEX 97
#define BRICKS_VARIATION_TEXTURE_INDEX 99
#define BLUE_BRICKS_VARIATION_TEXTURE_INDEX 100


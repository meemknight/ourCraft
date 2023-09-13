#include <blocksLoader.h>


const char *texturesNames[] = {
	"grass_block_top",
	"grass_block_side",
	"ice",
	"oak_log",
	"oak_log_top",
	"oak_planks",
	"cobblestone",
	"gold_block",
	"bricks",
	"sand",
	"sandstone",
	"sandstone_bottom",
	"sandstone_top",
	"grass_block_snow",
	"snow",
	"dark_oak_leaves",
	"gold_ore",
	"coal_ore",
	"stone_bricks",
	"iron_ore",
	"diamond_ore",
	"bookshelf",
	"birch_log",
	"birch_log_top",
	"gravel",
	"grass", //herbs
	"rose",
};


void BlocksLoader::loadAllTextures()
{
	size_t count = sizeof(texturesNames) / sizeof(char *);
	
	gpuIds.reserve(count);
	texturesIds.reserve(count);

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

GLuint64 BlocksLoader::getGpuIdForBlock(short type, int face)
{
	return GLuint64();
}

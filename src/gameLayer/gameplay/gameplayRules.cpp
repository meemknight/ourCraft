#include <gameplay/gameplayRules.h>



bool canBlockBePlaced(BlockType newBlock, BlockType oldBlock)
{

	if (!isBlock(newBlock)) { return false; }


	return true;

}

bool canBlockBeBreaked(BlockType oldBlock, bool isCreative)
{

	return true;
}

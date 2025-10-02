#include "blockUpdates.h"

NieghbourChangeUpdateResult blockNieghbourChangeUpdate(Block in,
	std::optional<Block> front, std::optional<Block> back,
	std::optional<Block> top, std::optional<Block> bottom,
	std::optional<Block> left, std::optional<Block> right)
{

	NieghbourChangeUpdateResult rez;
	rez.newBlockType = in;

	//DISABLE
	return rez;

	if (in.isWallMountedOrStangingBlock())
	{

		bool standingFlag = !in.getRotatedOrStandingForWallOrStandingBlocks();

		if (standingFlag)
		{
			if (bottom && bottom->getType() == 0)
			{
				rez.newBlockType.setType(0);
				rez.shouldDropCurrentBlock = true;
			}
		}
		else
		{
			int r = in.getRotationFor365RotationTypeBlocks();

			if (r == 0)
			{
				if (back && back->getType() == 0)
				{
					rez.newBlockType.setType(0);
					rez.shouldDropCurrentBlock = true;
				}
			}else if (r == 1)
			{
				if (left && left->getType() == 0)
				{
					rez.newBlockType.setType(0);
					rez.shouldDropCurrentBlock = true;
				}
			}
			else if (r == 2)
			{
				if (front && front->getType() == 0)
				{
					rez.newBlockType.setType(0);
					rez.shouldDropCurrentBlock = true;
				}
			}
			else if (r == 3)
			{
				if (right && right->getType() == 0)
				{
					rez.newBlockType.setType(0);
					rez.shouldDropCurrentBlock = true;
				}
			}

		}


	}


	return rez;
}

bool hasBlockNeighbourChangeUpdate(BlockType blockType)
{
	return isWallMountedOrStangingBlock(blockType);
}

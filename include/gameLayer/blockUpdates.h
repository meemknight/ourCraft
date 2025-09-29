#pragma once
#include <blocks.h>
#include <optional>


struct NieghbourChangeUpdateResult
{
	Block newBlockType = {};
	bool shouldDropCurrentBlock = 0;
};


NieghbourChangeUpdateResult blockNieghbourChangeUpdate(Block in,
	std::optional<Block> front, std::optional<Block> back,
	std::optional<Block> top, std::optional<Block> bottom,
	std::optional<Block> left, std::optional<Block> right
);


bool hasBlockNeighbourChangeUpdate(BlockType blockType);




#pragma once
#include <blocks.h>



bool canBlockBePlaced(BlockType newBlock, BlockType oldBlock);

bool canBlockBeBreaked(BlockType oldBlock, bool isCreative);
#pragma once
#include <blocks.h>


bool canBlockBePlaced(BlockType newBlock, BlockType oldBlock);

bool canBlockBeBreaked(BlockType oldBlock, bool isCreative);

const float BASE_HEALTH_REGEN_TIME = 2.0f;
const float BASE_HEALTH_DELAY_TIME = 20.0f;
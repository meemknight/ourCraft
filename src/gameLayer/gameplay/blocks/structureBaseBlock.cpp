#include "gameplay/blocks/structureBaseBlock.h"

void BaseBlock::formatIntoData
(std::vector<unsigned char> &appendTo)
{
	size_t startPos = appendTo.size();
	appendTo.resize(appendTo.size() + sizeof(BaseBlock));

	memcpy(appendTo.data() + startPos, this, sizeof(BaseBlock));
}

bool BaseBlock::readFromBuffer(unsigned char *data, size_t s, size_t &outReadSize)
{
	outReadSize = 0;
	if (s < sizeof(BaseBlock)) { return 0; };
	if (!data) { return 0; }

	memcpy(this, data, sizeof(BaseBlock));

	outReadSize = sizeof(BaseBlock);

	return true;
}


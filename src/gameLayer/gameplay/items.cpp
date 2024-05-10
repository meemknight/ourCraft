#include <gameplay/items.h>

bool Item::isBlock()
{
	return type > 0 && type < BlocksCount;
}

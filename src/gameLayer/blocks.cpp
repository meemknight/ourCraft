#include "blocks.h"
#include "renderer.h"


Block* Chunk::safeGet(int x, int y, int z)
{
	if (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || z >= CHUNK_SIZE || y >= CHUNK_HEIGHT)
	{
		return nullptr;
	}
	else
	{
		return &unsafeGet(x, y, z);
	}
}

void Chunk::bake(std::vector<int>& bakeVector)
{
	//bakeVector.clear();

	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
			for (int y = 0; y < CHUNK_HEIGHT; y++)
			{
				auto &b = unsafeGet(x, y, z);
				if (!b.air())
				{
					auto front = safeGet(x, y, z + 1);
					auto back = safeGet(x, y, z - 1);
					auto top = safeGet(x, y + 1, z);
					auto bottom = safeGet(x, y - 1, z);
					auto left = safeGet(x - 1, y, z);
					auto right = safeGet(x + 1, y, z);

					Block* sides[6] = {front, back, top, bottom, left, right};

					for (int i = 0; i < 6; i++)
					{
						if (sides[i]==nullptr || !(sides[i])->isOpaque())
						{
							bakeVector.push_back(mergeShorts(i, b.type));
							bakeVector.push_back(x + this->x * CHUNK_SIZE);
							bakeVector.push_back(y);
							bakeVector.push_back(z +this->z * CHUNK_SIZE);
						}
					}


				}
			}

}

void Chunk::create(int x, int z)
{
	clear();

	this->x = x;
	this->z = z;

	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
			for (int y = 0; y < 10; y++)
			{
				int calcX = x + this->x * CHUNK_SIZE;
				int calcZ = z + this->z * CHUNK_SIZE;

				if (sin((float)calcX/ CHUNK_SIZE*2) * cos((float)calcZ/CHUNK_SIZE*2) > y/10.f || y == 0)
				{
					unsafeGet(x, y, z).type = BlockTypes::dirt;
				}
			}

	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
		{
			int counter = 0;
			for (int y = CHUNK_HEIGHT - 1; y >= 0; y--)
			{
				if (counter == 0)
				{
					if (!unsafeGet(x, y, z).air())
					{
						unsafeGet(x, y, z).type = BlockTypes::grass;
						counter = 1;
					}
				}
				else if (counter < 4)
				{
					counter++;
				}
				else
				{
					unsafeGet(x, y, z).type = BlockTypes::stone;
				}
			}
		}
}

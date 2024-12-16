#pragma once
#include <gameplay/items.h>
#include <gameplay/weaponStats.h>
#include <rendering/UiEngine.h>



struct HitResult
{
	float hitCorectness = 1;
	float bonusCritChance = 0; // between -1 and 1;
	bool isSwipeAttack = 0;
	bool hit = 0;	//1 if the player pressed the attack button
};



struct BattleUI
{

	void reset();
	
	unsigned short lastItemType = 0;
	char lastInventorySlot = -1;
	bool started = 0;

	HitResult update(Item &item, int inventorySlot, bool dontRun, 
		UiENgine &uiEngine, std::minstd_rand &rng, float deltaTime);

	float timer = 0;

	union
	{

		struct 
		{
			struct Ball
			{
				glm::vec2 position = {};
				glm::vec2 velocity = {};
				char specialBall;
				float dieTimer = 0;
				bool passedCenter = 0;
			};

			static constexpr int MAX_POSITIONS = 6;
			Ball balls[MAX_POSITIONS];
			int currentBallsCount = 0;
		}spearData = {};



	};




};
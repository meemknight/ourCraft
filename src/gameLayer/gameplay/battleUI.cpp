#include <gameplay/battleUI.h>
#include <platform/platformInput.h>
#include <glui/glui.h>
#include <gameplay/entity.h>
#include <imgui.h>

void BattleUI::reset()
{
	*this = {};

}


HitResult BattleUI::update(Item &item, int inventorySlot, bool dontRun,
	UiENgine &ui, std::minstd_rand &rng, float deltaTime)
{

	static float debugSpeed = 1;
	static float debugDexterity = 0;
	static float debugCombo = 0;
	//ImGui::Begin("Test speed");
	//ImGui::SliderFloat("Speed: ", &debugSpeed, -10, 20);
	//ImGui::SliderFloat("Dexterity: ", &debugDexterity, -10, 20);
	//ImGui::SliderFloat("Combo Frequency: ", &debugCombo, -10, 20);
	//ImGui::End();

	HitResult result;

	if (item.type != lastItemType || inventorySlot != lastInventorySlot || dontRun)
	{
		reset();
	}

	if (notDieCount <= 0)
	{
		//reset();
	}
	notDieCount = std::min(notDieCount, 2);

	if (dontRun) { return result; }

	lastItemType = item.type;
	lastInventorySlot = inventorySlot;

	bool leftPressed = platform::isLMousePressed();
	bool rightPressed = platform::isRMousePressed();

	auto &renderer = ui.renderer2d;
	

	glui::Frame f({0, 0, renderer.windowW, renderer.windowH});

	{
	#pragma region set the ui box
		auto box1 = glui::Box().xCenter().yCenter().yDimensionPercentage(0.4).xAspectRatio(1)();
		auto box2 = glui::Box().xCenter().yCenter().xDimensionPercentage(0.4).yAspectRatio(1)();
		if (box2.z < box1.x) { box1 = box2; }
		glui::Frame f(box1);
		auto fullBox = glui::Box().xLeft(0).yTop(0).xDimensionPercentage(1).yDimensionPercentage(1)();
	#pragma endregion

		auto stats = item.getWeaponStats();
		stats.speed = debugSpeed;
		stats.dexterity = debugDexterity;
		stats.comboFrequency = debugCombo;

		float minHitDamage = stats.getAccuracyNormalized() * 0.2;

		if (item.isSpear())
		{

			if (!started)
			{
				if (leftPressed || rightPressed)
				{
					//TODO attack here.
					started = true;

					result.hit = true;
					result.isSwipeAttack = rightPressed;
					result.hitCorectness = 1;

					timer = 0;
					
				}

			}
			else
			{
				timer -= deltaTime;
				float speed = stats.getUIMoveSpeed();
				float dexterity = stats.getDexterityNormalized();
				float comboChance = stats.getComboFrequencyChance();

				if (timer <= 0)
				{
					auto range = stats.getTimerCulldownRangeForAttacks();
					timer = getRandomNumberFloat(rng, range.x, range.y);
					if (spearData.comboCount > 0) 
					{
						timer /= 2.f; 
						rng.seed(spearData.comboSeed);
					}

					if (spearData.currentBallsCount < BattleUI::MAX_POSITIONS)
					{
						glm::vec2 vector = {1,0};
						vector = glm::rotate(vector, getRandomNumberFloat(rng, 0, 3.1415926*2.f));
						spearData.balls[spearData.currentBallsCount] = {};
						spearData.balls[spearData.currentBallsCount].position = vector;

						if (getRandomChance(rng, 0.5))
						{
							spearData.balls[spearData.currentBallsCount].velocity =
								glm::normalize((glm::vec2(vector.y, -vector.x) - vector) / 2.f);
						}
						else
						{
							spearData.balls[spearData.currentBallsCount].velocity =
								glm::normalize((glm::vec2(-vector.y, vector.x) - vector) / 2.f);
						}
						
						//spearData.balls[spearData.currentBallsCount].initialVelocity = -vector;
						spearData.currentBallsCount++;
					}

					if (spearData.comboCount > 0)
					{
						spearData.comboCount--;
					}
					else
					{
						if (getRandomChance(rng, comboChance))
						{
							spearData.comboSeed = rng();
							spearData.comboCount = getRandomNumber(rng, 2, 4);
						}
					}

				}

				float ballRelativeSize = 0.05;
				float hitRelativeSize = dexterity * 0.08 + 0.04;

				for (int i = 0; i < spearData.currentBallsCount; i++)
				{
					auto &b = spearData.balls[i];
					
					if (b.passedCenter)
					{
						b.dieTimer -= deltaTime;
						if (b.dieTimer <= 0)
						{
							for (int j = i+1; j < spearData.currentBallsCount; j++)
							{
								spearData.balls[j - 1] = spearData.balls[j];
							}
							spearData.currentBallsCount--;
							notDieCount--;
							continue;
						}
					}
					else
					{
						if (glm::length(b.position) < (ballRelativeSize+hitRelativeSize)*2.f)
						{
							b.passedCenter = true;
							b.dieTimer = 0.5;
						}
					}

					glm::vec2 vectorTowardsCenter = -b.position;

					float length = glm::length(vectorTowardsCenter);
					if (length > 0.000001)
					{
						vectorTowardsCenter = glm::normalize(vectorTowardsCenter);

						if (length < 0.1f)
						{
							//b.velocity = vectorTowardsCenter;
							b.velocity += vectorTowardsCenter * speed * deltaTime * 10.f;
							b.velocity = glm::normalize(b.velocity);
						}
						else
						if (length < 0.2f)
						{
							//b.velocity = vectorTowardsCenter;
							b.velocity += vectorTowardsCenter * speed * deltaTime * 6.f;
							b.velocity = glm::normalize(b.velocity);
						}
						else
						if (length < 0.3f)
						{
							//b.velocity = vectorTowardsCenter;
							b.velocity += vectorTowardsCenter * speed * deltaTime * 4.f;
							b.velocity = glm::normalize(b.velocity);
						}else
						if (length < 0.4f)
						{
							//b.velocity = vectorTowardsCenter;
							b.velocity += vectorTowardsCenter * speed * deltaTime * 3.5f;
							b.velocity = glm::normalize(b.velocity);
						}
						else
						{
							b.velocity += vectorTowardsCenter * speed * deltaTime * 3.f;
							b.velocity = glm::normalize(b.velocity);
						}


					}

					b.position += b.velocity * deltaTime * speed;
						
				}
				
				
				if ((leftPressed || rightPressed) && spearData.currentBallsCount)
				{
					notDieCount++;
					result.hit = true;

					float length = glm::length(spearData.balls[0].position);

					if (length > (ballRelativeSize + hitRelativeSize) * 2.f)
					{
						result.hitCorectness = 0;
					}
					else
					{
						if (length <= (ballRelativeSize + hitRelativeSize) / 2.f)
						{
							result.hitCorectness = 1;
							result.bonusCritChance = 1;
						}
						else
						{
							float hitCorectness = length - ((ballRelativeSize + hitRelativeSize) / 2.f);
							hitCorectness /= ((ballRelativeSize + hitRelativeSize) * 1.5f);
							hitCorectness = 1 - hitCorectness;
							hitCorectness = glm::clamp(hitCorectness, 0.f, 1.f);

							result.bonusCritChance = (hitCorectness * 2.f) - 1;
							result.hitCorectness = hitCorectness;
							result.hitCorectness = std::max(result.hitCorectness, minHitDamage);
						}
					}


					for (int j = 1; j < spearData.currentBallsCount; j++)
					{
						spearData.balls[j - 1] = spearData.balls[j];
					}
					spearData.currentBallsCount--;
				}

				//renderer.renderRectangle(fullBox, {1,1,1,0.2});


				renderer.renderRectangle(fullBox,
					ui.battleTextures[UiENgine::BattleTextures::circle]);

				renderer.renderRectangle(fullBox,
					ui.battleTextures[UiENgine::BattleTextures::leftButton], Colors_Blue);

				renderer.renderRectangle(fullBox,
					ui.battleTextures[UiENgine::BattleTextures::rightButton], Colors_Yellow);

				renderer.renderRectangle(fullBox,
					ui.battleTextures[UiENgine::BattleTextures::leftButtonFrontAttack]);

				renderer.renderRectangle(fullBox,
					ui.battleTextures[UiENgine::BattleTextures::rightButtonSwipeAttack]);

				//renderer.renderCircleOutline(glm::vec2{glm::vec2(fullBox) 
				//	+ glm::vec2(fullBox.z/ 2.f,fullBox.w / 2.)}
				//, Colors_White, fullBox.z * hitRelativeSize, 6);

				renderer.renderRectangle(glm::vec4{glm::vec2(fullBox)
					+ glm::vec2(fullBox.z / 2.f,fullBox.w / 2.) - 
					glm::vec2(fullBox.z * hitRelativeSize)
					,
					glm::vec2(fullBox.z * hitRelativeSize * 2)
					}, ui.battleTextures[UiENgine::BattleTextures::circleSmall], {1,1,1,0.8});


				for (int i = 0; i < spearData.currentBallsCount; i++)
				{
					auto &b = spearData.balls[i];
					auto color = glm::vec4{1,1,1,0.8};

					if (b.passedCenter) { color = Colors_Green; }

					float size = fullBox.z * ballRelativeSize;

					glm::vec2 position = (b.position + glm::vec2(1.f))/2.f;
					position.x = fullBox.x + fullBox.z * position.x;
					position.y = fullBox.y + fullBox.w * position.y;

					renderer.renderRectangle(glm::vec4{glm::vec2{position} - glm::vec2(size), size*2, size*2},
						ui.battleTextures[UiENgine::BattleTextures::circleSmall], color);

					//renderer.renderCircleOutline(glm::vec2{position}
					//, color, size, 6);
				}

			}


		}
		else if (item.isSword())
		{
			if (leftPressed || rightPressed)
			{

				result.bonusCritChance = 0;
				result.hit = 1; //
				result.hitCorectness = 1;

			}


		}

		if (stats.dexterity < 0)
		{
			result.bonusCritChance = std::min(result.bonusCritChance, 0.f);
			result.bonusCritChance = powf(result.bonusCritChance, 3.f);
		}


	}


	return result;
}

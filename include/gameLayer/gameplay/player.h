#pragma once
#include <gameplay/entity.h>
#include <multyPlayer/server.h>
#include <gameplay/items.h>
#include <gl2d/gl2d.h>
#include <gameplay/life.h>
#include <gameplay/gameplayRules.h>
#include <gameplay/effects.h>

#define PLAYER_DEFAULT_LIFE Life(100)

//this is the shared data
struct Player : public PhysicalEntity, public CollidesWithPlacedBlocks,
	public CanPushOthers, public CanBeKilled, public CanBeAttacked,
	public CanHaveEffects, public HasOrientationAndHeadTurnDirection, 
	public HasEyesAndPupils<EYE_ANIMATION_TYPE_PLAYER>
{

	//todo use mem compare
	bool operator== (Player & other)
	{
		if(
			lookDirectionAnimation == other.lookDirectionAnimation &&
			bodyOrientation == other.bodyOrientation &&
			fly == other.fly
			)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool operator!= (Player & other)
	{
		return !(*this == other);
	}

	void flyFPS(glm::vec3 direction, glm::vec3 lookDirection);

	void moveFPS(glm::vec3 direction, glm::vec3 lookDirection, float deltaTime);

	int chunkDistance = 10; //TODO remove this from here!

	glm::vec3 getColliderSize();

	//todo implement!
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);


	static glm::vec3 getMaxColliderSize();

	bool fly = 0;
};

//here we store things like gamemode
struct OtherPlayerSettings
{
	constexpr static int SURVIVAL = 0;
	constexpr static int CREATIVE = 1;

	unsigned char gameMode = 0;
	char commandPermisionLevel = 2; //0 means nothing, 1 is basic stuff, 2 is admin, 3 is main admin.
};

//this is the player struct when playing locally
struct LocalPlayer
{
	PlayerInventory inventory;

	Player entity = {};

	std::uint64_t entityId = 0;

	OtherPlayerSettings otherPlayerSettings = {};
	//dodo add some other data here like inventory

	glm::ivec3 currentBlockInteractWith = {0,-1,0};
	unsigned char isInteractingWithBlock = 0;

	Life life = PLAYER_DEFAULT_LIFE;
	Life lastLife = PLAYER_DEFAULT_LIFE;
	float justHealedTimer = 0;
	float justRecievedDamageTimer = 0;

	Effects effects;
};


//the other players locally
struct PlayerClient: public ClientEntity<Player, PlayerClient>
{

	//todo other player settings here!


	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
	void setEntityMatrix(glm::mat4 *skinningMatrix);

	int getTextureIndex();

};


//todo update function
struct PlayerServer: public ServerEntity<Player>
{

	OtherPlayerSettings otherPlayerSettings = {};

	PlayerInventory inventory;


	//this also represents the interaction type
	unsigned char interactingWithBlock = 0;
	unsigned char revisionNumberInteraction = 0;
	glm::ivec3 currentBlockInteractWithPosition = {0, -1, 0};

	Life lifeLastFrame = PLAYER_DEFAULT_LIFE;
	Life newLife = PLAYER_DEFAULT_LIFE;
	bool forceUpdateLife = 0;

	//we update the effects every 20 ticks or if we set it
	// as dirty by setting this to 0
	short updateEffectsTicksTimer = 20;

	void applyDamageOrLife(short difference)
	{
		if (otherPlayerSettings.gameMode == OtherPlayerSettings::CREATIVE) { return; }

		int life = newLife.life;
		life += difference;

		if (life > newLife.maxLife) { life = newLife.maxLife; }

		newLife.life = life;
		if (difference < 0)
		{
			healingDelayCounterSecconds = 0;
		}
	}

	struct 
	{
		float regen = 0;


	}effectsTimers;

	bool killed = 0;

	//used for life regeneration
	float notIncreasedLifeSinceTimeSecconds = 0;
	float healingDelayCounterSecconds = BASE_HEALTH_DELAY_TIME;

	void kill();

	//todo calculate armour based on inventory
	Armour getArmour() { return {0}; };

	glm::ivec2 lastChunkPositionWhenAnUpdateWasSent = {};

	float calculateHealingDelayTime();
	float calculateHealingRegenTime();
};
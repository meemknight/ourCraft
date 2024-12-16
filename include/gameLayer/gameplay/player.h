#pragma once
#include <gameplay/entity.h>
#include <multyPlayer/server.h>
#include <gameplay/items.h>
#include <gl2d/gl2d.h>
#include <gameplay/life.h>

//this is the shared data
struct Player : public PhysicalEntity, public CollidesWithPlacedBlocks,
	public CanPushOthers, public CanBeKilled, public CanBeAttacked
{

	glm::vec3 lookDirectionAnimation = {0,0,-1};
	glm::vec2 bodyOrientation = {0, -1};

	void flyFPS(glm::vec3 direction, glm::vec3 lookDirection);

	void moveFPS(glm::vec3 direction, glm::vec3 lookDirection);

	int chunkDistance = 10; //remove this from here?

	glm::vec3 getColliderSize();

	static glm::vec3 getMaxColliderSize();

	bool fly = 0;
};

//here we store things like gamemode
struct OtherPlayerSettings
{
	constexpr static int SURVIVAL = 0;
	constexpr static int CREATIVE = 1;

	unsigned char gameMode = 0;

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

	Life life{20};
	Life lastLife{20};
	float justHealedTimer = 0;
	float justRecievedDamageTimer = 0;


};



struct PlayerClient: public ClientEntity<Player, PlayerClient>
{

	//todo other player settings here!

	void cleanup();

	gl2d::Texture skin = {};
	GLuint64 skinBindlessTexture = 0;

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

	Life life{20};

	bool killed = 0;

	//todo calculate armour based on inventory
	Armour getArmour() { return {0}; };

};
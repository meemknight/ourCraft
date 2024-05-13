#pragma once
#include <gameplay/entity.h>
#include <multyPlayer/server.h>
#include <gameplay/items.h>

//this is the shared data
struct Player : public PhysicalEntity
{

	glm::vec3 lookDirectionAnimation = {0,0,-1};
	glm::vec2 bodyOrientation = {0, -1};

	void flyFPS(glm::vec3 direction, glm::vec3 lookDirection);

	void moveFPS(glm::vec3 direction, glm::vec3 lookDirection);

	int chunkDistance = 10; //remove this from here?

	glm::vec3 getColliderSize();

	static glm::vec3 getMaxColliderSize();
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

};



struct PlayerClient: public ClientEntity<Player, PlayerClient>
{
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
	void setEntityMatrix(glm::mat4 *skinningMatrix);
};

//todo update function
struct PlayerServer: public ServerEntity<Player>
{

	OtherPlayerSettings otherPlayerSettings = {};

	PlayerInventory inventory;

};
#pragma once
#include <gameplay/entity.h>
#include <gameplay/life.h>
#include <random>
#include <gameplay/basicEnemyBehaviour.h>

struct Goblin: public PhysicalEntity, public CanPushOthers
	, public HasOrientationAndHeadTurnDirection, public CollidesWithPlacedBlocks,
	public CanBeKilled, public CanBeAttacked, public HasEyesAndPupils<EYE_ANIMATION_TYPE_NORMAL>,
	public Animatable
{

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);

	glm::vec3 getColliderSize();

	glm::vec3 getMaxColliderSize();

	Life life{55};

	Armour getArmour() { return {1}; };
};


struct GoblinClient: public ClientEntity<Goblin, GoblinClient>
{

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
	void setEntityMatrix(glm::mat4 *skinningMatrix);

	int getTextureIndex();
};

struct GoblinServer: public ServerEntity<Goblin> 
{

	BasicEnemyBehaviour basicEnemyBehaviour;

	bool isUnaware() { return  basicEnemyBehaviour.isUnaware(); };

	void appendDataToDisk(std::ofstream &f, std::uint64_t eId);

	bool update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
		std::unordered_set<std::uint64_t> &othersDeleted,
		std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFindingSurvival,
		std::unordered_map<std::uint64_t, glm::dvec3> &playersPositionSurvival,
		std::unordered_map < std::uint64_t, Client *> &allClients);

	void signalHit(glm::vec3 direction) { basicEnemyBehaviour.signalHit(direction, this); }

	WeaponStats getWeaponStats();

	LootTable &getLootTable() { return getEmptyLootTable(); }
};



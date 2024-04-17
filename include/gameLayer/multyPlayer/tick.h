#pragma once
#include <worldGenerator.h>
#include <multyPlayer/chunkSaver.h>
#include <multyPlayer/serverChunkStorer.h>


void doGameTick(float deltaTime,
	std::uint64_t currentTimer,
	ServerChunkStorer &chunkCache,
	EntityData &orphanEntities
);



glm::ivec2 determineChunkThatIsEntityIn(glm::dvec3 position);

#pragma once
#include <worldGenerator.h>
#include <multyPlayer/chunkSaver.h>
#include <multyPlayer/serverChunkStorer.h>
#include <random>


void doGameTick(float deltaTime,
	std::uint64_t currentTimer,
	ServerChunkStorer &chunkCache,
	EntityData &orphanEntities,
	unsigned int seed
);





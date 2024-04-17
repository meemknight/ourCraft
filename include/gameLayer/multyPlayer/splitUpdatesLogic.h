#pragma once
#include <worldGenerator.h>
#include <multyPlayer/chunkSaver.h>
#include <multyPlayer/serverChunkStorer.h>



void splitUpdatesLogic(float tickDeltaTime, std::uint64_t currentTimer,
	ServerChunkStorer &chunkCache);
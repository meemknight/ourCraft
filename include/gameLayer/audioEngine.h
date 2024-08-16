#pragma once


namespace AudioEngine
{

	void init();

	void loadAllMusic();

	void startPlayingMusicAtIndex(int index);

	void update();

	void playRandomNightMusic();

	void playTitleMusic();

	void stopAllMusicAndSounds();

	void playSound(int sound);

	enum sounds
	{
		none = 0,
		
		grass,
		dirt,
		stone,
		sand,
		wood,
		glass,
		leaves,
		snow,

		toolBreakingWood,
		toolBreakingStone,
		toolBreakingIron,

		LAST_SOUND
	};

};

int getSoundForBlock(unsigned int blockType);

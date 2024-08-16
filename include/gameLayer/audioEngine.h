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
		toolBreakingWood,
		toolBreakingStone,
		toolBreakingIron,

		LAST_SOUND
	};


};
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

	void playSound(int sound, float level);

	void playHitSound();


	enum sounds
	{
		none = 0,
		
		grass,
		dirt,
		stone,
		sand,
		wood,
		glassBreak,
		glassStep,
		leaves,
		snow,
		metal,
		wool,
		clay,
		sandStone,

		toolBreakingWood,
		toolBreakingStone,
		toolBreakingIron,

		uiButtonPress,
		uiButtonBack,
		uiOn,
		uiOff,
		uiSlider,

		hit,
		fallLow,
		fallMedium,
		fallHigh,


		LAST_SOUND
	};

};

int getSoundForBlockBreaking(unsigned int blockType);

int getSoundForBlockStepping(unsigned int blockType);

constexpr static float MINING_BLOCK_SOUND_VOLUME = 0.8;
constexpr static float PLACED_BLOCK_SOUND_VOLUME = 0.9;
constexpr static float BREAKED_BLOCK_SOUND_VOLUME = 1.0;
constexpr static float STEPPING_SOUND_VOLUME = 0.7;
constexpr static float HIT_SOUND_VOLUME = 0.8;
constexpr static float FALL_SOUND_VOLUME = 0.65;

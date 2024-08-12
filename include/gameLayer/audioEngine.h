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

};
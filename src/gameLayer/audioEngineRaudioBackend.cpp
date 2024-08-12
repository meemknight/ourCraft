#include <audioEngine.h>
#include <raudio.h>
#include <vector>


namespace AudioEngine
{

	std::vector<Music> allMusic;
	int currentMusicPlaying = 0;
	bool inited = 0;

	struct MusicIndex
	{
		int start = 0;
		int end = 0;

		int getRandomIndex()
		{
			if (start == end) { return 0; }
			return (rand() % (end - start)) + start;
		}
	};

	MusicIndex nightMusicIndex;
	MusicIndex titleScreenMusic;

	void init()
	{
		InitAudioDevice();
		SetMasterVolume(0.5);
		inited = true;
	}

	void loadAllMusic()
	{
		//an empty music at index 0
		allMusic.push_back({});

		auto loadAMusic = [&](const char *path)
		{
			Music m;

			m = LoadMusicStream(path);

			if (m.stream.buffer)
			{
				allMusic.push_back(m);
				m.looping = false;
			}
		};

		nightMusicIndex.start = allMusic.size();
		loadAMusic(RESOURCES_PATH "/music/Farlands.ogg");
		loadAMusic(RESOURCES_PATH "/music/Consuelo.ogg");
		loadAMusic(RESOURCES_PATH "/music/Fog_SlowedDown.ogg");
		loadAMusic(RESOURCES_PATH "/music/Andreas-Moon.ogg");
		nightMusicIndex.end = allMusic.size();

		titleScreenMusic.start = allMusic.size();
		loadAMusic(RESOURCES_PATH "/music/Cherry_InGame.ogg");
		titleScreenMusic.end = allMusic.size();


	}

	void startPlayingMusicAtIndex(int index)
	{

		if (allMusic.size() <= index) { return; }
	
		//todo transition period
		StopMusicStream(allMusic[index]);

		PlayMusicStream(allMusic[index]);
		SetMusicVolume(allMusic[index], 1.f);
		

		currentMusicPlaying = index;
	}

	void playRandomNightMusic()
	{
		int index = nightMusicIndex.getRandomIndex();
		startPlayingMusicAtIndex(index);
	}

	bool isMusicPlaying()
	{
		return IsMusicPlaying(allMusic[currentMusicPlaying]);
	}

	void update()
	{
		if (!inited) { return; }

		UpdateMusicStream(allMusic[currentMusicPlaying]);



	}

	void stopAllMusicAndSounds()
	{
		
		StopMusicStream(allMusic[currentMusicPlaying]);

		//todo sounds

	}


	void playTitleMusic()
	{
		stopAllMusicAndSounds();

		int index = titleScreenMusic.getRandomIndex();
		startPlayingMusicAtIndex(index);

	}


};



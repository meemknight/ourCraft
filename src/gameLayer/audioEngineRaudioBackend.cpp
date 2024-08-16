#include <audioEngine.h>
#include <raudio.h>
#include <vector>
#include <filesystem>
#include <blocks.h>

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
	

	//sounds
	struct SoundCollection
	{

		SoundCollection() {};

		//SoundCollection(std::initializer_list<const char*> s)
		//{
		//	sounds.reserve(s.size());
		//	for (auto &i : s)
		//	{
		//		Sound sound = LoadSound(i);
		//
		//		if (sound.stream.buffer != NULL)
		//		{
		//			sounds.push_back(sound);
		//		}
		//	}
		//}

		SoundCollection(const char* folderPath)
		{
			std::filesystem::path root(folderPath);
			
			if (std::filesystem::exists(root) &&
				std::filesystem::is_directory(root))
			{
				
				for (auto &f : std::filesystem::directory_iterator(root))
				{
					auto extension = f.path().extension().string();

					if (!f.is_directory() &&
						(
						extension == ".ogg" ||
						extension == ".OGG" ||
						extension == ".mp3" ||
						extension == ".MP3" ||
						extension == ".flac" ||
						extension == ".FLAC" ||
						extension == ".wav" ||
						extension == ".WAV"
						))
					{
						Sound sound = LoadSound(f.path().string().c_str());

						if (sound.stream.buffer != NULL)
						{
							sounds.push_back(sound);
						}
					}
				}


			}
		};

		std::vector<Sound> sounds;
		int lastSound = 999999;

		void playRandomSound(float volume = 1)
		{
			if (sounds.size() == 0) { return; }

			int s = rand() % sounds.size();
			if (s == lastSound) { s = rand() % sounds.size(); }
			lastSound = s;

			auto &picked = sounds[s];

			SetSoundVolume(picked, volume);
			PlaySound(picked);
		}
	};



	


	SoundCollection allSounds[] = 
	{
		SoundCollection(),


		SoundCollection(RESOURCES_PATH "/sounds/grass"),
		SoundCollection(RESOURCES_PATH "/sounds/dirt"),
		SoundCollection(RESOURCES_PATH "/sounds/stone"),
		SoundCollection(RESOURCES_PATH "/sounds/sand"),
		SoundCollection(RESOURCES_PATH "/sounds/wood"),
		SoundCollection(RESOURCES_PATH "/sounds/glass"),
		SoundCollection(RESOURCES_PATH "/sounds/leaves"),
		SoundCollection(RESOURCES_PATH "/sounds/snow"),
		

		SoundCollection(RESOURCES_PATH "sounds/toolBreakWood"),
		SoundCollection(RESOURCES_PATH "sounds/toolBreakStone"),
		SoundCollection(RESOURCES_PATH "sounds/toolBreakMetal"),

	};

	void playSound(int sound, float level)
	{
		static_assert(sizeof(allSounds) / sizeof(allSounds[0]) == LAST_SOUND);

		if (sound <= none || sound >= LAST_SOUND) { return; }

		allSounds[sound].playRandomSound(level);
	}



};

int getSoundForBlock(unsigned int type)
{
	if (!isBlock(type)) { return 0; }
	if (type == water) { return 0; }

	if (isAnyWoddenBlock(type))
	{
		return AudioEngine::wood;
	}

	if (isAnyDirtBlock(type))
	{
		return AudioEngine::dirt;
	}

	if (isAnyClayBlock(type))
	{
		return 0; //todo
	}

	if (isAnySandyBlock(type))
	{
		return AudioEngine::sand;
	}

	if (type == snow_block)
	{
		return 0; //todo
	}

	if (isAnySemiHardBlock(type))
	{
		return 0; //todo
	}

	if (isAnyStone(type) || type == testBlock)
	{
		return AudioEngine::stone;
	}

	if (isAnyPlant(type))
	{
		return 0; //todo
	}

	if (isAnyGlass(type) || type == glowstone)
	{
		return AudioEngine::glass;
	}

	if(type == ice)
	{
		return AudioEngine::glass; //todo?
	}

	if (isAnyLeaves(type))
	{
		return AudioEngine::leaves;
	}

	if (isAnyWool(type))
	{
		return 0;
	}

	if (isAnyUnbreakable(type))
	{
		return AudioEngine::stone; //todo
	}

	if (isTriviallyBreakable(torch))
	{
		return 0; //todo
	}

	return 0;
}
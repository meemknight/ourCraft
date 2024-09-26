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

		auto loadMusicFolder = [&](MusicIndex& index, const char *root)
		{
			index.start = allMusic.size();

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
					loadAMusic(f.path().string().c_str());
				}
			}


			index.end = allMusic.size();
		};

		
		loadMusicFolder(nightMusicIndex, RESOURCES_PATH "/music/night");
		loadMusicFolder(titleScreenMusic, RESOURCES_PATH "/music/titleScreen");


	}

	void startPlayingMusicAtIndex(int index)
	{

		if (allMusic.size() <= index) { return; }
	
		//todo transition period
		StopMusicStream(allMusic[index]);

		allMusic[index].looping = false;
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

			float pitch = 0.7f;
			pitch += 0.3f * ((rand() % 100) / 100.f);

			SetSoundVolume(picked, volume);
			SetSoundPitch(picked, pitch);
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
		SoundCollection(RESOURCES_PATH "/sounds/glassBreak"),
		SoundCollection(RESOURCES_PATH "/sounds/glass"),
		SoundCollection(RESOURCES_PATH "/sounds/leaves"),
		SoundCollection(RESOURCES_PATH "/sounds/snow"),
		SoundCollection(RESOURCES_PATH "/sounds/metal"),
		SoundCollection(RESOURCES_PATH "/sounds/wool"),
		SoundCollection(RESOURCES_PATH "/sounds/clay"),
		SoundCollection(RESOURCES_PATH "/sounds/sandStone"),
		SoundCollection(RESOURCES_PATH "/sounds/bricks"),
		

		SoundCollection(RESOURCES_PATH "sounds/toolBreakWood"),
		SoundCollection(RESOURCES_PATH "sounds/toolBreakStone"),
		SoundCollection(RESOURCES_PATH "sounds/toolBreakMetal"),

		SoundCollection(RESOURCES_PATH "/sounds/buttonPress"),
		SoundCollection(RESOURCES_PATH "/sounds/buttonBack"),
		SoundCollection(RESOURCES_PATH "/sounds/buttonOn"),
		SoundCollection(RESOURCES_PATH "/sounds/buttonOff"),
		SoundCollection(RESOURCES_PATH "/sounds/buttonSlider"),

		SoundCollection(RESOURCES_PATH "/sounds/hit"),

		SoundCollection(RESOURCES_PATH "/sounds/lowImpact"),
		SoundCollection(RESOURCES_PATH "/sounds/mediumImpact"),
		SoundCollection(RESOURCES_PATH "/sounds/highImpact"),

		SoundCollection(RESOURCES_PATH "/sounds/hurt"),


	};

	void playSound(int sound, float level)
	{
		static_assert(sizeof(allSounds) / sizeof(allSounds[0]) == LAST_SOUND);

		if (sound <= none || sound >= LAST_SOUND) { return; }

		allSounds[sound].playRandomSound(level);
	}

	void playHitSound()
	{
		playSound(hit, HIT_SOUND_VOLUME);
	}

	void playHurtSound()
	{
		playSound(hurt, HIT_SOUND_VOLUME);
	}


};

int getSoundForBlockStepping(unsigned int blockType)
{

	if (isAnyGlass(blockType) || blockType == glowstone)
	{
		return AudioEngine::glassStep;
	}

	if (blockType == ice)
	{
		return AudioEngine::glassStep; //todo?
	}

	return getSoundForBlockBreaking(blockType);

}

int getSoundForBlockBreaking(unsigned int type)
{
	if (!isBlock(type)) { return 0; }
	if (type == water) { return 0; }


	if(isBricksSound(type))
	{
		return AudioEngine::bricks;
	}

	if (type == gold_block)
	{
		return AudioEngine::metal;
	}

	if (type == grassBlock)
	{
		return AudioEngine::grass;
	}

	if (type == snow_block || type == snow_dirt)
	{
		return AudioEngine::snow;
	}


	///

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
		return AudioEngine::clay;
	}

	if (isAnySandyBlock(type))
	{
		return AudioEngine::sand;
	}

	if (isAnySemiHardBlock(type))
	{
		return AudioEngine::sandStone;
	}

	if (isAnyStone(type) || type == testBlock)
	{
		return AudioEngine::stone;
	}

	if (isAnyPlant(type))
	{
		return AudioEngine::leaves; //todo
	}

	if (isAnyGlass(type) || type == glowstone)
	{
		return AudioEngine::glassBreak;
	}

	if (type == ice)
	{
		return AudioEngine::glassBreak; //todo?
	}

	if (isAnyLeaves(type))
	{
		return AudioEngine::leaves;
	}

	if (isAnyWool(type))
	{
		return AudioEngine::wool;
	}

	if (isAnyUnbreakable(type))
	{
		return AudioEngine::stone; //todo
	}

	if (isTriviallyBreakable(type))
	{
		return 0; //todo
	}

	return AudioEngine::stone;
}
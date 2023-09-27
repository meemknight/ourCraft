#pragma once 
#include <FastNoiseSIMD.h>
#include <splines.h>
#include <string>


struct NoiseSetting
{
	float scale = 0.35;
	int type = FastNoiseSIMD::NoiseType::SimplexFractal;
	float frequency = 0.015;
	int octaves = 3;
	int perturbFractalOctaves = 0;

	Spline spline;

	std::string saveSettings(int tabs);

	void sanitize();
};

struct WorldGeneratorSettings
{

	int seed = 1234;
	NoiseSetting continentalnessNoiseSettings;

	std::string saveSettings();

	void sanitize();

	bool loadSettings(const char *data);
};

struct WorldGenerator
{

	FastNoiseSIMD *continentalnessNoise;
	Spline continentalSplines;

	void init();
	void clear();

	void applySettings(WorldGeneratorSettings &s);
};
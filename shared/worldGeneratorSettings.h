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

	float power = 1;

	Spline spline;

	std::string saveSettings(int tabs);

	void sanitize();
};

struct WorldGeneratorSettings
{

	int seed = 1234;
	NoiseSetting continentalnessNoiseSettings;
	NoiseSetting peaksAndValies;
	float peaksAndValiesContribution = 0.5;

	std::string saveSettings();

	void sanitize();

	bool loadSettings(const char *data);
};

struct WorldGenerator
{

	FastNoiseSIMD *continentalnessNoise;
	Spline continentalSplines;
	float continentalPower = 1.f;

	FastNoiseSIMD *peaksValiesNoise;
	Spline peaksValiesSplines;
	float peaksValiesPower = 1.f;

	float peaksAndValiesContribution = 0.5;

	void init();
	void clear();

	void applySettings(WorldGeneratorSettings &s);
};
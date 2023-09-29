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

	std::string saveSettings(int tabs, bool saveSpline = true);

	void sanitize();
};

struct WorldGeneratorSettings
{

	int seed = 1234;
	NoiseSetting continentalnessNoiseSettings;
	NoiseSetting peaksAndValies;
	NoiseSetting oceansAndTerases;
	NoiseSetting stone3Dnoise;
	Spline peaksAndValiesContributionSpline;
	Spline oceansAndTerasesContributionSpline;

	NoiseSetting vegetationNoise;

	float densityBias = 0.1;
	float densityBiasPower = 1;

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
	Spline peaksValiesContributionSplines;
	float peaksValiesPower = 1.f;

	FastNoiseSIMD *oceansAndTerasesNoise;
	Spline oceansAndTerasesSplines;
	Spline oceansAndTerasesContributionSplines;
	float oceansAndTerasesPower = 1.f;

	FastNoiseSIMD *stone3Dnoise;
	Spline stone3DnoiseSplines;
	float stone3Dpower = 1.f;
	float densityBias = 0.1;
	float densityBiasPower = 1;

	FastNoiseSIMD *vegetationNoise;
	float vegetationPower = 1;
	Spline vegetationSplines;

	FastNoiseSIMD *whiteNoise;

	FastNoiseSIMD *whiteNoise2;


	void init();
	void clear();

	void applySettings(WorldGeneratorSettings &s);
};
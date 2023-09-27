#pragma once 
#include <FastNoiseSIMD.h>
#include <splines.h>


struct NoiseSetting
{
	float scale = 0.35;
	int type = FastNoiseSIMD::NoiseType::SimplexFractal;
	float frequency = 0.015;
	int octaves = 3;
	int perturbFractalOctaves = 0;

	Spline spline;
};

struct WorldGeneratorSettings
{

	NoiseSetting continentalnessNoiseSettings;

};

struct WorldGenerator
{

	FastNoiseSIMD *continentalnessNoise;

	void init();
	void clear();

	void applySettings(WorldGeneratorSettings &s);
};
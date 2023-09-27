#include "worldGeneratorSettings.h"
#include <FastNoiseSIMD.h>

void WorldGenerator::init()
{
	continentalnessNoise = FastNoiseSIMD::NewFastNoiseSIMD();

	WorldGeneratorSettings s;
	applySettings(s);
}

void WorldGenerator::clear()
{
	delete continentalnessNoise;
}

void WorldGenerator::applySettings(WorldGeneratorSettings &s)
{

	continentalnessNoise->SetNoiseType((FastNoiseSIMD::NoiseType)s.continentalnessNoiseSettings.type);
	continentalnessNoise->SetAxisScales(s.continentalnessNoiseSettings.scale, 1, s.continentalnessNoiseSettings.scale);
	continentalnessNoise->SetFrequency(s.continentalnessNoiseSettings.frequency);
	continentalnessNoise->SetFractalOctaves(s.continentalnessNoiseSettings.octaves);
	continentalnessNoise->SetPerturbFractalOctaves(s.continentalnessNoiseSettings.perturbFractalOctaves);



}

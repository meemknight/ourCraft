#pragma once 
#include <FastNoiseSIMD.h>
#include <splines.h>
#include <string>

const int PLAINS_HEIGHT_INDEX = 2;

struct NoiseSetting
{
	float scale = 0.35;
	int type = FastNoiseSIMD::NoiseType::SimplexFractal;
	int cellularReturnType = FastNoiseSIMD::CellularReturnType::Distance;
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
	NoiseSetting continentalness2NoiseSettings;
	NoiseSetting continentalnessPickSettings;
	NoiseSetting peaksAndValies;
	NoiseSetting randomHills;
	NoiseSetting wierdness;
	NoiseSetting stone3Dnoise;
	NoiseSetting riversNoise;
	NoiseSetting roadsNoise;
	NoiseSetting spagettiNoise;
	NoiseSetting hillsDrops;
	NoiseSetting randomSand;
	NoiseSetting stonePatches;
	NoiseSetting treesAmountNoise;
	NoiseSetting lakesNoise;
	NoiseSetting treesTypeNoise;
	NoiseSetting cavesNoise;
	NoiseSetting swampNoise;
	NoiseSetting stoneSpikesNoise;
	NoiseSetting swampMask;
	NoiseSetting stoneSpikeMask;
	NoiseSetting iceNoise;

	bool isSuperFlat = 0;

	//todo remove
	Spline peaksAndValiesContributionSpline;
	Spline regionsHeightSpline;

	NoiseSetting vegetationNoise;

	std::string saveSettings();

	void sanitize();

	bool loadSettings(const char *data);
};

namespace gl2d
{
	struct Texture;
};

struct WorldGenerator
{


	FastNoiseSIMD *regionsHeightNoise = 0;
	FastNoiseSIMD *regionsHeightTranzition = 0;
	FastNoiseSIMD *regionsRandomNumber = 0;
	FastNoiseSIMD *regionsX = 0;
	FastNoiseSIMD *regionsZ = 0;
	Spline regionsHeightSplines;



	//
	FastNoiseSIMD *continentalnessNoise;
	Spline continentalSplines;
	float continentalPower = 1.f;

	FastNoiseSIMD *continentalness2Noise;
	Spline continental2Splines;
	float continental2Power = 1.f;

	FastNoiseSIMD *continentalnessPickNoise;
	Spline continentalnessPickSplines;
	float continentalnessPickPower = 1.f;

	FastNoiseSIMD *peaksValiesNoise;
	Spline peaksValiesSplines;
	Spline peaksValiesContributionSplines;
	float peaksValiesPower = 1.f;

	FastNoiseSIMD *randomHillsNoise;
	Spline randomHillsSplines;
	float randomHillsPower = 1.f;

	FastNoiseSIMD *swampNoise;
	Spline swampSplines;
	float swampPower = 1.f;

	FastNoiseSIMD *swampMask;
	Spline swampMaskSplines;
	float swampMaskPower = 1.f;

	FastNoiseSIMD *stoneSpikesNoise;
	Spline stoneSpikesSplines;
	float stoneSpikesPower = 1.f;

	FastNoiseSIMD *stoneSpikesMask;
	Spline stoneSpikesMaskSplines;
	float stoneSpikesMaskPower = 1.f;

	FastNoiseSIMD *iceNoise;
	Spline iceNoiseSplines;
	float iceNoisePower = 1.f;

	FastNoiseSIMD *wierdnessNoise;
	Spline wierdnessSplines;
	//Spline oceansAndTerasesContributionSplines;
	float wierdnessPower = 1.f;

	FastNoiseSIMD *stone3Dnoise;
	Spline stone3DnoiseSplines;
	float stone3Dpower = 1.f;

	FastNoiseSIMD *spagettiNoise;
	Spline spagettiNoiseSplines;
	float spagettiNoisePower = 1.f;

	FastNoiseSIMD *riversNoise;
	Spline riversSplines;
	float riversPower = 1.f;

	FastNoiseSIMD *cavesNoise;
	Spline cavesSpline;
	float cavesPower = 1.f;

	FastNoiseSIMD *treesAmountNoise;
	Spline treesAmountSpline;
	float treesAmountPower = 1.f;

	FastNoiseSIMD *treesTypeNoise;
	Spline treesTypeSpline;
	float treesTypePower = 1.f;

	FastNoiseSIMD *hillsDropsNoise;
	Spline hillsDropsSpline;
	float hillsDropsPower = 1.f;

	FastNoiseSIMD *stonePatchesNoise;
	Spline stonePatchesSpline;
	float stonePatchesPower = 1.f;

	FastNoiseSIMD *roadNoise;
	Spline roadSplines;
	float roadPower = 1.f;


	FastNoiseSIMD *lakesNoise;
	Spline lakesSplines;
	float lakesPower = 1.f;

	FastNoiseSIMD *whiteNoise;

	FastNoiseSIMD *whiteNoise2;

	FastNoiseSIMD *randomStonesNoise;

	FastNoiseSIMD *randomSandPatchesNoise;
	Spline randomSandSplines;
	float randomSandPower = 1.f;

	FastNoiseSIMD *alternativePatchesOfBlocks;
	bool isSuperFlat = 0;


	void init();
	void clear();

	void applySettings(WorldGeneratorSettings &s);

	int getRegionHeightForChunk(int chunkX, int chunkZ);

	int getRegionHeightAndBlendingsForChunk(int chunkX, int chunkZ, float values[16*16],
		float borderingFactor[16 * 16], float &vegetationMaster, float tightBorders[16 * 16],
		float &xValuue, float &zValue, float &biomeTypeRandomValue
		);

	//this isn't game accurate!
	void generateChunkPreview(gl2d::Texture &t, glm::ivec2 size, glm::ivec2 pos);


};
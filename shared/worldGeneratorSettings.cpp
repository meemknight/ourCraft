#include "worldGeneratorSettings.h"
#include <FastNoiseSIMD.h>
#include <magic_enum.hpp>
#include <glm/glm.hpp>
#include <iostream>

void WorldGenerator::init()
{
	FastNoiseSIMD::SetSIMDLevel(3);
	continentalnessNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	continentalness2Noise = FastNoiseSIMD::NewFastNoiseSIMD();
	continentalnessPickNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	randomHillsNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	peaksValiesNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	wierdnessNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	stone3Dnoise = FastNoiseSIMD::NewFastNoiseSIMD();
	whiteNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	whiteNoise2 = FastNoiseSIMD::NewFastNoiseSIMD();
	spagettiNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	iceNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	regionsX = FastNoiseSIMD::NewFastNoiseSIMD();
	regionsZ = FastNoiseSIMD::NewFastNoiseSIMD();
	hillsDropsNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	regionsRandomNumber = FastNoiseSIMD::NewFastNoiseSIMD();
	stonePatchesNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	treesAmountNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	treesTypeNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	cavesNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	lakesNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	swampNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	swampMask = FastNoiseSIMD::NewFastNoiseSIMD();
	stoneSpikesNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	stoneSpikesMask = FastNoiseSIMD::NewFastNoiseSIMD();

	riversNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	roadNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	randomSandPatchesNoise = FastNoiseSIMD::NewFastNoiseSIMD();


	regionsHeightNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	regionsHeightTranzition = FastNoiseSIMD::NewFastNoiseSIMD();

	randomStonesNoise = FastNoiseSIMD::NewFastNoiseSIMD();

	alternativePatchesOfBlocks = FastNoiseSIMD::NewFastNoiseSIMD();

	WorldGeneratorSettings s;
	applySettings(s);
}

void WorldGenerator::clear()
{
	delete continentalnessNoise;
	delete continentalness2Noise;
	delete continentalnessPickNoise;
	delete randomHillsNoise;
	delete peaksValiesNoise;
	delete wierdnessNoise;
	delete stone3Dnoise;
	delete whiteNoise;
	delete whiteNoise2;
	delete spagettiNoise;
	delete iceNoise;
	delete roadNoise;
	delete riversNoise;
	delete regionsHeightNoise;
	delete regionsHeightTranzition;
	delete randomStonesNoise;
	delete alternativePatchesOfBlocks;
	delete hillsDropsNoise;
	delete regionsRandomNumber;
	delete randomSandPatchesNoise;
	delete stonePatchesNoise;
	delete treesTypeNoise;
	delete cavesNoise;
	delete treesAmountNoise;
	delete lakesNoise;
	delete regionsX;
	delete regionsZ;
	delete swampNoise;
	delete stoneSpikesNoise;
	delete swampMask;
	delete stoneSpikesMask;

	*this = {};
}

void WorldGenerator::applySettings(WorldGeneratorSettings &s)
{
	whiteNoise->SetSeed(s.seed);
	whiteNoise->SetNoiseType(FastNoiseSIMD::NoiseType::WhiteNoise);

	whiteNoise2->SetSeed(s.seed + 1);
	whiteNoise2->SetNoiseType(FastNoiseSIMD::NoiseType::WhiteNoise);

	auto apply = [&](FastNoiseSIMD *noise, int seed, NoiseSetting &s)
	{
		noise->SetSeed(seed);
		noise->SetNoiseType((FastNoiseSIMD::NoiseType)s.type);
		noise->SetAxisScales(s.scale, 1, s.scale);
		noise->SetFrequency(s.frequency);
		noise->SetFractalOctaves(s.octaves);
		noise->SetPerturbFractalOctaves(s.perturbFractalOctaves);

		if (s.type == FastNoiseSIMD::NoiseType::Cellular)
		{
			noise->SetCellularReturnType((FastNoiseSIMD::CellularReturnType)s.cellularReturnType);
		}
	};

	apply(continentalnessNoise, s.seed + 2, s.continentalnessNoiseSettings);
	continentalSplines = s.continentalnessNoiseSettings.spline;
	continentalPower = s.continentalnessNoiseSettings.power;

	apply(continentalness2Noise, s.seed + 33, s.continentalness2NoiseSettings);
	continental2Splines = s.continentalness2NoiseSettings.spline;
	continental2Power = s.continentalness2NoiseSettings.power;

	apply(continentalnessPickNoise, s.seed + 43, s.continentalnessPickSettings);
	continentalnessPickSplines = s.continentalnessPickSettings.spline;
	continentalnessPickPower = s.continentalnessPickSettings.power;

	apply(stone3Dnoise, s.seed + 3, s.stone3Dnoise);
	stone3DnoiseSplines = s.stone3Dnoise.spline;
	stone3Dpower = s.stone3Dnoise.power;

	apply(randomHillsNoise, s.seed + 45, s.randomHills);
	randomHillsSplines = s.randomHills.spline;
	randomHillsPower = s.randomHills.power;

	apply(spagettiNoise, s.seed + 4, s.spagettiNoise);
	spagettiNoiseSplines = s.spagettiNoise.spline;
	spagettiNoisePower = s.spagettiNoise.power;

	apply(peaksValiesNoise, s.seed + 5, s.peaksAndValies);
	peaksValiesSplines = s.peaksAndValies.spline;
	peaksValiesPower = s.peaksAndValies.power;
	peaksValiesContributionSplines = s.peaksAndValiesContributionSpline;

	apply(wierdnessNoise, s.seed + 6, s.wierdness);
	wierdnessSplines = s.wierdness.spline;
	wierdnessPower = s.wierdness.power;

	apply(riversNoise, s.seed + 9, s.riversNoise);
	riversPower = s.riversNoise.power;
	riversSplines = s.riversNoise.spline;

	apply(roadNoise, s.seed + 10, s.roadsNoise);
	roadPower = s.roadsNoise.power;
	roadSplines = s.roadsNoise.spline;

	regionsHeightSplines = s.regionsHeightSpline;


	apply(hillsDropsNoise, s.seed + 11, s.hillsDrops);
	hillsDropsPower = s.hillsDrops.power;
	hillsDropsSpline = s.hillsDrops.spline;


	apply(randomSandPatchesNoise, s.seed + 12, s.randomSand);
	randomSandPower = s.randomSand.power;
	randomSandSplines = s.randomSand.spline;

	apply(stonePatchesNoise, s.seed + 13, s.stonePatches);
	stonePatchesPower = s.stonePatches.power;
	stonePatchesSpline = s.stonePatches.spline;

	apply(treesAmountNoise, s.seed + 14, s.treesAmountNoise);
	treesAmountPower = s.treesAmountNoise.power;
	treesAmountSpline = s.treesAmountNoise.spline;

	apply(treesTypeNoise, s.seed + 15, s.treesTypeNoise);
	treesTypePower = s.treesTypeNoise.power;
	treesTypeSpline = s.treesTypeNoise.spline;

	apply(cavesNoise, s.seed + 16, s.cavesNoise);
	cavesPower = s.cavesNoise.power;
	cavesSpline = s.cavesNoise.spline;

	apply(lakesNoise, s.seed + 17, s.lakesNoise);
	lakesPower = s.lakesNoise.power;
	lakesSplines = s.lakesNoise.spline;

	apply(swampNoise, s.seed + 18, s.swampNoise);
	swampPower = s.swampNoise.power;
	swampSplines = s.swampNoise.spline;

	apply(stoneSpikesNoise, s.seed + 19, s.stoneSpikesNoise);
	stoneSpikesPower = s.stoneSpikesNoise.power;
	stoneSpikesSplines = s.stoneSpikesNoise.spline;

	apply(swampMask, s.seed + 20, s.swampMask);
	swampMaskPower = s.swampMask.power;
	swampMaskSplines = s.swampMask.spline;

	apply(stoneSpikesMask, s.seed + 21, s.stoneSpikeMask);
	stoneSpikesMaskPower = s.stoneSpikeMask.power;
	stoneSpikesMaskSplines = s.stoneSpikeMask.spline;

	apply(iceNoise, s.seed + 22, s.iceNoise);
	iceNoisePower = s.iceNoise.power;
	iceNoiseSplines = s.iceNoise.spline;

	regionsHeightNoise->SetSeed(s.seed);
	regionsHeightNoise->SetAxisScales(1, 1, 1);
	//regionsHeightNoise->SetFrequency(0.002);
	//regionsHeightNoise->SetFrequency(0.024); //original intended scale
	//regionsHeightNoise->SetFrequency(0.04); //probably will use this
	regionsHeightNoise->SetFrequency(0.1);
	//regionsHeightNoise->SetFrequency(0.4); //for testing

	regionsHeightNoise->SetNoiseType(FastNoiseSIMD::NoiseType::Cellular);
	regionsHeightNoise->SetCellularReturnType(FastNoiseSIMD::CellularReturnType::NoiseLookup);
	regionsHeightNoise->SetCellularDistanceFunction(FastNoiseSIMD::CellularDistanceFunction::Natural);
	regionsHeightNoise->SetCellularJitter(0.31);
	regionsHeightNoise->SetCellularNoiseLookupType(FastNoiseSIMD::NoiseType::Perlin);
	//regionsHeightNoise->SetCellularNoiseLookupFrequency(0.22); //original
	//regionsHeightNoise->SetCellularNoiseLookupFrequency(0.25);
	//regionsHeightNoise->SetCellularNoiseLookupFrequency(0.30); //currently used
	regionsHeightNoise->SetCellularNoiseLookupFrequency(0.35);


	*regionsHeightTranzition = *regionsHeightNoise;
	regionsHeightTranzition->SetCellularReturnType(FastNoiseSIMD::CellularReturnType::Distance);


	*regionsRandomNumber = *regionsHeightNoise;
	regionsRandomNumber->SetCellularReturnType(FastNoiseSIMD::CellularReturnType::CellValue);

	*regionsX = *regionsHeightNoise;
	regionsX->SetCellularReturnType(FastNoiseSIMD::CellularReturnType::CellX);

	*regionsZ = *regionsHeightNoise;
	regionsZ->SetCellularReturnType(FastNoiseSIMD::CellularReturnType::CellZ);

	randomStonesNoise->SetSeed(s.seed + 100);
	randomStonesNoise->SetNoiseType(FastNoiseSIMD::NoiseType::Simplex);
	float scale = 0.05 * 16;
	randomStonesNoise->SetAxisScales(scale, scale, scale);
	randomStonesNoise->SetFrequency(0.015);


	alternativePatchesOfBlocks->SetSeed(s.seed + 101);
	alternativePatchesOfBlocks->SetAxisScales(1, 1, 1);
	alternativePatchesOfBlocks->SetFrequency(0.016);
	alternativePatchesOfBlocks->SetNoiseType(FastNoiseSIMD::NoiseType::Cellular);
	alternativePatchesOfBlocks->SetCellularReturnType(FastNoiseSIMD::CellularReturnType::NoiseLookup);
	alternativePatchesOfBlocks->SetCellularDistanceFunction(FastNoiseSIMD::CellularDistanceFunction::Natural);
	alternativePatchesOfBlocks->SetCellularNoiseLookupFrequency(0.30);
	alternativePatchesOfBlocks->SetCellularNoiseLookupType(FastNoiseSIMD::NoiseType::Simplex);

	isSuperFlat = s.isSuperFlat;
}


float truncateTo4Decimals(float number)
{
	// Multiply by 10^4 to shift the decimal point 4 places to the right
	float factor = 10000.0f;
	number = std::floor(number * factor) / factor; // Truncate the decimals
	return number;
}


int WorldGenerator::getRegionHeightAndBlendingsForChunk(int chunkX, int chunkZ,
	float values[16 * 16], float borderingFactor[16 * 16], float &vegetationMaster,
	float tightBorders[16 * 16], float &xValue, float &zValue, float &biomeTypeRandomValue)
{
	float *rezult
		= regionsHeightNoise->GetNoiseSet(chunkX-1, 0, chunkZ-1,
		3, (1), 3, 1);

	float *rezult2 = regionsHeightTranzition->GetNoiseSet(chunkX * 16, 0, chunkZ * 16, 16, 1, 16,
		1.f / 16.f);

	float *rezult3
		= regionsRandomNumber->GetNoiseSet(chunkX-1, 0, chunkZ-1,
		3, (1), 3);

	biomeTypeRandomValue = rezult3[4]; //todo change !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	biomeTypeRandomValue += 1.f;
	biomeTypeRandomValue /= 2.f;
	

	float *rezultX = regionsX->GetNoiseSet(chunkX-1, 0, chunkZ-1, 3, 1, 3);
	for (int i = 0; i < 9; i++)
	{
		rezultX[i] = truncateTo4Decimals(rezultX[i]);
		rezultX[i] *= (1.f / regionsX->GetFrequency()) * (1.f / regionsX->GetAxisScaleX());
	}
	xValue = rezultX[4];

	float *rezultZ = regionsZ->GetNoiseSet(chunkX-1, 0, chunkZ-1, 3, 1, 3);
	for (int i = 0; i < 9; i++)
	{
		rezultZ[i] = truncateTo4Decimals(rezultZ[i]);
		rezultZ[i] *= (1.f / regionsZ->GetFrequency()) * (1.f / regionsZ->GetAxisScaleZ());
	}
	zValue = rezultZ[4];
	


	//float *test = 
		//regionsHeightNoise->SetCellularDistanceFunction()
	//FastNoiseSIMD::CellularDistanceFunction;

	vegetationMaster = rezult3[4];
	vegetationMaster += 1.f;
	vegetationMaster /= 2.f;

	for (int i = 0; i < 16 * 16; i++)
	{
		//todo investigate here!!!!!!!!!!!!!!!!!1
		//rezult2[i] += 1;
		//rezult2[i] /= 2;
		//rezult2[i] = std::powf(rezult2[i], 1.1);
		if (rezult2[i] < 0.05f) { rezult2[i] = 0; }
		borderingFactor[i] = rezult2[i];
	}

	//if (rezult[0] < 0) { std::cout << rezult[0] << " "; }
	//if (rezult2[0] < 0) { std::cout << rezult2[0] << " "; }

	for (int i = 0; i < 3*3; i++)
	{
		//rezult[i] += 1;
		//rezult[i] /= 2.f;

		rezult[i] = regionsHeightSplines.applySpline((rezult[i] + 1) / 2.f);

		rezult[i] *= 6;
		rezult[i] = floor(rezult[i]);
		if (rezult[i] == 6) { rezult[i] = 5; }
		//rezult[i] += 0.1;
	}

	auto getRezultValues = [&](int x, int y)
	{
		return rezult[x + y * 3];
	};

	auto getXValues = [&](int x, int y)
	{
		return rezultX[x + y * 3];
	};

	auto getZValues = [&](int x, int y)
	{
		return rezultZ[x + y * 3];
	};

	for (int j = 0; j < 16; j++)
		for (int i = 0; i < 16; i++)
		{
			tightBorders[i + j * 16] = 0;

			if (j == 0 && ((getXValues(1, 1) != getXValues(1, 0)) || (getZValues(1, 1) != getZValues(1, 0))) )
			{
				tightBorders[i + j * 16] = 1;
			}
			else if (j == 15 && (  (getXValues(1, 1) != getXValues(1, 2)) || (getZValues(1, 1) != getZValues(1, 2))  )   )
			{
				tightBorders[i + j * 16] = 1;
			}
			else if (i == 0 &&( (getXValues(1, 1) != getXValues(0, 1)) || (getZValues(1, 1) != getZValues(0, 1)))  )
			{
				tightBorders[i + j * 16] = 1;
			}
			else if (i == 15 && ( (getXValues(1, 1) != getXValues(2, 1))  ||  (getZValues(1, 1) != getZValues(2, 1)))  )
			{
				tightBorders[i + j * 16] = 1;
			}
		}

	for (int j = 0; j < 16; j++)
		for (int i = 0; i < 16; i++)
		{
			
			float rez = 0;
			int counter = 0;
			
			for (int y = -14; y < 14; y++)
				for (int x = -14; x < 14; x++)
				{
					if (x * x + y * y > 14 * 14) { continue; } //round up corners

					int rezX = i + x;
					int rezY = j + y;

					if(rezX < 0){rezX = 0;}else if(rezX >= 16){rezX = 2;}else{rezX = 1;}
					if(rezY < 0){rezY = 0;}else if(rezY >= 16){rezY = 2;}else{rezY = 1;}

					rez += getRezultValues(rezX, rezY);
					counter++;
				}
			rez /= (float)counter;

			values[i + j * 16] = rez;
		}

	int value = getRezultValues(1, 1);

	FastNoiseSIMD::FreeNoiseSet(rezult);
	FastNoiseSIMD::FreeNoiseSet(rezult2);
	FastNoiseSIMD::FreeNoiseSet(rezult3);
	FastNoiseSIMD::FreeNoiseSet(rezultX);
	FastNoiseSIMD::FreeNoiseSet(rezultZ);

	return value;
}

/*
int WorldGenerator::getRegionHeightAndBlendingsForChunk(int chunkX, int chunkZ, float values[16 * 16])
{
	float *rezult
		= regionsHeightNoise->GetNoiseSet(chunkX-2, 0, chunkZ-2,
		5, (1), 5, 1);


	for (int i = 0; i < 5*5; i++)
	{
		rezult[i] = regionsHeightSplines.applySpline((rezult[i] + 1) / 2.f);

		rezult[i] *= 6;
		rezult[i] = floor(rezult[i]);
		//rezult[i] += 0.1;
	}

	auto getRezultValues = [&](int x, int y)
	{
		return rezult[x + y * 5];
	};

	for (int j = 0; j < 16; j++)
		for (int i = 0; i < 16; i++)
		{

			float rez = 0;
			int counter = 0;

			for (int y = -24; y < 24; y++)
				for (int x = -24; x < 24; x++)
				{
					if (x * x + y * y > 24 * 24) { continue; } //round up corners

					int rezX = i + x;
					int rezY = j + y;

					if(rezX < -32){rezX = 0;}else if(rezX < 16){rezX = 1;}else if(rezX<0){rezX = 2;}else if(rezX<16){rezX = 3;}else{rezX = 4;}
					if(rezY < -32){rezY = 0;}else if(rezY < 16){rezY = 1;}else if(rezY<0){rezY = 2;}else if(rezY<16){rezY = 3;}else{rezY = 4;}

					rez += getRezultValues(rezX, rezY);
					counter++;
				}
			rez /= (float)counter;

			values[i + j * 16] = rez;
		}

	int value = getRezultValues(2, 2);

	FastNoiseSIMD::FreeNoiseSet(rezult);

	return value;
}
*/



std::string WorldGeneratorSettings::saveSettings()
{

	std::string rez;
	rez.reserve(500);


	rez += "seed: "; rez += std::to_string(seed); rez += ";\n";

	if (isSuperFlat)
	{
		rez += "superFlat: 1;\n";
	}
	else
	{

		rez += "continentalnessNoise:\n";
		rez += continentalnessNoiseSettings.saveSettings(1);

		rez += "continentalnessNoise2:\n";
		rez += continentalness2NoiseSettings.saveSettings(1);

		rez += "continentalnessPickNoise:\n";
		rez += continentalnessPickSettings.saveSettings(1);

		rez += "swampNoise:\n";
		rez += swampNoise.saveSettings(1);

		rez += "stoneSpikesNoise:\n";
		rez += stoneSpikesNoise.saveSettings(1);

		rez += "stoneSpikesMask:\n";
		rez += stoneSpikeMask.saveSettings(1);

		rez += "swampMask:\n";
		rez += swampMask.saveSettings(1);

		rez += "iceNoise:\n";
		rez += iceNoise.saveSettings(1);

		rez += "peaksAndValies:\n";
		rez += peaksAndValies.saveSettings(1);
		rez += "peaksAndValiesContributionSpline:\n";
		rez += peaksAndValiesContributionSpline.saveSettings(1);

		rez += "randomHills:\n";
		rez += randomHills.saveSettings(1);

		rez += "wierdness:\n";
		rez += wierdness.saveSettings(1);

		rez += "vegetationNoise:\n";
		rez += vegetationNoise.saveSettings(1);

		rez += "stonetDnoise:\n";
		rez += stone3Dnoise.saveSettings(1);

		rez += "riversNoise:\n";
		rez += riversNoise.saveSettings(1);

		rez += "roadsNoise:\n";
		rez += roadsNoise.saveSettings(1);

		rez += "stonePatches:\n";
		rez += stonePatches.saveSettings(1);

		rez += "randomSand:\n";
		rez += randomSand.saveSettings(1);

		rez += "hillsDrop:\n";
		rez += hillsDrops.saveSettings(1);

		rez += "treesAmountNoise:\n";
		rez += treesAmountNoise.saveSettings(1);

		rez += "treesTypeNoise:\n";
		rez += treesTypeNoise.saveSettings(1);

		rez += "cavesNoise:\n";
		rez += cavesNoise.saveSettings(1);

		rez += "lakesNoise:\n";
		rez += lakesNoise.saveSettings(1);

		rez += "spagettiNoise:\n";
		rez += spagettiNoise.saveSettings(1);
		//rez += "spagettiBias: "; rez += std::to_string(spagettiBias); rez += ";\n";
		//rez += "spagettiBiasPower: "; rez += std::to_string(spagettiBiasPower); rez += ";\n";


		//rez += "densityBias: "; rez += std::to_string(densityBias); rez += ";\n";
		//rez += "densityBiasPower: "; rez += std::to_string(densityBiasPower); rez += ";\n";

		//rez += "densitySquishFactor: "; rez += std::to_string(densitySquishFactor); rez += ";\n";
		//rez += "densitySquishPower: "; rez += std::to_string(densitySquishPower); rez += ";\n";
		//rez += "densityHeightoffset: "; rez += std::to_string(densityHeightoffset); rez += ";\n";

		rez += "regionsHeightSpline: "; rez += regionsHeightSpline.saveSettings(1);
	}



	return rez;
}


void WorldGeneratorSettings::sanitize()
{
	continentalnessNoiseSettings.sanitize();
	peaksAndValies.sanitize();
	wierdness.sanitize();
	stone3Dnoise.sanitize();
	riversNoise.sanitize();
	vegetationNoise.sanitize();
	treesTypeNoise.sanitize();
	treesAmountNoise.sanitize();
	hillsDrops.sanitize();
	randomSand.sanitize();
	stonePatches.sanitize();
	cavesNoise.sanitize();
	lakesNoise.sanitize();

	peaksAndValiesContributionSpline.sanitize();

	//densityBias = glm::clamp(densityBias, 0.f, 1.f);
	//densityBiasPower = glm::clamp(densityBiasPower, 0.1f, 10.f);


	spagettiNoise.sanitize();
	//spagettiBias = glm::clamp(spagettiBias, 0.f, 10.f);
	//spagettiBiasPower = glm::clamp(spagettiBiasPower, 0.1f, 10.f);

}


std::string NoiseSetting::saveSettings(int tabs, bool saveSpline)
{
	std::string rez;
	rez.reserve(200);


	auto addTabs = [&]() { for (int i = 0; i < tabs; i++) { rez += '\t'; } };

	addTabs();
	rez += "{\n";

	addTabs();
	rez += "scale: "; rez += std::to_string(scale); rez += ";\n"; addTabs();
	rez += "type: "; rez += magic_enum::enum_name((FastNoiseSIMD::NoiseType)type); rez += ";\n"; addTabs();
	rez += "celularReturnType: "; rez += magic_enum::enum_name((FastNoiseSIMD::CellularReturnType)cellularReturnType); rez += ";\n"; addTabs();
	rez += "frequency: "; rez += std::to_string(frequency); rez += ";\n"; addTabs();
	rez += "octaves: "; rez += std::to_string(octaves); rez += ";\n"; addTabs();
	rez += "perturbFractalOctaves: "; rez += std::to_string(perturbFractalOctaves); rez += ";\n"; addTabs();
	rez += "power: "; rez += std::to_string(power); rez += ";\n"; 

	if (saveSpline)
	{
		addTabs();
		rez += "spline:\n";
		rez += spline.saveSettings(tabs + 1);
	}
	
	addTabs();
	rez += "}\n";


	return rez;
}

void NoiseSetting::sanitize()
{
	spline.sanitize();

	scale = glm::clamp(scale, 0.001f, 100.f);
	type = glm::clamp(type, (int)FastNoiseSIMD::Value, (int)FastNoiseSIMD::CubicFractal);
	frequency = glm::clamp(frequency, 0.001f, 100.f);
	octaves = glm::clamp(octaves, 0, 8);
	perturbFractalOctaves = glm::clamp(perturbFractalOctaves, 0, 8);
	power = glm::clamp(power, 0.1f, 10.f);
	cellularReturnType = glm::clamp(cellularReturnType, (int)FastNoiseSIMD::CellularReturnType::CellValue, (int)FastNoiseSIMD::CellularReturnType::Distance2Cave);


}


bool WorldGeneratorSettings::loadSettings(const char *data)
{
	*this = {};

	enum
	{
		TokenNone,
		TokenString,
		TokenNumber,
		TokenSymbol,
	};

	struct Token
	{
		int type = 0;
		std::string s = "";
		char symbol = 0;
		double number = 0;

		bool operator==(Token &other)
		{
			return type == other.type &&
				s == other.s &&
				symbol == other.symbol &&
				number == other.number;
		}
	};

	std::vector<Token> tokens;

#pragma region tokenize
	{
		int lastPos = 0;
		for (int i = 0; data[i] != '\0';)
		{
			auto checkSymbol = [&]() -> bool
			{
				if (data[i] == ';' || data[i] == ':' || data[i] == '{' || data[i] == '}' || data[i] == ',')
				{
					tokens.push_back({Token{TokenSymbol, "", data[i], 0}});
					i++;
					return true;
				}
				return 0;
			};

			auto checkNumber = [&]() -> bool
			{
				if (std::isdigit(data[i]) || (data[i] == '-' && std::isdigit(data[i+1])) )
				{
					Token t;
					t.type = TokenNumber;
					t.s += data[i];
					i++;

					for (; data[i] != '\0'; i++)
					{
						if (std::isdigit(data[i]) || data[i] == '.')
						{
							t.s += data[i];
						}
						else
						{
							break;
						}
					}
					t.number = std::stod(t.s); //todo replace with something that reports errors

					if (!t.s.empty()) { tokens.push_back(t); }

					return true;
				}

				return 0;
			};

			auto consumeWhiteSpaces = [&]()
			{
				for (; data[i] != '\0'; i++)
				{
					if (data[i] != '\n' && data[i] != '\v' && data[i] != '\t' && data[i] != ' ' && data[i] != '\r')
					{
						break;
					}
				}
			};

			auto addString = [&]()
			{
				Token t;
				t.type = TokenString;

				for (; data[i] != '\0'; i++)
				{
					if (isalnum(data[i]))
					{
						t.s += data[i];
					}
					else
					{
						break;
					}
				}

				if (!t.s.empty()) { tokens.push_back(t); }
			};

			consumeWhiteSpaces();
			if (data[i] == '\0') { break; }

			if (checkSymbol())continue;
			if (checkNumber())continue;
			addString();

			if (lastPos == i)
			{
				return 0;
			}
			else
			{
				lastPos = i;
			}
		}
	}
#pragma endregion


#pragma region parse
	{

		for (int i = 0; i < tokens.size();)
		{

			auto isString = [&]() -> bool
			{
				return tokens[i].type == TokenString;
			};

			auto consume = [&](Token t) -> bool
			{
				if (i >= tokens.size()) { return false; }

				if (tokens[i] == t)
				{
					i++;
					return true;
				}
				else
				{
					return false;
				}
			};

			auto isEof = [&]() { return i >= tokens.size(); };

			auto isNumber = [&]()
			{
				return (tokens[i].type == TokenNumber);
			};

			auto consumeNumber = [&]() -> double
			{
				if (tokens[i].type == TokenNumber)
				{
					i++;
					return tokens[i - 1].number;
				};
				assert(0);
				return 0;
			};

			auto consumeSpline = [&](Spline &spline) -> bool
			{
				spline.size = 0;

				if (!consume(Token{TokenSymbol, "", '{', 0})) { return 0; }

				while (!consume(Token{TokenSymbol, "", '}', 0}))
				{
					if (isEof()) { return 0; }

					float nrA = 0;
					float nrB = 0;

					if (!isNumber()) { return 0; }
					nrA = consumeNumber();
					if (!consume(Token{TokenSymbol, "", ',', 0})) { return 0; }
					if (!isNumber()) { return 0; }
					nrB = consumeNumber();
					if (!consume(Token{TokenSymbol, "", ';', 0})) { return 0; }

					if (spline.size >= MAX_SPLINES_COUNT)
					{
						return 0;
					}

					spline.points[spline.size] = glm::vec2(nrA, nrB);
					spline.size++;
				}

				return true;
			};

			auto consumeNoise = [&](NoiseSetting &settings) -> bool
			{
				if (!consume(Token{TokenSymbol, "", '{', 0})) { return 0; }

				while (!consume(Token{TokenSymbol, "", '}', 0}))
				{
					if (isEof()) { return 0; }

					if (isString())
					{
						auto s = tokens[i].s; //todo to lower
						i++;

						if (s == "scale")
						{
							if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
							if (isEof()) { return 0; }
							if (!isNumber()) { return 0; }
							settings.scale = consumeNumber();
							if (!consume(Token{TokenSymbol, "", ';', 0})) { return 0; }
						}
						else if (s == "frequency")
						{
							if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
							if (isEof()) { return 0; }
							if (!isNumber()) { return 0; }
							settings.frequency = consumeNumber();
							if (!consume(Token{TokenSymbol, "", ';', 0})) { return 0; }
						}
						else if (s == "octaves")
						{
							if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
							if (isEof()) { return 0; }
							if (!isNumber()) { return 0; }
							settings.octaves = consumeNumber();
							if (!consume(Token{TokenSymbol, "", ';', 0})) { return 0; }
						}
						else if (s == "perturbFractalOctaves")
						{
							if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
							if (isEof()) { return 0; }
							if (!isNumber()) { return 0; }
							settings.perturbFractalOctaves = consumeNumber();
							if (!consume(Token{TokenSymbol, "", ';', 0})) { return 0; }
						}
						else if (s == "power")
						{
							if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
							if (isEof()) { return 0; }
							if (!isNumber()) { return 0; }
							settings.power = consumeNumber();
							if (!consume(Token{TokenSymbol, "", ';', 0})) { return 0; }
						}
						else if (s == "type")
						{
							if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
							if (isEof()) { return 0; }

							if (!isString()) { return 0; }

							auto s = tokens[i].s; //todo to lower
							i++;
							settings.type = -1;

							const char *types[] = {
								"Value", "ValueFractal", "Perlin", "PerlinFractal", "Simplex",
								"SimplexFractal", "WhiteNoise", "Cellular", "Cubic", "CubicFractal"};

							for (int t = 0; t < sizeof(types) / sizeof(types[0]); t++)
							{
								if (s == types[t])
								{
									settings.type = t;
									break;
								}
							}

							if (settings.type < 0) { return 0; }

							if (!consume(Token{TokenSymbol, "", ';', 0})) { return 0; }
						}
						else if (s == "celularReturnType")
						{
							if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
							if (isEof()) { return 0; }

							if (!isString()) { return 0; }

							auto s = tokens[i].s; //todo to lower
							i++;
							settings.cellularReturnType = -1;

							const char *types[] = {
								"CellValue", "Distance", 
								"Distance2", "Distance2Add", "Distance2Sub", 
								"Distance2Mul", "Distance2Div", "NoiseLookup", "Distance2Cave"};

							for (int t = 0; t < sizeof(types) / sizeof(types[0]); t++)
							{
								if (s == types[t])
								{
									settings.cellularReturnType = t;
									break;
								}
							}

							if (settings.cellularReturnType < 0) { return 0; }

							if (!consume(Token{TokenSymbol, "", ';', 0})) { return 0; }
						}
						else if (s == "spline")
						{
							if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
							if (isEof()) { return 0; }
							if (!consumeSpline(settings.spline)) { return 0; }
						}
						else
						{
							return 0;
						}


					}
					else if (consume(Token{TokenSymbol, "", ';', 0}))
					{

					}
					else
					{
						return 0;
					}

				}

				return true;
			};

			//if (isEof()) { break; }

			if (isString())
			{
				auto s = tokens[i].s; //todo to lower
				i++;

				if (s == "seed")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }
					if (!isNumber()) { return 0; }
					seed = consumeNumber();
					if (!consume(Token{TokenSymbol, "", ';', 0})) { return 0; }
				}else if (s == "superFlat") 
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }
					if (!isNumber()) { return 0; }
					isSuperFlat = consumeNumber();
					if (!consume(Token{TokenSymbol, "", ';', 0})) { return 0; }
				}
				else if (s == "continentalnessNoise")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(continentalnessNoiseSettings))
					{
						return 0;
					}
				}
				else if (s == "continentalnessNoise2")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(continentalness2NoiseSettings))
					{
						return 0;
					}
				}
				else if (s == "continentalnessPickNoise")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(continentalnessPickSettings))
					{
						return 0;
					}
				}
				else if (s == "swampNoise")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(swampNoise))
					{
						return 0;
					}
				}
				else if (s == "stoneSpikesNoise")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(stoneSpikesNoise))
					{
						return 0;
					}
				}
				else if (s == "stoneSpikesMask")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(stoneSpikeMask))
					{
						return 0;
					}
				}
				else if (s == "swampMask")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(swampMask))
					{
						return 0;
					}
				}
				else if (s == "iceNoise")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(iceNoise))
					{
						return 0;
					}
				}
				else if (s == "vegetationNoise")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(vegetationNoise))
					{
						return 0;
					}
				}
				else if (s == "riversNoise")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(riversNoise))
					{
						return 0;
					}
				}
				else if (s == "roadsNoise")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(roadsNoise))
					{
						return 0;
					}
				}
				else if (s == "stonePatches")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(stonePatches))
					{
						return 0;
					}
				}
				else if (s == "cavesNoise")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(cavesNoise))
					{
						return 0;
					}
				}
				else if (s == "lakesNoise")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(lakesNoise))
					{
						return 0;
					}
				}
				else if (s == "randomSand")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(randomSand))
					{
						return 0;
					}
				}
				else if (s == "peaksAndValies")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(peaksAndValies))
					{
						return 0;
					}
				}
				else if (s == "randomHills")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(randomHills))
					{
						return 0;
					}
					}
				else if (s == "wierdness")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(wierdness))
					{
						return 0;
					}
				}
				else if (s == "stonetDnoise")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(stone3Dnoise))
					{
						return 0;
					}
				}
				else if (s == "spagettiNoise")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(spagettiNoise))
					{
						return 0;
					}
				}
				else if (s == "hillsDrop")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(hillsDrops))
					{
						return 0;
					}
				}
				else if (s == "treesAmountNoise")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(treesAmountNoise))
					{
						return 0;
					}
				}
				else if (s == "treesTypeNoise")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(treesTypeNoise))
					{
						return 0;
					}
				}
				else if (s == "peaksAndValiesContributionSpline")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeSpline(peaksAndValiesContributionSpline))
					{
						return 0;
					}
				}
				else if (s == "regionsHeightSpline")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeSpline(regionsHeightSpline))
					{
						return 0;
					}
				}
				else
				{
					return 0;
				}
			}
			else if (consume(Token{TokenSymbol, "", ';', 0}))
			{

			}
			else
			{

				int test = 0;
				return 0;
			}


		}



	}
#pragma endregion



	sanitize();

	return true;
}


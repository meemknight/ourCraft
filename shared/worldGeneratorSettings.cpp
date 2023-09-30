#include "worldGeneratorSettings.h"
#include <FastNoiseSIMD.h>
#include <magic_enum.hpp>
#include <glm/glm.hpp>

void WorldGenerator::init()
{
	continentalnessNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	peaksValiesNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	oceansAndTerasesNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	stone3Dnoise = FastNoiseSIMD::NewFastNoiseSIMD();
	vegetationNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	whiteNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	whiteNoise2 = FastNoiseSIMD::NewFastNoiseSIMD();
	spagettiNoise = FastNoiseSIMD::NewFastNoiseSIMD();

	temperatureNoise = FastNoiseSIMD::NewFastNoiseSIMD();
	humidityNoise = FastNoiseSIMD::NewFastNoiseSIMD();

	WorldGeneratorSettings s;
	applySettings(s);
}

void WorldGenerator::clear()
{
	delete continentalnessNoise;
	delete peaksValiesNoise;
	delete oceansAndTerasesNoise;
	delete stone3Dnoise;
	delete vegetationNoise;
	delete whiteNoise;
	delete whiteNoise2;
	delete spagettiNoise;
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
	};

	apply(continentalnessNoise, s.seed + 2, s.continentalnessNoiseSettings);
	continentalSplines = s.continentalnessNoiseSettings.spline;
	continentalPower = s.continentalnessNoiseSettings.power;

	apply(stone3Dnoise, s.seed + 3, s.stone3Dnoise);
	stone3DnoiseSplines = s.stone3Dnoise.spline;
	stone3Dpower = s.stone3Dnoise.power;
	densityBias = s.densityBias;
	densityBiasPower = s.densityBiasPower;

	apply(spagettiNoise, s.seed + 4, s.spagettiNoise);
	spagettiNoiseSplines = s.spagettiNoise.spline;
	spagettiNoisePower = s.spagettiNoise.power;
	spagettiNoiseBias = s.spagettiBias;
	spagettiNoiseBiasPower = s.spagettiBiasPower;

	apply(peaksValiesNoise, s.seed + 5, s.peaksAndValies);
	peaksValiesSplines = s.peaksAndValies.spline;
	peaksValiesPower = s.peaksAndValies.power;
	peaksValiesContributionSplines = s.peaksAndValiesContributionSpline;

	apply(oceansAndTerasesNoise, s.seed + 6, s.oceansAndTerases);
	oceansAndTerasesSplines = s.oceansAndTerases.spline;
	oceansAndTerasesPower = s.oceansAndTerases.power;
	oceansAndTerasesContributionSplines = s.oceansAndTerasesContributionSpline;

	apply(vegetationNoise, s.seed + 7, s.vegetationNoise);
	vegetationPower = s.vegetationNoise.power;
	vegetationSplines = s.vegetationNoise.spline;


	apply(temperatureNoise, s.seed + 8, s.temperatureNoise);
	temperaturePower = s.temperatureNoise.power;
	temperatureSplines = s.temperatureNoise.spline;

	apply(humidityNoise, s.seed + 9, s.humidityNoise);
	humidityPower = s.humidityNoise.power;
	humiditySplines = s.humidityNoise.spline;

}


std::string WorldGeneratorSettings::saveSettings()
{

	std::string rez;
	rez.reserve(500);


	rez += "seed: "; rez += std::to_string(seed); rez += ";\n";

	rez += "continentalnessNoise:\n";
	rez += continentalnessNoiseSettings.saveSettings(1);

	rez += "peaksAndValies:\n";
	rez += peaksAndValies.saveSettings(1);
	rez += "peaksAndValiesContributionSpline:\n";
	rez += peaksAndValiesContributionSpline.saveSettings(1);

	rez += "oceansAndTerases:\n";
	rez += oceansAndTerases.saveSettings(1);
	rez += "oceansAndTerasesContributionSpline:\n";
	rez += oceansAndTerasesContributionSpline.saveSettings(1);

	rez += "vegetationNoise:\n";
	rez += vegetationNoise.saveSettings(1);

	rez += "stonetDnoise:\n";
	rez += stone3Dnoise.saveSettings(1);

	rez += "humidityNoise:\n";
	rez += humidityNoise.saveSettings(1);

	rez += "temperatureNoise:\n";
	rez += temperatureNoise.saveSettings(1);

	rez += "spagettiNoise:\n";
	rez += spagettiNoise.saveSettings(1);
	rez += "spagettiBias: "; rez += std::to_string(spagettiBias); rez += ";\n";
	rez += "spagettiBiasPower: "; rez += std::to_string(spagettiBiasPower); rez += ";\n";


	rez += "densityBias: "; rez += std::to_string(densityBias); rez += ";\n";
	rez += "densityBiasPower: "; rez += std::to_string(densityBiasPower); rez += ";\n";

	return rez;
}

void WorldGeneratorSettings::sanitize()
{
	continentalnessNoiseSettings.sanitize();
	peaksAndValies.sanitize();
	oceansAndTerases.sanitize();
	stone3Dnoise.sanitize();
	humidityNoise.sanitize();
	temperatureNoise.sanitize();
	vegetationNoise.sanitize();
	
	peaksAndValiesContributionSpline.sanitize();
	oceansAndTerasesContributionSpline.sanitize();

	densityBias = glm::clamp(densityBias, 0.f, 1.f);
	densityBiasPower = glm::clamp(densityBiasPower, 0.1f, 10.f);


	spagettiNoise.sanitize();
	spagettiBias = glm::clamp(spagettiBias, 0.f, 10.f);
	spagettiBiasPower = glm::clamp(spagettiBiasPower, 0.1f, 10.f);

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

	//todo the rest

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
				if (std::isdigit(data[i]))
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
					if (isalpha(data[i]))
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
					return tokens[i-1].number;
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
						}else if (s == "frequency")
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
				}else if (s == "densityBias")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }
					if (!isNumber()) { return 0; }
					densityBias = consumeNumber();
					if (!consume(Token{TokenSymbol, "", ';', 0})) { return 0; }
				}else if (s == "densityBiasPower")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }
					if (!isNumber()) { return 0; }
					densityBiasPower = consumeNumber();
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
				else if (s == "vegetationNoise")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(vegetationNoise))
					{
						return 0;
					}
				}
				else if (s == "humidityNoise")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(humidityNoise))
					{
						return 0;
					}
				}
				else if (s == "temperatureNoise")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(temperatureNoise))
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
				else if (s == "oceansAndTerases")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeNoise(oceansAndTerases))
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
				else if (s == "spagettiBias")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }
					if (!isNumber()) { return 0; }
					spagettiBias = consumeNumber();
					if (!consume(Token{TokenSymbol, "", ';', 0})) { return 0; }
				}
				else if (s == "spagettiBiasPower")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }
					if (!isNumber()) { return 0; }
					spagettiBiasPower = consumeNumber();
					if (!consume(Token{TokenSymbol, "", ';', 0})) { return 0; }
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
				else if (s == "oceansAndTerasesContributionSpline")
				{
					if (!consume(Token{TokenSymbol, "", ':', 0})) { return 0; }
					if (isEof()) { return 0; }

					if (!consumeSpline(oceansAndTerasesContributionSpline))
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

			}else
			{
				return 0;
			}


		}



	}
#pragma endregion



	sanitize();

	return true;
}


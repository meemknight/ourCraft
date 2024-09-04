#define GLM_ENABLE_EXPERIMENTAL
#include "gameLayer.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "platformInput.h"
#include "imgui.h"
#include <iostream>
#include <sstream>
#include <gl2d/gl2d.h>
#include <worldGeneratorSettings.h>
#include <bezie.h>
#include <fstream>


gl2d::Renderer2D renderer;
WorldGenerator wg;
WorldGeneratorSettings settings;

glm::ivec2 size = {512,512};
glm::ivec2 displacement = {};

void renderSettingsForOneNoise(const char *name, NoiseSetting &n)
{
	ImGui::PushID((void*)name);

	ImGui::Text(name);

	ImGui::DragFloat("Scale: ", &n.scale, 0.01);
	ImGui::Combo("Noise Type", &n.type, 
		"Value\0ValueFractal\0Perlin\0PerlinFractal\0Simplex\0SimplexFractal\0WhiteNoise\0Cellular\0Cubic\0CubicFractal");
	ImGui::DragFloat("Frequency: ", &n.frequency, 0.0001, 0.0001);
	ImGui::DragInt("Octaves", &n.octaves, 0.5, 0, 8);
	ImGui::DragInt("Perturb Fractal Octaves", &n.perturbFractalOctaves, 0.5, 0, 8);
	ImGui::DragFloat("Power", &n.power, 0.1, 0.1, 10);

	ImGui::PopID();
}

void recreate();

bool showFinal = 0;
bool showSlice = 0;
bool showContinentalness = 0;
bool showPeaksAndValies = 0;
bool showWierdness = 0;
bool showVegetation = 0;
bool show3DStone = 0;
bool showSpagetti = 0;

bool showHumidityAndTemperature = 0;


bool initGame()
{
	//initializing stuff for the renderer
	gl2d::init();
	wg.init();

	recreate();


	return true;
}

gl2d::Texture noiseT;
gl2d::Texture peaksValiesT;
gl2d::Texture finalTexture;
gl2d::Texture sliceT;
gl2d::Texture stone3DNoiseT;
gl2d::Texture wierdnessT;
gl2d::Texture vegetationT;
gl2d::Texture spagettiT;

gl2d::Texture humidityT;
gl2d::Texture temperatureT;

gl2d::Texture fractalT;


void createFromFloats(gl2d::Texture &t, float *data, glm::ivec2 s)
{
	t.cleanup();

	GLuint id = 0;

	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s.x, s.y, 0, GL_RGBA, GL_FLOAT, data);

	t.id = id;
}

void createFromGrayScale(gl2d::Texture &t, float *data, glm::ivec2 s)
{

	float *dataConverted = new float[s.x * s.y * 4];
	{
		int j = 0;
		for (int i = 0; i < s.x * s.y; i++)
		{
			dataConverted[j++] = data[i];
			dataConverted[j++] = data[i];
			dataConverted[j++] = data[i];
			dataConverted[j++] = 1.f;
		}
	}

	createFromFloats(t, dataConverted, s);

	delete[] dataConverted;
}

constexpr int startLevel = 45;
constexpr int waterLevel = 65;
constexpr int maxMountainLevel = 220;
constexpr int heightDiff = maxMountainLevel - startLevel;


void recreate()
{

	float *fractalTestNoise = 0;
	float *smallerFractalNoise = 0;

	{
		if (1)
		{
			fractalTestNoise = new float[size.x * size.y] {};

			smallerFractalNoise
				= wg.regionsHeightNoise->GetNoiseSet(displacement.x, 0, displacement.y,
				size.y / 16, (1), size.x / 16, 1);

			for (int i = 0; i < (size.x / 16) * (size.y / 16); i++)
			{
				smallerFractalNoise[i] = wg.regionsHeightSplines.applySpline((smallerFractalNoise[i] + 1) / 2.f);

				//smallerFractalNoise[i] *= 6;
				//smallerFractalNoise[i] = floor(smallerFractalNoise[i]);
				//smallerFractalNoise[i] /= 6;
			}

			for (int j = 0; j < size.y; j++)
				for (int i = 0; i < size.x; i++)
				{
					int sampleI = i / 16;
					int sampleJ = j / 16;

					if (sampleI < (size.x / 16) && sampleJ < (size.y / 16))
					{
						fractalTestNoise[i + j * size.x] =
							smallerFractalNoise[sampleI + sampleJ * (size.x / 16)];
					}
				}


			createFromGrayScale(fractalT, fractalTestNoise, size);

			FastNoiseSIMD::FreeNoiseSet(smallerFractalNoise);
			delete[] fractalTestNoise;

		}
		else
		{
			fractalTestNoise
				= wg.regionsHeightNoise->GetNoiseSet(displacement.x, 0, displacement.y,
				size.y, (1), size.x, 1);

			for (int i = 0; i < size.x * size.y; i++)
			{
				fractalTestNoise[i] = wg.regionsHeightSplines.applySpline((fractalTestNoise[i] + 1) / 2.f);

				fractalTestNoise[i] *= 6;
				fractalTestNoise[i] = floor(fractalTestNoise[i]);
				fractalTestNoise[i] /= 6;
			}

			createFromGrayScale(fractalT, fractalTestNoise, size);

			FastNoiseSIMD::FreeNoiseSet(fractalTestNoise);
		}

	}





	float *finalNoise = new float[size.x * size.y];

	float *testNoise
		= wg.continentalnessNoise->GetNoiseSet(displacement.x, 0, displacement.y, size.y, (1), size.x, 1);
	for (int i = 0; i < size.x * size.y; i++)
	{
		testNoise[i] += 1;
		testNoise[i] /= 2;
		testNoise[i] = std::pow(testNoise[i], settings.continentalnessNoiseSettings.power);
		testNoise[i] = applySpline(testNoise[i], settings.continentalnessNoiseSettings.spline);
	}
	createFromGrayScale(noiseT, testNoise, size);

	float *humidityNoise
		= wg.humidityNoise->GetNoiseSet(displacement.x, 0, displacement.y, size.y, (1), size.x, 1);
	for (int i = 0; i < size.x * size.y; i++)
	{
		humidityNoise[i] += 1;
		humidityNoise[i] /= 2;
		humidityNoise[i] = std::pow(humidityNoise[i], settings.humidityNoise.power);
		humidityNoise[i] = applySpline(humidityNoise[i], settings.humidityNoise.spline);
	}
	createFromGrayScale(humidityT, humidityNoise, size);

	float *temperatureNoise
		= wg.temperatureNoise->GetNoiseSet(displacement.x, 0, displacement.y, size.y, (1), size.x, 1);
	for (int i = 0; i < size.x * size.y; i++)
	{
		temperatureNoise[i] += 1;
		temperatureNoise[i] /= 2;
		temperatureNoise[i] = std::pow(temperatureNoise[i], settings.temperatureNoise.power);
		temperatureNoise[i] = applySpline(temperatureNoise[i], settings.temperatureNoise.spline);
	}
	createFromGrayScale(temperatureT, temperatureNoise, size);

	
	float *peaksNoise;
	{
		peaksNoise
			= wg.peaksValiesNoise->GetNoiseSet(displacement.x, 0, displacement.y, size.y, (1), size.x, 1);

		for (int i = 0; i < size.x * size.y; i++)
		{
			peaksNoise[i] += 1;
			peaksNoise[i] /= 2;
			peaksNoise[i] = std::pow(peaksNoise[i], settings.peaksAndValies.power);
			float val = peaksNoise[i];
			peaksNoise[i] = applySpline(peaksNoise[i], settings.peaksAndValies.spline);

			finalNoise[i] = lerp(testNoise[i], peaksNoise[i], settings.peaksAndValiesContributionSpline.applySpline(val));

		}
		createFromGrayScale(peaksValiesT, peaksNoise, size);

	};

	float *wierdnessNoise;
	{
		wierdnessNoise
			= wg.wierdnessNoise->GetNoiseSet(displacement.x, 0, displacement.y, size.y, (1), size.x, 1);

		for (int i = 0; i < size.x * size.y; i++)
		{
			wierdnessNoise[i] += 1;
			wierdnessNoise[i] /= 2;
			wierdnessNoise[i] = std::pow(wierdnessNoise[i], settings.wierdness.power);
			float val = wierdnessNoise[i];
			wierdnessNoise[i] = applySpline(wierdnessNoise[i], settings.wierdness.spline);

			//finalNoise[i] = lerp(finalNoise[i], wierdnessNoise[i], settings.oceansAndTerasesContributionSpline.applySpline(val));

		}
		createFromGrayScale(wierdnessT, wierdnessNoise, size);


	}

	auto getWierdness = [&](int x, int y) 
	{
		return wierdnessNoise[y * size.x + x];
	};

	{
		float *vegetationNoise;

		vegetationNoise
			= wg.vegetationNoise->GetNoiseSet(displacement.x, 0, displacement.y, size.y, (1), size.x, 1);

		for (int i = 0; i < size.x * size.y; i++)
		{
			vegetationNoise[i] += 1;
			vegetationNoise[i] /= 2;
			vegetationNoise[i] = std::pow(vegetationNoise[i], settings.vegetationNoise.power);
			vegetationNoise[i] = applySpline(vegetationNoise[i], settings.vegetationNoise.spline);

		}

		createFromGrayScale(vegetationT, vegetationNoise, size);
		
		FastNoiseSIMD::FreeNoiseSet(vegetationNoise);


	}

	float *stoneNoise;
	{

		stoneNoise
			= wg.stone3Dnoise->GetNoiseSet(displacement.x, 0, displacement.y, 1, 256, size.x, 1);

		for (int i = 0; i < size.x * 256; i++)
		{
			stoneNoise[i] += 1;
			stoneNoise[i] /= 2;
			stoneNoise[i] = std::pow(stoneNoise[i], settings.stone3Dnoise.power);
			stoneNoise[i] = applySpline(stoneNoise[i], settings.stone3Dnoise.spline);
		}
		createFromGrayScale(stone3DNoiseT, stoneNoise, glm::ivec2{size.x,256});

	}

	float *spagettiNoise;
	{
		spagettiNoise
			= wg.spagettiNoise->GetNoiseSet(displacement.x, 0, displacement.y, 1, 256, size.x, 1);

		for (int i = 0; i < size.x * 256; i++)
		{
			spagettiNoise[i] += 1;
			spagettiNoise[i] /= 2;
			spagettiNoise[i] = std::pow(spagettiNoise[i], settings.spagettiNoise.power);
			spagettiNoise[i] = applySpline(spagettiNoise[i], settings.spagettiNoise.spline);
		}

		if (showSpagetti)
		{
			createFromGrayScale(spagettiT, spagettiNoise, glm::ivec2{size.x,256});
		}
	}

	createFromGrayScale(finalTexture, finalNoise, size);

	{

		auto getDensityNoiseVal = [&](int x, int y) //todo more cache friendly operation here please
		{
			return stoneNoise[y * size.x + x];
		};

		auto getSpagettiNoiseVal = [&](int x, int y) //todo more cache friendly operation here please
		{
			return spagettiNoise[y * size.x + x];
		};


		glm::vec4 *colorData = new glm::vec4[256 * size.x]{};
		for (int i = 0; i < size.x; i++)
		{

			constexpr int stoneNoiseStartLevel = 1;

			float heightPercentage = finalNoise[i + size.y - 1];
			int height = int(startLevel + heightPercentage * heightDiff);

			float squishFactor = settings.densitySquishFactor + getWierdness(i,0) * 30 - 15;
			squishFactor = std::max(squishFactor, 0.f);


			float firstH = 1;
			for (int y = 0; y < 256; y++)
			{

				auto density = getDensityNoiseVal(i, y);

				int heightOffset = height + settings.densityHeightoffset;
				int difference = y - heightOffset;
				float differenceMultiplier = 
					glm::clamp(pow(abs(difference)/ squishFactor, settings.densitySquishPower),
					1.f, 10.f);

				if (difference > 0)
				{
					density = powf(density, differenceMultiplier);
				}
				else if (difference < 0)
				{
					density = powf(density, 1.f/(differenceMultiplier));
				}


				if (y < stoneNoiseStartLevel)
				{
					colorData[y * size.x + i] = glm::vec4(77, 73, 70, 255) / 255.f; //stone
				}
				else
				{
					if (density > 0.5)
					{
						firstH = y;
						colorData[y * size.x + i] = glm::vec4(77, 73, 70, 255) / 255.f; //stone
					}
					else
					{
						colorData[y * size.x + i] = glm::vec4(7, 7, 7, 255) / 255.f; //cave
					}
				}

				//float bias = (y - stoneNoiseStartLevel) / (height - 1.f - stoneNoiseStartLevel);
				//
				//bias = std::powf(bias, wg.densityBiasPower);
				////density = std::powf(density, wg.densityBiasPower);
				//
				//if (y < stoneNoiseStartLevel)
				//{
				//	colorData[y * size.x + i] = glm::vec4(77, 73, 70, 255) / 255.f; //stone
				//}
				//else
				//{
				//	if (density > wg.densityBias * bias)
				//	{
				//		firstH = y;
				//		colorData[y * size.x + i] = glm::vec4(77, 73, 70, 255) / 255.f; //stone
				//	}
				//	else
				//	{
				//		colorData[y * size.x + i] = glm::vec4(7, 7, 7, 255) / 255.f; //cave
				//	}
				//}
				
			}

			int y = 255;
			for (; y >= 1; y--)
			{
			
				if (colorData[y * size.x + i].r < 9/255.f)
				{
					colorData[y * size.x + i] = glm::vec4(101, 154, 207, 255) / 255.f; //sky
				}
				else
				{
					break;
				}
			}
			
			if (y >= waterLevel)
			{
				if (colorData[y * size.x + i].r > 2 / 255.f)
				{
					colorData[y * size.x + i] = glm::vec4(10, 84, 18, 255) / 255.f; //grass
					y--;

					int start = y;
					for (; y >= start - 4; y--)
					{
						if (colorData[y * size.x + i].r > 2 / 255.f)
						{
							colorData[y * size.x + i] = glm::vec4(120, 66, 26, 255) / 255.f; //dirt
						}
						else
						{
							break;
						}
					}
				}
			}

			//
			for (y=waterLevel; y >= 0; y--)
			{
				if (colorData[y * size.x + i].b > 200.f / 255.f)
				{
					colorData[y * size.x + i] = glm::vec4(11, 16, 77, 255) / 255.f; //water
				}
				else
				{
					break;
				}
			}
			
			
			for (int y = 0; y < firstH; y++)
			{
				auto density = getSpagettiNoiseVal(i, y);
				float bias = (y - 1) / (256 - 1.f - 1);
				bias = glm::clamp(bias, 0.f, 1.f);
				bias = 1.f - bias;

				bias = powf(bias, wg.spagettiNoiseBiasPower);

				if (density > wg.spagettiNoiseBias * bias)
				{
					//stone
				}
				else
				{
					if (colorData[y * size.x + i] != glm::vec4(11, 16, 77, 255) / 255.f) //not water
					{
						colorData[y * size.x + i] = glm::vec4(7, 7, 7, 255) / 255.f; //cave
					}
				}

			}

		}

		createFromFloats(sliceT, &colorData->x, {size.x, 256});

		delete[] colorData;
	}

	FastNoiseSIMD::FreeNoiseSet(testNoise);
	FastNoiseSIMD::FreeNoiseSet(peaksNoise);
	FastNoiseSIMD::FreeNoiseSet(wierdnessNoise);
	FastNoiseSIMD::FreeNoiseSet(stoneNoise);
	FastNoiseSIMD::FreeNoiseSet(humidityNoise);
	FastNoiseSIMD::FreeNoiseSet(temperatureNoise);
	FastNoiseSIMD::FreeNoiseSet(spagettiNoise);


	delete[] finalNoise;
}


void drawNoise(const char *name, gl2d::Texture t)
{


	ImGui::Text(name);

	auto cursor = ImGui::GetCursorPos();
	ImGui::Image((ImTextureID)t.id, {(float)t.GetSize().x, (float)t.GetSize().y},
		{}, {1,1}, {1,1,1,1}, {1,1,1,1});


	auto draw_list = ImGui::GetWindowDrawList();
	auto immagePos = ImGui::GetWindowPos();
	
	cursor.x += immagePos.x;
	cursor.y += immagePos.y;

	draw_list->AddRect(cursor, {cursor.x+16, cursor.y+16}, IM_COL32(255,0,0,128));
	draw_list->AddRect(cursor, {cursor.x+16*10, cursor.y+16*10}, IM_COL32(0,255,0,128));


}

bool gameLogic(float deltaTime)
{
#pragma region init stuff
	int w = 0; int h = 0;
	w = platform::getFrameBufferSizeX(); //window w
	h = platform::getFrameBufferSizeY(); //window h

	glViewport(0, 0, w, h);
	glClear(GL_COLOR_BUFFER_BIT); //clear screen

#pragma endregion



	ImGui::Begin("Settings");

	ImGui::DragInt("Size", &size[0]);
	size[1] = size[0];
	size = glm::clamp(size, glm::ivec2(1, 1), glm::ivec2(2048, 2048));

	ImGui::DragInt2("Displacement", &displacement[0], 16);

	if (ImGui::Button("Save"))
	{
		std::ofstream f(MINECRAFT_RESOURCES_PATH "gameData/worldGenerator/default.mgenerator");
		if (f.is_open())
		{
			f << settings.saveSettings();
			f.close();
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Load"))
	{
		std::ifstream f(MINECRAFT_RESOURCES_PATH "gameData/worldGenerator/default.mgenerator");
		if (f.is_open())
		{
			std::stringstream buffer;
			buffer << f.rdbuf();
			WorldGeneratorSettings s2;
			if (s2.loadSettings(buffer.str().c_str()))
			{
				settings = s2;
			}
			f.close();
		}
	}

	auto splineEditor = [&](Spline &s, const char *name)
	{
		ImGui::Separator();

		ImGui::Table(name, s.points,
			s.size);

		if (ImGui::Button("Add spline"))
		{
			s.addSpline();
		}

		ImGui::SameLine();

		if (ImGui::Button("Remove spline"))
		{
			s.removeSpline();
		}

		ImGui::SameLine();

		if (ImGui::Button("Invert"))
		{
			for (int i = 0; i < s.size; i++)
			{
				auto &spl = s;
				spl.points[i].y = 1 - spl.points[i].y;
			}
		}

		ImGui::Separator();
	};

	auto noiseEditor = [&](NoiseSetting &s, const char *name, bool showSpline = 1)
	{
		ImGui::PushID(name);

		ImGui::Separator();
		renderSettingsForOneNoise(name, s);

		if (showSpline)
			splineEditor(s.spline, name);

		ImGui::PopID();
	};

	if (showContinentalness)
	{
		noiseEditor(settings.continentalnessNoiseSettings, "Continentalness Noise");
	}

	if (showPeaksAndValies)
	{
		noiseEditor(settings.peaksAndValies, "Peaks And Valies");
		splineEditor(settings.peaksAndValiesContributionSpline, "Peaks And Valies spline");
	}

	if (showWierdness)
	{
		ImGui::Separator();

		noiseEditor(settings.wierdness, "Wierdness");
	}

	if (show3DStone)
	{
		noiseEditor(settings.stone3Dnoise, "3D noise");

		ImGui::SliderFloat("3D bias", &settings.densityBias, 0, 1);
		ImGui::SliderFloat("3D noise bias power", &settings.densityBiasPower, 0.1, 10);

		ImGui::SliderFloat("3D noise squish factor", &settings.densitySquishFactor, 0.1, 100);
		ImGui::SliderFloat("3D noise squish power", &settings.densitySquishPower, 0.1, 10);
		ImGui::SliderInt("3D noise density height offset", &settings.densityHeightoffset, -100, 100);

	}

	if (showSpagetti)
	{
		noiseEditor(settings.spagettiNoise, "Spagetti noise");

		ImGui::SliderFloat("Spagetti noise bias", &settings.spagettiBias, 0, 10);
		ImGui::SliderFloat("Spagetti noise bias power", &settings.spagettiBiasPower, 0.1, 10);
	}

	if (showVegetation)
	{
		ImGui::Separator();

		noiseEditor(settings.vegetationNoise, "Vegetation");
	}

	if (showHumidityAndTemperature)
	{
		ImGui::Separator();

		noiseEditor(settings.temperatureNoise, "Temperature");

		ImGui::Separator();

		noiseEditor(settings.humidityNoise, "Humidity");
	}

	{
		//noiseEditor(fractalSettings, "Fractal");

		ImGui::PushID("Fractal");

		splineEditor(settings.regionsHeightSpline, "Fractal height");

		//ImGui::Combo("Noise Type", &n.type,
		//	"Value\0ValueFractal\0Perlin\0PerlinFractal\0Simplex\0SimplexFractal\0WhiteNoise\0Cellular\0Cubic\0CubicFractal");

		ImGui::PopID();

	}


	ImGui::End();

	//ImGui::ShowDemoWindow();
	//ImGui::ShowBezierDemo();

	settings.sanitize();
	wg.applySettings(settings);

	{
		static int counter = 5;
		counter--;
		if (counter <= 0)
		{
			recreate();
			counter = 5;
		}
	}

	ImGui::Begin("Noise Viewer");

	ImGui::Checkbox("showFinal", &showFinal); ImGui::SameLine();
	ImGui::Checkbox("showSlice", &showSlice); ImGui::SameLine();
	ImGui::Checkbox("showContinentalness", &showContinentalness);
	ImGui::Checkbox("showPeaksAndValies", &showPeaksAndValies); ImGui::SameLine();
	ImGui::Checkbox("showWierdness", &showWierdness); ImGui::SameLine();
	ImGui::Checkbox("show3DStone", &show3DStone); ImGui::SameLine();
	ImGui::Checkbox("showVegetation", &showVegetation);
	ImGui::Checkbox("showHumidityAndTemperature", &showHumidityAndTemperature); ImGui::SameLine();
	ImGui::Checkbox("showSpagetti", &showSpagetti);

	if (show3DStone)
	{
		ImGui::Text("3D noise");

		ImGui::Image((ImTextureID)stone3DNoiseT.id, {(float)stone3DNoiseT.GetSize().x, 256},
			{0,1}, {1,0}, {1,1,1,1}, {1,1,1,1});
	}

	if (showSpagetti)
	{
		ImGui::Text("Spagetti noise");

		ImGui::Image((ImTextureID)spagettiT.id, {(float)spagettiT.GetSize().x, 256},
			{0,1}, {1,0}, {1,1,1,1}, {1,1,1,1});
	}

	if (showSlice)
	{
		ImGui::Text("Slice");

		ImGui::Image((ImTextureID)sliceT.id, {(float)sliceT.GetSize().x, 256},
			{0,1}, {1,0}, {1,1,1,1}, {1,1,1,1});
	}

	if (showFinal)
	{
		drawNoise("Result", finalTexture);
	}


	if (showContinentalness)
	{
		drawNoise("Continentalness", noiseT);
	}
	
	if (showPeaksAndValies)
	{
		drawNoise("PeaksValies", peaksValiesT);
	}

	if (showWierdness)
	{
		drawNoise("wierdness noise", wierdnessT);
	}

	if (showVegetation)
	{
		drawNoise("Vegetation", vegetationT);
	}

	if (showHumidityAndTemperature)
	{
		drawNoise("Temperature", temperatureT);
		drawNoise("Humidity", humidityT);
	}

	drawNoise("Fractal", fractalT);

	ImGui::End();



	return true;
#pragma endregion

}

//This function might not be be called if the program is forced closed
void closeGame()
{


}

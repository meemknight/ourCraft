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
	ImGui::DragFloat("Frequency: ", &n.frequency, 0.005, 0.0001);
	ImGui::DragInt("Octaves", &n.octaves, 0.5, 0, 8);
	ImGui::DragInt("Perturb Fractal Octaves", &n.perturbFractalOctaves, 0.5, 0, 8);
	ImGui::DragFloat("Power", &n.power, 0.1, 0.1, 10);

	ImGui::PopID();
}

void recreate();




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

void createFromFloats(gl2d::Texture &t, float *data, glm::ivec2 s)
{
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
	float *peaksNoise;
	{
		peaksNoise
			= wg.peaksValiesNoise->GetNoiseSet(displacement.x, 0, displacement.y, size.y, (1), size.x, 1);

		for (int i = 0; i < size.x * size.y; i++)
		{
			peaksNoise[i] += 1;
			peaksNoise[i] /= 2;
			peaksNoise[i] = std::pow(peaksNoise[i], settings.peaksAndValies.power);
			peaksNoise[i] = applySpline(peaksNoise[i], settings.peaksAndValies.spline);
		}
		createFromGrayScale(peaksValiesT, peaksNoise, size);

	};

	
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

	float *finalNoise = new float[size.x * size.y];

	for (int i = 0; i < size.x * size.y; i++)
	{
		finalNoise[i] = lerp(testNoise[i], peaksNoise[i], settings.peaksAndValiesContribution);
	}

	createFromGrayScale(finalTexture, finalNoise, size);


	{
		glm::vec4 *colorData = new glm::vec4[256 * size.x];

		for (int i = 0; i < size.x; i++)
		{
			int height = int(startLevel + finalNoise[i + size.y-1] * heightDiff);
			
			for (int y = 256 - 1; y > height; y--)
			{
				colorData[y * size.x + i] = glm::vec4(101, 154, 207, 255) / 255.f; //sky
			}

			if (waterLevel > height)
			{
				for (int y = waterLevel; y > height; y--)
				{
					colorData[y * size.x + i] = glm::vec4(11, 16, 77, 255) / 255.f; //water
				}

				for (int y = height; y >= 0; y--)
				{
					colorData[y * size.x + i] = glm::vec4(77, 73, 70, 255) / 255.f; //stone
				}
			}
			else
			{
				colorData[height * size.x + i] = glm::vec4(10, 84, 18, 255) / 255.f; //grass

				for (int y = height - 1; y > height - 4; y--)
				{
					colorData[y * size.x + i] = glm::vec4(120, 66, 26, 255) / 255.f; //dirt
				}

				for (int y = height - 4; y >= 0; y--)
				{
					colorData[y * size.x + i] = glm::vec4(77, 73, 70, 255) / 255.f; //stone
				}
			}
		}

		createFromFloats(sliceT, &colorData->x, {size.x, 256});

		delete[] colorData;
	}

	FastNoiseSIMD::FreeNoiseSet(testNoise);
	FastNoiseSIMD::FreeNoiseSet(peaksNoise);
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

	ImGui::InputInt2("Size", &size[0]);
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



	auto noiseEditor = [&](NoiseSetting &s, const char *name)
	{
		ImGui::PushID(name);

		ImGui::Separator();
		renderSettingsForOneNoise(name, s);
		ImGui::Separator();

		ImGui::Table("Test", s.spline.points,
			s.spline.size);

		if (ImGui::Button("Add spline"))
		{
			s.spline.addSpline();
		}

		ImGui::SameLine();

		if (ImGui::Button("Remove spline"))
		{
			s.spline.removeSpline();
		}

		ImGui::SameLine();

		if (ImGui::Button("Invert"))
		{
			for (int i = 0; i < s.spline.size; i++)
			{
				auto &spl = s.spline;
				spl.points[i].y = 1 - spl.points[i].y;
			}
		}

		ImGui::Separator();


		ImGui::PopID();
	};


	noiseEditor(settings.continentalnessNoiseSettings, "Continentalness Noise");

	ImGui::SliderFloat("Peaks and valies contribution: ", &settings.peaksAndValiesContribution, 0, 1);
	noiseEditor(settings.peaksAndValies, "Peaks And Valies");




	ImGui::End();//		test	//

	//ImGui::ShowDemoWindow();
	//ImGui::ShowBezierDemo();

	settings.sanitize();
	wg.applySettings(settings);
	recreate();

	ImGui::Begin("Noise Viewer");

	ImGui::Text("Slice");

	ImGui::Image((ImTextureID)sliceT.id, {(float)sliceT.GetSize().x, 256},
		{0,1}, {1,0}, {1,1,1,1}, {1,1,1,1});

	drawNoise("Result", finalTexture);
	drawNoise("Continentalness", noiseT);
	drawNoise("PeaksValies", peaksValiesT);
	

	ImGui::End();





	return true;
#pragma endregion

}

//This function might not be be called if the program is forced closed
void closeGame()
{


}

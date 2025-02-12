#include <rendering/renderSettings.h>
#include <gamePlayLogic.h>
#include <filesystem>
#include <iostream>
#include <platform/platformInput.h>
#include "multyPlayer/createConnection.h"
#include <audioEngine.h>
#include <safeSave.h>
#include <sstream>

void displayRenderSettingsMenuButton(ProgramData &programData)
{
	programData.ui.menuRenderer.BeginMenu("Rendering", Colors_Gray, programData.ui.buttonTexture);

	displayRenderSettingsMenu(programData);

	programData.ui.menuRenderer.EndMenu();
}

#define DEFAULT_SLIDER Colors_White, programData.ui.buttonTexture, Colors_Gray, programData.ui.buttonTexture, Colors_White

void displayRenderSettingsMenu(ProgramData &programData)
{

	for (auto &s : programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId])
	{
		if (s == "Rendering")
		{
			programData.ui.renderer2d.renderText({150,50},
				("fps: " + std::to_string(programData.currentFps)).c_str(), programData.ui.font, Colors_Gray, 0.75f);
			break;
		}
	}



	programData.ui.menuRenderer.Text("Rendering Settings...", Colors_White);

	programData.ui.menuRenderer.sliderInt("View Distance", &getShadingSettings().viewDistance,
		1, 50, Colors_White, programData.ui.buttonTexture, Colors_Gray,
		programData.ui.buttonTexture, Colors_White);

	programData.ui.menuRenderer.sliderInt("Lod Strength", &getShadingSettings().lodStrength,
		0, 5, Colors_White, programData.ui.buttonTexture, Colors_Gray,
		programData.ui.buttonTexture, Colors_White);

#pragma region water
{
	programData.ui.menuRenderer.BeginMenu("Water", Colors_Gray, programData.ui.buttonTexture);

	programData.ui.menuRenderer.Text("Water settings...", Colors_White);


	programData.ui.menuRenderer.colorPicker("Water color",
		&programData.renderer.defaultShader.shadingSettings.waterColor[0],
		programData.ui.buttonTexture, programData.ui.buttonTexture, Colors_Gray, Colors_Gray);
	getShadingSettings().waterColor = programData.renderer.defaultShader.shadingSettings.waterColor;


	programData.ui.menuRenderer.colorPicker("Under water color",
		&programData.renderer.defaultShader.shadingSettings.underWaterColor[0],
		programData.ui.buttonTexture, programData.ui.buttonTexture, Colors_Gray, Colors_Gray);
	getShadingSettings().underWaterColor = programData.renderer.defaultShader.shadingSettings.underWaterColor;


	programData.ui.menuRenderer.sliderFloat("Underwater Fog strength",
		&programData.renderer.defaultShader.shadingSettings.underwaterDarkenStrength,
		0, 1, Colors_White, programData.ui.buttonTexture, Colors_Gray, 
		programData.ui.buttonTexture, Colors_White
		);
	getShadingSettings().underwaterDarkenStrength = programData.renderer.defaultShader.shadingSettings.underwaterDarkenStrength;


	programData.ui.menuRenderer.sliderFloat("Underwater Fog Distance",
		&programData.renderer.defaultShader.shadingSettings.underwaterDarkenDistance,
		0, 40, Colors_White, programData.ui.buttonTexture, Colors_Gray,
		programData.ui.buttonTexture, Colors_White);
	getShadingSettings().underwaterDarkenDistance = programData.renderer.defaultShader.shadingSettings.underwaterDarkenDistance;


	programData.ui.menuRenderer.sliderFloat("Underwater Fog Gradient",
		&programData.renderer.defaultShader.shadingSettings.fogGradientUnderWater,
		0, 32, Colors_White, programData.ui.buttonTexture, Colors_Gray,
		programData.ui.buttonTexture, Colors_White);
	getShadingSettings().fogGradientUnderWater = programData.renderer.defaultShader.shadingSettings.fogGradientUnderWater;


	static glm::vec4 colors[] = {{0.6,0.9,0.6,1}, Colors_Red};

	programData.ui.menuRenderer.toggleOptions("Water type: ", "cheap|fancy",
		&getShadingSettings().waterType, true, Colors_White, colors, programData.ui.buttonTexture,
		Colors_Gray,
		"How the water should be rendered\n-Cheap: \
good performance.\n-Fancy: significant performance cost but looks very nice.");

	programData.ui.menuRenderer.EndMenu();
}
#pragma endregion

#pragma region bloom
	programData.ui.menuRenderer.BeginMenu("Bloom", Colors_Gray, programData.ui.buttonTexture);
	programData.ui.menuRenderer.Text("Bloom settings...", Colors_White);

	programData.ui.menuRenderer.toggleOptions("BLoom: ", "OFF|ON", &getShadingSettings().bloom, true, Colors_White, 0, programData.ui.buttonTexture,
		Colors_Gray);

	if (getShadingSettings().bloom)
	{
		programData.ui.menuRenderer.sliderFloat("Bloom Multiplier", &getShadingSettings().bloomMultiplier, 0, 1, DEFAULT_SLIDER);
		programData.ui.menuRenderer.sliderFloat("Bloom Tresshold", &getShadingSettings().bloomTresshold, 0.1, 1, DEFAULT_SLIDER);
	};

	programData.ui.menuRenderer.EndMenu();
#pragma endregion


	//static glm::vec4 colorsTonemapper[] = {{0.6,0.9,0.6,1}, {0.6,0.9,0.6,1}, {0.7,0.8,0.6,1} , {0.4,0.8,0.4,1}};
	programData.ui.menuRenderer.toggleOptions("Tonemapper: ",
		"ACES|AgX|ZCAM|Uncharted", &getShadingSettings().tonemapper,
		true, Colors_White, nullptr, programData.ui.buttonTexture,
		Colors_Gray, 
"The tonemapper is the thing that displays the final color\n\
-Aces: a filmic look.\n-AgX: a more dull neutral look.\n-ZCAM a verey neutral and vanila look\n   preserves colors, slightly more expensive.\n Unchrated :))");
	programData.renderer.defaultShader.shadingSettings.tonemapper = getShadingSettings().tonemapper;


	programData.ui.menuRenderer.newColum(2);

	programData.ui.menuRenderer.Text("", {});
	
	programData.ui.menuRenderer.sliderInt("Chunk building extra threads", 
		&getShadingSettings().workerThreadsForBaking,
		0, 10, Colors_White, programData.ui.buttonTexture, Colors_Gray,
		programData.ui.buttonTexture, Colors_White);

	displayTexturePacksSettingsMenuButton(programData);

#pragma region SSR
	programData.ui.menuRenderer.BeginMenu("Screen Space Reflections", Colors_Gray, programData.ui.buttonTexture);
	programData.ui.menuRenderer.Text("Screen Space Reflections settings...", Colors_White);

	programData.ui.menuRenderer.toggleOptions("SSR: ", "OFF|ON", &getShadingSettings().SSR, true, Colors_White, 0, programData.ui.buttonTexture,
		Colors_Gray);

	if (getShadingSettings().SSR)
	{
		//... other SSR settings like quality stuff
	};

	programData.ui.menuRenderer.EndMenu();
#pragma endregion


	//programData.menuRenderer.BeginMenu("Volumetric", Colors_Gray, programData.buttonTexture);
	//programData.menuRenderer.Text("Volumetric Settings...", Colors_White);
	programData.ui.menuRenderer.sliderFloat("Fog gradient (O to disable it)",
		&getShadingSettings().fogGradient,
		0, 100, Colors_White, programData.ui.buttonTexture, Colors_Gray,
		programData.ui.buttonTexture, Colors_White);
	//programData.menuRenderer.EndMenu();


	static glm::vec4 colorsShadows[] = {{0.0,1,0.0,1}, {0.8,0.6,0.6,1}, {0.9,0.3,0.3,1}};
	programData.ui.menuRenderer.toggleOptions("Shadows: ", "Off|Hard|Soft",
		&getShadingSettings().shadows, true,
		Colors_White, colorsShadows, programData.ui.buttonTexture,
		Colors_Gray, "Shadows can affect the performance significantly."
	);


}

glm::ivec4 shrinkPercentage(glm::ivec4 dimensions, glm::vec2 p)
{
	glm::vec4 b = dimensions;

	b.x += (b.z * p.x) / 2.f;
	b.y += (b.w * p.y) / 2.f;

	b.z *= (1.f - p.x);
	b.w *= (1.f - p.y);

	dimensions = b;
	return dimensions;
}

bool texturePackDirty = 1;
int leftAdvance = 0;
int rightAdvance = 0;
std::vector<std::filesystem::path> loadedTexturePacks;
std::unordered_map<std::string, gl2d::Texture> logoTextures;

std::vector<std::filesystem::path> usedTexturePacks{"ourcraft"};

void displaySettingsMenuButton(ProgramData &programData)
{
	programData.ui.menuRenderer.BeginMenu("Settings", Colors_Gray, programData.ui.buttonTexture);

	displaySettingsMenu(programData);

	programData.ui.menuRenderer.EndMenu();
}

void displaySettingsMenu(ProgramData &programData)
{

	programData.ui.menuRenderer.Text("Settings", Colors_White);

	displayRenderSettingsMenuButton(programData);
	
	displayVolumeMenuButton(programData);
}

bool shouldReloadTexturePacks()
{
	return texturePackDirty;
}

std::vector<std::filesystem::path> getUsedTexturePacksAndResetFlag()
{
	texturePackDirty = false;
	return usedTexturePacks;
}

bool isTexturePackUsed(std::string t)
{
	for (auto &t2 : usedTexturePacks)
	{
		if (t == t2) { return true; }
	}
	return false;
}

void useTexturePack(std::string t)
{
	if (!isTexturePackUsed(t))
	{
		usedTexturePacks.push_back(t);
		texturePackDirty = true;
	}
}

void unuseTexturePack(std::string t)
{
	auto f = std::find(usedTexturePacks.begin(), usedTexturePacks.end(), t);

	if (f != usedTexturePacks.end())
	{
		usedTexturePacks.erase(f);
		texturePackDirty = true;
	}
}

void displayTexturePacksSettingsMenuButton(ProgramData &programData)
{
	programData.ui.menuRenderer.BeginMenu("Textures Packs", Colors_Gray, programData.ui.buttonTexture);

	displayTexturePacksSettingsMenu(programData);

	programData.ui.menuRenderer.EndMenu();
}


void openFolder(const char *path)
{
	//TODO!
#if defined(_WIN32) || defined(_WIN64)
	std::string command = "explorer ";
	command += path;
	system(command.c_str());
#elif defined(__APPLE__) || defined(__MACH__)
	std::string command = "open ";
	command += path;
	system(command.c_str());
#elif defined(__linux__)
	std::string command = "xdg-open ";
	command += path;
	system(command.c_str());
#else
	
#endif
}

inline void stubErrorFunc(const char *msg, void *userDefinedData)
{
	
}


bool renderButton(gl2d::Renderer2D &renderer,
	gl2d::Texture buttonTexture,
	glm::ivec4 box, const std::string &text = "", gl2d::Font *f = 0)
{
	bool hovered = 0;
	bool held = 0;
	bool released = 0;
	auto cursorPos = platform::getRelMousePosition();

	if (glui::aabb(box, cursorPos))
	{
		hovered = 1;

		if (platform::isLMouseHeld())
		{
			held = 1;
		}
		else if (platform::isLMouseReleased())
		{
			released = 1;
		}
	}

	if (held)
	{
		box.y += 10;
	}

	if (buttonTexture.id)
	{

		auto color = Colors_Gray;

		if (hovered)
		{
			color += glm::vec4(0.1, 0.1, 0.1, 0);
		}

		renderer.render9Patch(box,
			20, color, {}, 0, buttonTexture,
			GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});
	}

	if (text.size() && f)
	{
		glui::renderText(renderer, text, *f, box, Colors_White, 0, true, true);
	}

	return released;
};


void displayTexturePacksSettingsMenu(ProgramData &programData)
{
	std::error_code err;


	programData.ui.menuRenderer.Text("Texture packs...", Colors_White);

	//todo
	//if (programData.ui.menuRenderer.Button("Open Folder", Colors_Gray))
	//{
	//	if (!std::filesystem::exists(RESOURCES_PATH "texturePacks"))
	//	{
	//		std::filesystem::create_directories(RESOURCES_PATH "texturePacks", err);
	//	}
	//	
	//	openFolder(RESOURCES_PATH "texturePacks");
	//}


	glm::vec4 customWidgetTransform = {};
	programData.ui.menuRenderer.CustomWidget(169, &customWidgetTransform);



	//programData.ui.menuRenderer.cu
	auto &renderer = programData.ui.renderer2d;


	auto renderBox = [&](glm::vec4 c)
	{
		renderer.render9Patch(glui::Box().xCenter().yCenter().xDimensionPercentage(1).yDimensionPercentage(1.f)(),
			20, c, {}, 0, programData.ui.buttonTexture, GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});
	};

	if (programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].size()
		&& programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].back() == "Textures Packs"
		)
	{

		std::vector<std::filesystem::path> allTexturePacks;

		if (!std::filesystem::exists(RESOURCES_PATH "texturePacks"))
		{
			std::filesystem::create_directories(RESOURCES_PATH "texturePacks", err);
		}

		for (auto const &d : std::filesystem::directory_iterator{RESOURCES_PATH "texturePacks", err})
		{
			if (d.is_directory())
			{
				allTexturePacks.push_back(d.path().filename());
			}
		}

		//load logo textures
		gl2d::setErrorFuncCallback(stubErrorFunc);
		for (auto &pack : allTexturePacks)
		{
			auto file = pack;
			file = (RESOURCES_PATH "texturePacks") / file;
			file /= "logo.png";

			if (logoTextures.find(pack.string()) == logoTextures.end())
			{
				gl2d::Texture t;
				t.loadFromFile(file.string().c_str());

				if (!t.id)
				{
					file = pack;
					file = (RESOURCES_PATH "texturePacks") / file;
					file /= "pack.png";
					t.loadFromFile(file.string().c_str(), true);

				}

				logoTextures[pack.string()] = t;
			}
		}
		gl2d::setErrorFuncCallback(gl2d::defaultErrorFunc);
		//TODO delete unused entries


		auto listImplementation = [&](bool left)
		{
			std::vector<std::filesystem::path> *listPacks = 0;
			int *advance = 0;
			//buttons
			glm::ivec4 b;
			if (left)
			{
				b = glui::Box().xLeft().yTop().
					xDimensionPercentage(0.5).yDimensionPercentage(1).shrinkPercentage({0.05,0.05});
				listPacks = &allTexturePacks;
				advance = &leftAdvance;
			}
			else
			{
				b = glui::Box().xRight().yTop().
					xDimensionPercentage(0.5).yDimensionPercentage(1).shrinkPercentage({0.05,0.05});
				listPacks = &usedTexturePacks;
				advance = &rightAdvance;
			}

			glui::Frame f(b);

			float buttonSize = glui::Box().xLeft().yTop().xDimensionPercentage(0.1)().z;

			auto currentUpperBox = glui::Box().xCenter().yCenter().xDimensionPercentage(1).yDimensionPercentage(1.f)();

			//center
			{

				glui::Frame f(glui::Box().xCenter().yCenter().
					xDimensionPercentage(1).
					yDimensionPixels(currentUpperBox.w - buttonSize * 2)());

				auto currentBox = glui::Box().xCenter().yCenter().xDimensionPercentage(1).yDimensionPercentage(1.f)();

				//renderer.renderRectangle(,
				//	{0,1,0,0.2});

				renderBox({0.6,0.6,0.6,0.5});

				int height = currentBox.w - 10;

				int maxH = std::min(height, int(currentBox.z * 0.2));

				auto defaultB = glui::Box().xLeft().yTop().xDimensionPercentage(1).yDimensionPixels(maxH)();
				defaultB.y += 10;

				auto renderOneTeturePack = [&](glm::ivec4 box,
					std::string name)
				{
					box = shrinkPercentage(box, {0.1,0.05});

					renderer.render9Patch(box,
						20, {0.6,0.6,0.6,0.9}, {}, 0, programData.ui.buttonTexture,
						GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});

					{
						glui::Frame f(box);

						auto button = glui::Box().xRight().yTop().yDimensionPercentage(1.f).xDimensionPixels(buttonSize)();
						if (renderButton(renderer, programData.ui.buttonTexture, button))
						{
							if (left)
							{
								useTexturePack(name);
							}
							else
							{
								unuseTexturePack(name);
							}
						}

						auto icon = glui::Box().xLeft().yTop().yDimensionPercentage(1.f).xDimensionPixels(box.w)();

						if (logoTextures[name].id)
						{
							glui::renderTexture(renderer, shrinkPercentage(icon, {0.1,0.1}),
								logoTextures[name], Colors_White, GL2D_DefaultTextureCoords);
						}
						else
						{
							glui::renderTexture(renderer, shrinkPercentage(icon, {0.1,0.1}),
								programData.defaultCover, Colors_White, GL2D_DefaultTextureCoords);
						}


						auto text = icon;
						text.x += icon.z;
						text.z = button.x - text.x;

						glui::renderText(renderer, name, programData.ui.font, text, Colors_White, true);
					}


				};

				int canRenderCount = height / defaultB.w;

				int overflow = listPacks->size() - canRenderCount;
				if (overflow < 0) { overflow = 0; }
				if (*advance > overflow) { *advance = overflow; }

				for (int i = 0; i < listPacks->size(); i++)
				{
					if (i >= canRenderCount) { break; }

					renderOneTeturePack(defaultB, (*listPacks)[i + *advance].filename().string());
					defaultB.y += defaultB.w;
				}


			}

			//bottom button
			{
				auto currentDownBox = glui::Box().xLeft().yBottom().xDimensionPercentage(1).yDimensionPixels(buttonSize)();

				if (renderButton(renderer, programData.ui.buttonTexture, currentDownBox))
				{
					(*advance)++;
				}
			}

			//top button
			{
				auto currentUpperBox = glui::Box().xLeft().yTop().xDimensionPercentage(1).yDimensionPixels(buttonSize)();

				if (renderButton(renderer, programData.ui.buttonTexture, currentUpperBox))
				{
					(*advance)--;
				}
			}

			if (*advance < 0) { *advance = 0; }

		};
		

		glui::Frame f({0,0,renderer.windowW, renderer.windowH});

		{
			float ySize = renderer.windowH - customWidgetTransform.y - customWidgetTransform.w/2.f;

			glui::Frame f(glui::Box().xCenter().yTop(customWidgetTransform.y).
				yDimensionPixels(ySize).xDimensionPercentage(0.9)());

			//renderer.renderRectangle(glui::Box().xCenter().yCenter().xDimensionPercentage(1).yDimensionPercentage(1.f),
			//	{1,0,0,0.2});
			renderBox({0.2,0.2,0.2,0.4});

			//left
			{
				listImplementation(true);
			}

			//rioght
			{
				listImplementation(false);
			}

		}

	}
}

std::string currentSkinSelected = "";

gl2d::Texture currentTextureLoaded;
std::string currentTextureLoadedName = "";

void loadTexture()
{

	if (currentTextureLoadedName != currentSkinSelected || !currentTextureLoaded.id)
	{
		currentTextureLoaded.cleanup();
	}

	if(!currentTextureLoaded.id)
	{
		currentTextureLoadedName = currentSkinSelected;

		if (currentTextureLoadedName == "")
		{
			currentTextureLoaded
				= loadPlayerSkin(RESOURCES_PATH "assets/models/steve.png");
		}
		else
		{
			currentTextureLoaded
				= loadPlayerSkin((RESOURCES_PATH "skins/" + currentTextureLoadedName + ".png").c_str());
		}
	}

}

std::string getSkinName()
{
	return currentSkinSelected;
}

ShadingSettings shadingSettings;
ShadingSettings shadingSettingsLastFrame;

ShadingSettings &getShadingSettings()
{
	return shadingSettings;
}

bool checkIfShadingSettingsChangedForShaderReloads()
{
	shadingSettings.normalize();
	if (
		shadingSettings.shadows != shadingSettingsLastFrame.shadows ||
		shadingSettings.PBR != shadingSettingsLastFrame.PBR ||
		shadingSettings.SSR != shadingSettingsLastFrame.SSR
		)
	{
		shadingSettingsLastFrame = shadingSettings;
		return true;
	}

	return false;
}

#define SET_INT(x) data.setInt( #x, shadingSettings. x )
#define SET_VEC3(x) data.setRawData( #x, &shadingSettings. x [0], sizeof(shadingSettings. x) )
#define SET_FLOAT(x) data.setFloat( #x, shadingSettings. x )


void saveShadingSettings()
{

	shadingSettings.normalize();

	sfs::SafeSafeKeyValueData data;
	data.setInt("Version", 1);

	SET_INT(viewDistance);
	SET_INT(lodStrength);
	SET_INT(workerThreadsForBaking);
	SET_INT(tonemapper);
	SET_INT(shadows);
	SET_INT(waterType);
	SET_INT(PBR);
	SET_INT(SSR);
	SET_INT(bloom);

	SET_VEC3(waterColor);
	SET_VEC3(underWaterColor);

	SET_FLOAT(underwaterDarkenStrength);
	SET_FLOAT(underwaterDarkenDistance);
	SET_FLOAT(fogGradientUnderWater);
	SET_FLOAT(bloomTresshold);
	SET_FLOAT(bloomMultiplier);
	SET_FLOAT(exposure);
	SET_FLOAT(fogGradient);

	sfs::safeSave(data, RESOURCES_PATH "../playerSettings/renderSettings", 0);

}

#undef SET_INT
#undef SET_VEC3
#undef SET_FLOAT



#define GET_INT(x) data.getInt( #x, shadingSettings. x )
#define GET_VEC3(x) data.setRawData( #x, &shadingSettings. x [0], sizeof(shadingSettings. x) )
#define GET_FLOAT(x) data.getFloat( #x, shadingSettings. x )
void loadShadingSettings()
{

	shadingSettings = ShadingSettings{};

	sfs::SafeSafeKeyValueData data;

	if (sfs::safeLoad(data, RESOURCES_PATH "../playerSettings/renderSettings", 0) == sfs::noError)
	{
		GET_INT(viewDistance);
		GET_INT(lodStrength);
		GET_INT(workerThreadsForBaking);
		GET_INT(tonemapper);
		GET_INT(shadows);
		GET_INT(waterType);
		GET_INT(PBR);
		GET_INT(SSR);
		GET_INT(bloom);


		void *rawData = 0;
		size_t dataSize = 0;
		data.getRawDataPointer("waterColor", rawData, dataSize);
		if (dataSize == sizeof(shadingSettings.waterColor))
			{ memcpy(&shadingSettings.waterColor[0], rawData, dataSize); }

		data.getRawDataPointer("underWaterColor", rawData, dataSize);
		if (dataSize == sizeof(shadingSettings.underWaterColor))
			{ memcpy(&shadingSettings.underWaterColor[0], rawData, dataSize); }

		GET_FLOAT(underwaterDarkenStrength);
		GET_FLOAT(underwaterDarkenDistance);
		GET_FLOAT(fogGradientUnderWater);
		GET_FLOAT(exposure);

		GET_FLOAT(bloomTresshold);
		GET_FLOAT(bloomMultiplier);
		GET_FLOAT(fogGradient);

	}

	shadingSettings.normalize();

	//todo apply

}

#undef GET_INT
#undef GET_VEC3
#undef GET_FLOAT


void displaySkinSelectorMenu(ProgramData &programData)
{
	std::error_code err;

	programData.ui.menuRenderer.Text("Change Skin", Colors_White);

	//todo
	//if (programData.ui.menuRenderer.Button("Open Folder", Colors_Gray))
	//{
	//	if (!std::filesystem::exists(RESOURCES_PATH "texturePacks"))
	//	{
	//		std::filesystem::create_directories(RESOURCES_PATH "texturePacks", err);
	//	}
	//	
	//	openFolder(RESOURCES_PATH "texturePacks");
	//}

	glm::vec4 customWidgetTransform = {};
	programData.ui.menuRenderer.CustomWidget(170, &customWidgetTransform);

	//programData.ui.menuRenderer.cu
	auto &renderer = programData.ui.renderer2d;
	
	auto renderBox = [&](glm::vec4 c)
	{
		renderer.render9Patch(glui::Box().xCenter().yCenter().xDimensionPercentage(1).yDimensionPercentage(1.f)(),
			20, c, {}, 0, programData.ui.buttonTexture, GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});
	};

	if (programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].size()
		&& programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].back() == "Change Skin"
		)
	{

		if (!std::filesystem::exists(RESOURCES_PATH "skins"))
		{
			std::filesystem::create_directories(RESOURCES_PATH "skins", err);
		}

		std::vector<std::string> skins;

		for (auto const &d : std::filesystem::directory_iterator{RESOURCES_PATH "skins", err})
		{
			if (!d.is_directory())
			{
				if (d.path().filename().extension() == ".png")
				{
					skins.push_back(d.path().filename().stem().string());
				}
			}
		}

		int posInVect = -1;
		if (currentSkinSelected == "")
		{
			posInVect = -1;
		}
		else
		{
			int index = 0;
			for(auto &s : skins)
			{
				if (s == currentSkinSelected)
				{
					currentSkinSelected = s;
					posInVect = index;
					break;
				}
				index++;
			}
		}

		

		glui::Frame f({0,0,renderer.windowW, renderer.windowH});
		{
			float ySize = renderer.windowH - customWidgetTransform.y - customWidgetTransform.w / 2.f;

			glui::Frame f(glui::Box().xCenter().yTop(customWidgetTransform.y).
				yDimensionPixels(ySize).xDimensionPercentage(0.9)());

			//renderer.renderRectangle(glui::Box().xCenter().yCenter().xDimensionPercentage(1).yDimensionPercentage(1.f),
			//	{1,0,0,1.0});
			renderBox({0.4,0.4,0.4,0.5});


			auto textBox = glui::Box().xCenter().yBottom().xDimensionPercentage(1).yDimensionPercentage(0.2)();
			if (currentSkinSelected == "")
			{
				glui::renderText(renderer, "Default", programData.ui.font, textBox, Colors_White, true);
			}
			else
			{
				glui::renderText(renderer, currentSkinSelected, programData.ui.font, textBox, Colors_White, true);
			}

			auto center = glui::Box().xCenter().yCenter().yDimensionPercentage(0.5).xDimensionPercentage(0.5)();
			center.z = std::min(center.z, center.w);
			center = glui::Box().xCenter().yCenter().yDimensionPixels(center.z).xDimensionPixels(center.z)();

			loadTexture();
			if (currentTextureLoaded.id)
			{
				renderer.renderRectangle(center, currentTextureLoaded);
			}

			auto left = glui::Box().xLeft().yCenter().yDimensionPercentage(0.2).xAspectRatio(1.0)();
			auto right = glui::Box().xRight().yCenter().yDimensionPercentage(0.2).xAspectRatio(1.0)();

			//move cursor
			{
				if (renderButton(renderer, programData.ui.buttonTexture, left))
				{
					posInVect--;
				}

				if (renderButton(renderer, programData.ui.buttonTexture, right))
				{
					posInVect++;
				}

				if (posInVect < -1)
				{
					posInVect = skins.size() - 1;
				}

				if (posInVect >= skins.size())
				{
					posInVect = -1;
				}

				if (posInVect == -1)
				{
					currentSkinSelected = "";
				}
				else
				{
					currentSkinSelected = skins[posInVect];
				}
			}

		
		}
	}

}

void displaySkinSelectorMenuButton(ProgramData &programData)
{
	programData.ui.menuRenderer.BeginMenu("Change Skin", Colors_Gray, programData.ui.buttonTexture);

	displaySkinSelectorMenu(programData);

	programData.ui.menuRenderer.EndMenu();
}

void displayVolumeMenuButton(ProgramData &programData)
{
	programData.ui.menuRenderer.BeginMenu("Audio Settings", Colors_Gray, programData.ui.buttonTexture);

	displayVolumeMenu(programData);

	programData.ui.menuRenderer.EndMenu();
}

void displayVolumeMenu(ProgramData &programData)
{

	programData.ui.menuRenderer.Text("Audio Settings", Colors_White);

	programData.ui.menuRenderer.sliderFloat("Master Volume", &AudioEngine::getMasterVolume(), 0, 1, Colors_White, programData.ui.buttonTexture, Colors_Gray, programData.ui.buttonTexture, Colors_White);
	programData.ui.menuRenderer.newLine();
	programData.ui.menuRenderer.sliderFloat("Music Volume", &AudioEngine::getMusicVolume(), 0, 1, Colors_White, programData.ui.buttonTexture, Colors_Gray, programData.ui.buttonTexture, Colors_White);
	programData.ui.menuRenderer.sliderFloat("UI Volume", &AudioEngine::getUIVolume(), 0, 1, Colors_White, programData.ui.buttonTexture, Colors_Gray, programData.ui.buttonTexture, Colors_White);
	programData.ui.menuRenderer.sliderFloat("Sounds Volume", &AudioEngine::getSoundsVolume(), 0, 1, Colors_White, programData.ui.buttonTexture, Colors_Gray, programData.ui.buttonTexture, Colors_White);

}


void displayWorldSelectorMenuButton(ProgramData &programData)
{
	programData.ui.menuRenderer.BeginMenu("Play", Colors_Gray, programData.ui.buttonTexture);

	displayWorldSelectorMenu(programData);

	programData.ui.menuRenderer.EndMenu();
}


void displayWorldSelectorMenu(ProgramData &programData)
{
	auto &renderer = programData.ui.renderer2d;



	programData.ui.menuRenderer.Text("Select world", Colors_White);

	//programData.ui.menuRenderer.Button("Create new world", Colors_Gray, programData.ui.buttonTexture);
	programData.ui.menuRenderer.Text("", Colors_White);

	glm::vec4 customWidgetTransform = {};
	programData.ui.menuRenderer.CustomWidget(171, &customWidgetTransform);

	glui::Frame f({0,0,renderer.windowW, renderer.windowH});


	auto drawButton = [&](glm::vec4 transform, glm::vec4 color,
		const std::string &s)
	{
		return glui::drawButton(renderer, transform, color, s, programData.ui.font, programData.ui.buttonTexture,
			platform::getRelMousePosition(), platform::isLMouseHeld(), platform::isLMouseReleased());
	};

	if (programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].size()
		&& programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].back() == "Play"
		)
	{

		//background
		{
			float rezolution = 256;
			glm::vec2 size{renderer.windowW, renderer.windowH};
			size /= 256.f;

			renderer.renderRectangle({0,0, renderer.windowW, renderer.windowH},
				programData.blocksLoader.backgroundTexture, {0.6,0.6,0.6,1}, {}, 0,
				{0,size.y, size.x, 0}
				);
		}

		//folder logic

		if (!std::filesystem::exists(RESOURCES_PATH "worlds"))
		{
			std::filesystem::create_directories(RESOURCES_PATH "worlds");
		}
			
		std::vector<std::filesystem::path> allWorlds;
		for (auto const &d : std::filesystem::directory_iterator{RESOURCES_PATH "worlds"})
		{
			if (d.is_directory())
			{
				allWorlds.push_back(d.path().filename());
			}
		}

		static std::string selected = "";

		//center
		{
			float ySize = renderer.windowH - customWidgetTransform.y * 2;
			
			if (ySize > 50)
			{
				static int advance = 0;

				glui::Frame f(glui::Box().xCenter().yTop(customWidgetTransform.y).
					yDimensionPixels(ySize).xDimensionPercentage(1)());
				auto fullBox = glui::Box().xLeft().yTop().xDimensionPercentage(1.f).yDimensionPercentage(1.f)();

				float buttonH = customWidgetTransform.w;
				int maxElements = fullBox.w / buttonH;

				advance = std::max(advance, 0);
				int overflow = allWorlds.size() - maxElements;
				if (overflow < 0) { overflow = 0; }
				advance = std::min(advance, overflow);

				//background
				{
					float rezolution = 256;
					glm::vec2 size{fullBox.z, fullBox.w};
					size /= 256.f;

					float padding = advance * 0.4;

					renderer.renderRectangle(fullBox,
						programData.blocksLoader.backgroundTexture, {0.4,0.4,0.4,1}, {}, 0,
						{padding,size.y, size.x + padding, 0}
					);
				}

				//renderer.renderRectangle(glui::Box().xCenter().yCenter().xDimensionPercentage(1).yDimensionPercentage(1.f),
				//	{1,0,0,1.0});

				glm::vec4 worldBox = fullBox;
				worldBox.w = buttonH;
				worldBox.z -= buttonH;

				for (int i = 0; i < maxElements; i++)
				{
					if (allWorlds.size() <= i + advance) { break; }

					auto s = allWorlds[i + advance].string();

					if (s == selected)
					{
						renderer.render9Patch(worldBox,
							20, {0.3,0.3,0.3,0.7}, {}, 0, programData.ui.buttonTexture,
							GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});
					}

					if (renderButton(renderer, {}, worldBox, s, &programData.ui.font))
					{
						selected = s;
					}

					worldBox.y += buttonH;
				}

				auto topButton = glui::Box().yTop().xRight().yDimensionPixels(buttonH).xDimensionPixels(buttonH)();
				auto bottomButton = glui::Box().yBottom().xRight().yDimensionPixels(buttonH).xDimensionPixels(buttonH)();

				if (renderButton(renderer, programData.ui.buttonTexture, topButton)) { advance--; }
				if (renderButton(renderer, programData.ui.buttonTexture, bottomButton)) { advance++; }

			}

		}

		//bottom
		{
			float ySize = customWidgetTransform.y;
			glui::Frame f(glui::Box().xCenter().yBottom().
				yDimensionPixels(ySize).xDimensionPercentage(1)());

			//renderer.renderRectangle(glui::Box().xCenter().yCenter().xDimensionPercentage(1).yDimensionPercentage(1.f),
			//	{0,1,0,1.0});


			//top
			{
				glui::Frame f(glui::Box().xCenter().yTop().
					yDimensionPercentage(0.5).xDimensionPercentage(1)());

				{
					auto leftButton = glui::Box().xLeft().yCenter().xDimensionPercentage(0.5).yDimensionPercentage(1)();
					if (drawButton(shrinkPercentage(leftButton, {0.1,0.05}), Colors_Gray, "Play Selected World"))
					{
						if (selected.size())
						{
							hostServer(selected);
						}

					}

					auto rightButton = glui::Box().xRight().yCenter().xDimensionPercentage(0.5).yDimensionPercentage(1)();
					if (drawButton(shrinkPercentage(rightButton, {0.1,0.05}), Colors_Gray, "Settings"))
					{
						


					}


				}

			}

			//bottom
			{
				glui::Frame f(glui::Box().xCenter().yBottom().
					yDimensionPercentage(0.5).xDimensionPercentage(1)());

				{
					auto leftButton = glui::Box().xLeft().yCenter().xDimensionPercentage(0.5).yDimensionPercentage(1)();
					if (drawButton(shrinkPercentage(leftButton, {0.1,0.05}), Colors_Gray, "Create a new world!"))
					{
						programData.ui.menuRenderer.StartManualMenu("Create world");
					}

					auto rightButton = glui::Box().xRight().yCenter().xDimensionPercentage(0.5).yDimensionPercentage(1)();
					drawButton(shrinkPercentage(rightButton, {0.1,0.05}), Colors_Gray, "Delete world");
				}


			}

		}

	}

	static char seed[12] = {};
	static char name[20] = {};
	static int currentIndex = 0; //0 normal, 1 super flat
	static WorldGeneratorSettings settings;
	static gl2d::Texture worldPreviewTexture;
	static WorldGenerator wg;
	if (wg.regionsHeightNoise == 0)
	{
		wg.init();
	}

	programData.ui.menuRenderer.BeginManualMenu("Create world");

	
	if (programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].size()
		&& programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].back() == "Create world"
		)
	{

		programData.ui.menuRenderer.temporalViewPort
			= glm::vec4(0, 0, programData.ui.renderer2d.windowW / 2.6f, programData.ui.renderer2d.windowH);

		programData.ui.menuRenderer.Text("Create a new world!", Colors_White);
		

		//background
		{
			float rezolution = 256;
			glm::vec2 size{renderer.windowW, renderer.windowH};
			size /= 256.f;
	
			renderer.renderRectangle({0,0, renderer.windowW, renderer.windowH},
				programData.blocksLoader.backgroundTexture, {0.6,0.6,0.6,1}, {}, 0,
				{0,size.y, size.x, 0}
			);
		}


		programData.ui.menuRenderer.InputText("Name: ", name, sizeof(name),
			Colors_Gray, programData.ui.buttonTexture);

		programData.ui.menuRenderer.InputText("Seed: ", seed, sizeof(seed),
			Colors_Gray, programData.ui.buttonTexture);
		
		//programData.ui.menuRenderer.Toggle("Super Flat", Colors_Gray, &superFlatWorld, programData.ui.buttonTexture, programData.ui.buttonTexture);

		std::string finalName = RESOURCES_PATH "worlds/";
		finalName += name;

		if (name[0] == '\0')
		{
			programData.ui.menuRenderer.Button("Please enter a name!", {0.6,0.4,0.4,1},
				programData.ui.buttonTexture);
		}
		else if (std::filesystem::exists(finalName))
		{
			programData.ui.menuRenderer.Button("Name already exists!", {0.6,0.4,0.4,1},
				programData.ui.buttonTexture);
		}
		else
		{
			bool create = 0;
			bool createAndPlay = 0;


			if (programData.ui.menuRenderer.Button("Create!", Colors_Gray,
				programData.ui.buttonTexture))
			{
				create = true;
			}

			if (programData.ui.menuRenderer.Button("Create and play", Colors_Gray,
				programData.ui.buttonTexture))
			{
				createAndPlay = true;
			}

			if (create || createAndPlay)
			{
				std::error_code err;
				bool err2 = 0;
				std::filesystem::create_directory(finalName, err);


				{
					std::ifstream f(RESOURCES_PATH "gameData/worldGenerator/default.wgenerator");
					if (f.is_open())
					{
						std::stringstream buffer;
						buffer << f.rdbuf();
						if (!settings.loadSettings(buffer.str().c_str()))
						{
							err2 = true;
						}
					}

				}

				if (!err && !err2)
				{
					int finalSeed = 0;
					{
						//std::ofstream f(finalName + "/seed.txt");

						long long computedSeed = 0;
						long long pow = 1;
						for (int i = sizeof(seed) - 1; i >= 0; i--)
						{
							if (seed[i] != 0)
							{
								computedSeed += (seed[i] - '0') * pow;
								pow *= 10;
							}
						}

						if (computedSeed == 0)
						{
							computedSeed = time(0);
						}

						finalSeed = computedSeed;
						if (finalSeed < 0) { finalSeed = -finalSeed; }
						if (finalSeed == 0) { finalSeed = 1; }

						//f << (int)finalSeed;
						//f.close();
					};

					{
						std::ofstream f(finalName + "/worldGenSettings.wgenerator");
						settings.seed = finalSeed;
						settings.isSuperFlat = (currentIndex == 1);

						settings.sanitize();
						f << settings.saveSettings();

					}

					if (createAndPlay)
					{
						hostServer(name);
					}
				}

				programData.ui.menuRenderer.ExitCurrentMenu();
			}

		}
		
		
		//programData.ui.menuRenderer.newColum(11);
		//programData.ui.menuRenderer.newColum(12);

		{
			auto &renderer = programData.ui.renderer2d;
			glui::Frame f({0,0, renderer.windowW, renderer.windowH});

			{
				glui::Frame f(glui::Box().xLeftPerc(0.35).yTopPerc(0.1).yDimensionPercentage(0.8).xDimensionPercentage(0.6)());

				{
					auto fullBox = glui::Box().xLeft().yTop().xDimensionPercentage(1.f).yDimensionPercentage(1.f)();
					

					auto buttonBox = glui::Box().xLeft().yBottom().xDimensionPercentage(1.f).yDimensionPixels(150.f)();
					glui::toggleOptions(renderer, buttonBox, "World type: ", Colors_White,
						"Normal|Super Flat", &currentIndex, true, programData.ui.font,
						programData.ui.buttonTexture, Colors_Gray, platform::getRelMousePosition(),
						platform::isLMouseHeld(), platform::isLMouseReleased());

					auto mapBox = glui::Box().xLeft().yTop().xDimensionPercentage(1.f).yDimensionPercentage(1.f)();
					mapBox.w -= buttonBox.w;

					if (mapBox.w > 0)
					{
						renderer.render9Patch(mapBox, 20,
							Colors_Gray, {}, 0.f, programData.ui.buttonTexture, 
							GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});

						//auto textureBox = mapBox;
						//textureBox = shrinkPercentage(textureBox, {0.6f, 0.6f});
						//
						//worldPreviewTexture.cleanup();
						//wg.applySettings(settings);
						//	
						//wg.generateChunkPreview(worldPreviewTexture, {textureBox.z,textureBox.w}, {});
						//
						//renderer.renderRectangle(textureBox, worldPreviewTexture);

					}

				}


			}


		}


	}
	else
	{
		memset(seed, 0, sizeof(seed));
		memset(name, 0, sizeof(name));
		currentIndex = 0;
		settings = {};
	}
	
	programData.ui.menuRenderer.EndMenu();


}

void ShadingSettings::normalize()
{

	viewDistance = glm::clamp(viewDistance, 1, 50);
	tonemapper = glm::clamp(tonemapper, 0, 3);
	shadows = glm::clamp(shadows, 0, 2);
	waterType = glm::clamp(waterType, 0, 1);

	waterColor = glm::clamp(waterColor, glm::vec3(0.f), glm::vec3(2.f, 2.f, 2.f));
	underWaterColor = glm::clamp(underWaterColor, glm::vec3(0.f), glm::vec3(2.f, 2.f, 2.f));

	underwaterDarkenStrength = glm::clamp(underwaterDarkenStrength, 0.f, 1.f);
	underwaterDarkenDistance = glm::clamp(underwaterDarkenDistance, 0.f, 40.f);
	fogGradientUnderWater = glm::clamp(fogGradientUnderWater, 0.f, 32.f);
	workerThreadsForBaking = glm::clamp(workerThreadsForBaking, 0, 10);
	lodStrength = glm::clamp(lodStrength, 0, 5);

	bloomTresshold = glm::clamp(bloomTresshold, 0.1f, 1.f);
	bloomMultiplier = glm::clamp(bloomMultiplier, 0.0f, 1.f);
	
	PBR = glm::clamp(PBR, 0, 1);
	bloom = glm::clamp(bloom, 0, 1);
	SSR = glm::clamp(SSR, 0, 1);

	exposure = glm::clamp(exposure, -2.f, 2.f);
	fogGradient = glm::clamp(fogGradient, 0.f, 100.f);

}

#define GET_STR(x) "c_" #x

#define ADD_TO_RESULT(x) result += ("#define " GET_STR(x) " ") + std::to_string(x) + "\n"
#define ADD_TO_RESULT_VEC3(x) result += ("#define " GET_STR(x) " vec3(") + std::to_string(x.r) + "," + std::to_string(x.g) + "," + std::to_string(x.b) + ")\n"


std::string ShadingSettings::formatIntoGLSLcode()
{
	normalize();

	std::string result;
	result.reserve(500);


	ADD_TO_RESULT(shadows);
	ADD_TO_RESULT(PBR);
	ADD_TO_RESULT(SSR);




	return result;
}

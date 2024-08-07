#include <rendering/renderSettings.h>
#include <gamePlayLogic.h>
#include <filesystem>
#include <iostream>
#include <platform/platformInput.h>

void displayRenderSettingsMenuButton(ProgramData &programData)
{
	programData.ui.menuRenderer.BeginMenu("Rendering", Colors_Gray, programData.ui.buttonTexture);

	displayRenderSettingsMenu(programData);

	programData.ui.menuRenderer.EndMenu();

}

void displayRenderSettingsMenu(ProgramData &programData)
{


	programData.ui.menuRenderer.Text("Rendering Settings...", Colors_White);

	programData.ui.menuRenderer.sliderInt("View Distance", &programData.otherSettings.viewDistance,
		1, 64, Colors_White, programData.ui.buttonTexture, Colors_Gray,
		programData.ui.buttonTexture, Colors_White);

#pragma region water
{
	programData.ui.menuRenderer.BeginMenu("Water", Colors_Gray, programData.ui.buttonTexture);

	programData.ui.menuRenderer.Text("Water settings...", Colors_White);


	programData.ui.menuRenderer.colorPicker("Water color",
		&programData.renderer.defaultShader.shadingSettings.waterColor[0],
		programData.ui.buttonTexture, programData.ui.buttonTexture, Colors_Gray, Colors_Gray);

	programData.ui.menuRenderer.colorPicker("Under water color",
		&programData.renderer.defaultShader.shadingSettings.underWaterColor[0],
		programData.ui.buttonTexture, programData.ui.buttonTexture, Colors_Gray, Colors_Gray);

	programData.ui.menuRenderer.sliderFloat("Underwater Fog strength",
		&programData.renderer.defaultShader.shadingSettings.underwaterDarkenStrength,
		0, 1, Colors_White, programData.ui.buttonTexture, Colors_Gray, 
		programData.ui.buttonTexture, Colors_White
		);

	programData.ui.menuRenderer.sliderFloat("Underwater Fog Distance",
		&programData.renderer.defaultShader.shadingSettings.underwaterDarkenDistance,
		0, 40, Colors_White, programData.ui.buttonTexture, Colors_Gray,
		programData.ui.buttonTexture, Colors_White);

	programData.ui.menuRenderer.sliderFloat("Underwater Fog Gradient",
		&programData.renderer.defaultShader.shadingSettings.fogGradientUnderWater,
		0, 32, Colors_White, programData.ui.buttonTexture, Colors_Gray,
		programData.ui.buttonTexture, Colors_White);

	static glm::vec4 colors[] = {{0.6,0.9,0.6,1}, Colors_Red};

	programData.ui.menuRenderer.toggleOptions("Water type: ", "cheap|fancy",
		&programData.renderer.waterRefraction, true, Colors_White, colors, programData.ui.buttonTexture,
		Colors_Gray,
		"How the water should be rendered\n-Cheap: \
good performance.\n-Fancy: significant performance cost but looks very nice.");

	programData.ui.menuRenderer.EndMenu();
}
#pragma endregion

	static glm::vec4 colorsTonemapper[] = {{0.6,0.9,0.6,1}, {0.6,0.9,0.6,1}, {0.7,0.8,0.6,1}};
	programData.ui.menuRenderer.toggleOptions("Tonemapper: ",
		"ACES|AgX|ZCAM", &programData.renderer.defaultShader.shadingSettings.tonemapper,
		true, Colors_White, colorsTonemapper, programData.ui.buttonTexture,
		Colors_Gray, 
"The tonemapper is the thing that displays the final color\n\
-Aces: a filmic look.\n-AgX: a more dull neutral look.\n-ZCAM a verey neutral and vanila look\n   preserves colors, slightly more expensive.");

	
	programData.ui.menuRenderer.newColum(2);

	programData.ui.menuRenderer.Text("", {});
	
	displayTexturePacksSettingsMenuButton(programData);

	//programData.menuRenderer.BeginMenu("Volumetric", Colors_Gray, programData.buttonTexture);
	//programData.menuRenderer.Text("Volumetric Settings...", Colors_White);
	programData.ui.menuRenderer.sliderFloat("Fog gradient (O to disable it)",
		&programData.renderer.defaultShader.shadingSettings.fogCloseGradient,
		0, 64, Colors_White, programData.ui.buttonTexture, Colors_Gray,
		programData.ui.buttonTexture, Colors_White);
	//programData.menuRenderer.EndMenu();

	static glm::vec4 colorsShadows[] = {{0.0,1,0.0,1}, {0.8,0.6,0.6,1}, {0.9,0.3,0.3,1}};
	programData.ui.menuRenderer.toggleOptions("Shadows: ", "Off|Hard|Soft",
		&programData.renderer.defaultShader.shadingSettings.shadows, true,
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

bool texturePackDirty = 0;
int leftAdvance = 0;
int rightAdvance = 0;
std::vector<std::filesystem::path> loadedTexturePacks;
std::unordered_map<std::string, gl2d::Texture> logoTextures;

std::vector<std::filesystem::path> usedTexturePacks;

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

void displayTexturePacksSettingsMenu(ProgramData &programData)
{

	programData.ui.menuRenderer.Text("Texture packs...", Colors_White);

	glm::vec4 customWidgetTransform = {};
	programData.ui.menuRenderer.CustomWidget(169, &customWidgetTransform);

	//programData.ui.menuRenderer.cu
	auto &renderer = programData.ui.renderer2d;


	auto renderButton = [&](glm::ivec4 box) -> bool
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

		auto color = Colors_Gray;

		if (hovered)
		{
			color += glm::vec4(0.1, 0.1, 0.1, 0);
		}

		if (held)
		{
			box.y += 10;
		}

		renderer.render9Patch(box,
			20, color, {}, 0, programData.ui.buttonTexture,
			GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});

		return released;
	};


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
		std::error_code err;

		std::vector<std::filesystem::path> allTexturePacks;

		if (!std::filesystem::exists(RESOURCES_PATH "texturePacks"))
		{
			std::filesystem::create_directories(RESOURCES_PATH "texturePacks", err);
		}

		for (auto const &d : std::filesystem::directory_iterator{RESOURCES_PATH "texturePacks"})
		{
			if (d.is_directory())
			{
				allTexturePacks.push_back(d.path().filename());
			}
		}

		//load logo textures
		for (auto &pack : allTexturePacks)
		{

			auto file = pack;
			file = (RESOURCES_PATH "texturePacks") / file;
			file /= "logo.png";

			if (logoTextures.find(pack.string()) == logoTextures.end())
			{
				gl2d::Texture t;
				t.loadFromFile(file.string().c_str());

				logoTextures[pack.string()] = t;
			}
		}

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
						if (renderButton(button))
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

				if (renderButton(currentDownBox))
				{
					(*advance)++;
				}
			}

			//top button
			{
				auto currentUpperBox = glui::Box().xLeft().yTop().xDimensionPercentage(1).yDimensionPixels(buttonSize)();

				if (renderButton(currentUpperBox))
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

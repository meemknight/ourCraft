#include <rendering/renderSettings.h>
#include <gamePlayLogic.h>

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
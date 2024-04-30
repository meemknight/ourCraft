#include <rendering/renderSettings.h>
#include <gamePlayLogic.h>
#include <intrin.h>

void displayRenderSettingsMenuButton(ProgramData &programData)
{
	programData.menuRenderer.BeginMenu("Rendering", Colors_Gray, programData.buttonTexture);

	displayRenderSettingsMenu(programData);

	programData.menuRenderer.EndMenu();


}

void displayRenderSettingsMenu(ProgramData &programData)
{


	programData.menuRenderer.Text("Rendering Settings...", Colors_White);

#pragma region water
{
	programData.menuRenderer.BeginMenu("Water", Colors_Gray, programData.buttonTexture);

	programData.menuRenderer.Text("Water settings...", Colors_White);


	programData.menuRenderer.colorPicker("Water color",
		&programData.renderer.defaultShader.shadingSettings.waterColor[0],
		programData.buttonTexture, programData.buttonTexture, Colors_Gray, Colors_Gray);

	programData.menuRenderer.colorPicker("Under water color",
		&programData.renderer.defaultShader.shadingSettings.underWaterColor[0],
		programData.buttonTexture, programData.buttonTexture, Colors_Gray, Colors_Gray);

	programData.menuRenderer.sliderFloat("Underwater Fog strength",
		&programData.renderer.defaultShader.shadingSettings.underwaterDarkenStrength,
		0, 1, Colors_White, programData.buttonTexture, Colors_Gray);

	programData.menuRenderer.sliderFloat("Underwater Fog Distance",
		&programData.renderer.defaultShader.shadingSettings.underwaterDarkenDistance,
		0, 40, Colors_White, programData.buttonTexture, Colors_Gray);

	programData.menuRenderer.sliderFloat("Underwater Fog Gradient",
		&programData.renderer.defaultShader.shadingSettings.fogGradientUnderWater,
		0, 32, Colors_White, programData.buttonTexture, Colors_Gray);

	static int index = 0;
	static glm::vec4 colors[] = {{0.6,0.9,0.6,1}, Colors_Red};

	programData.menuRenderer.toggleOptions("Water type: ", "cheap|fancy",
		&index, true, Colors_White, colors, programData.buttonTexture,
		Colors_Gray,
		"How the water should be rendered\n-Cheap: \
good performance.\n-Fancy: significant performance cost but looks very nice.");

	programData.menuRenderer.EndMenu();
}
#pragma endregion

	static glm::vec4 colorsTonemapper[] = {{0.6,0.9,0.6,1}, {0.6,0.9,0.6,1}, {0.7,0.8,0.6,1}};
	programData.menuRenderer.toggleOptions("Tonemapper: ",
		"ACES|AgX|ZCAM", &programData.renderer.defaultShader.shadingSettings.tonemapper,
		true, Colors_White, colorsTonemapper, programData.buttonTexture,
		Colors_Gray, 
"The tonemapper is the thing that displays the final color\n\
-Aces: a filmic look.\n-AgX: a more dull neutral look.\n-ZCAM a verey neutral and vanila look\n   preserves colors, slightly more expensive.");

	
	programData.menuRenderer.newColum(2);
	
	//programData.menuRenderer.BeginMenu("Volumetric", Colors_Gray, programData.buttonTexture);
	//programData.menuRenderer.Text("Volumetric Settings...", Colors_White);
	programData.menuRenderer.sliderFloat("Fog gradient (O to disable it)", 
		&programData.renderer.defaultShader.shadingSettings.fogCloseGradient,
		0, 64, Colors_White, programData.buttonTexture, Colors_Gray);
	//programData.menuRenderer.EndMenu();

	static glm::vec4 colorsShadows[] = {{0.0,1,0.0,1}, {0.8,0.6,0.6,1}, {0.9,0.3,0.3,1}};
	programData.menuRenderer.toggleOptions("Shadows: ", "Off|Hard|Soft",
		&programData.renderer.defaultShader.shadingSettings.shadows, true,
		Colors_White, colorsShadows, programData.buttonTexture,
		Colors_Gray, "Shadows can affect the performance significantly."
	);



}
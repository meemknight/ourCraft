#pragma once

#include "glui/glui.h"
#include <filesystem>

struct ProgramData;


void displayRenderSettingsMenuButton(ProgramData &programData);

void displayRenderSettingsMenu(ProgramData &programData);

bool shouldReloadTexturePacks();

std::vector<std::filesystem::path> getUsedTexturePacksAndResetFlag();

void displayTexturePacksSettingsMenuButton(ProgramData &programData);

void displayTexturePacksSettingsMenu(ProgramData &programData);
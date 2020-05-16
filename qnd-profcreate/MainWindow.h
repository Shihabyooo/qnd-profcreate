#pragma once
#include <Windows.h>
#include "imgui.h"
#include "FilesystemBrowser.h"
//#include "ProfileMaker.h"
#include "Globals.h"

extern ProfileMaker * profileMaker;

void DrawMainWindow();
void UpdateMainWindowSizeAndPos(long int resolutionX, long int resolutionY);
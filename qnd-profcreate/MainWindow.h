#pragma once
#include <Windows.h>
#include "imgui.h"
#include "FilesystemBrowser.h"
//#include "ProfileMaker.h"
#include "Globals.h"
#include "LogWindow.h"

#define DEM_LIST_ID 'D'
#define GEOMETRY_LIST_ID 'I'

extern ProfileMaker * profileMaker;

void DrawMainWindow();
void UpdateMainWindowSizeAndPos(long int resolutionX, long int resolutionY);
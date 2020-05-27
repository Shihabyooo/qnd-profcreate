#pragma once

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <d3dcompiler.h>
//#include <d3d10.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#include <wchar.h>
#include "MainWindow.h"
#include "LogWindow.h"
#include "ProfileMaker.h"
#include "Settings.h"

#define PROGRAMNAME		_T("QnD_ProfCreate")
#define WINDOWNAME		_T("QnD Profile Creator")

#define WINDOW_INIT_FAIL 100


static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;



int StartGUI(ProfileMaker * _profileMaker);
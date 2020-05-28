#pragma once
#include <Windows.h>
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
#include "Renderer.h"
#include "Globals.h"



class D3d11 : public D3d_Renderer
{
public:
	D3d11();
	~D3d11();

	virtual int Start();
	virtual int RenderLoop();
	virtual void HandleSizeChange(WPARAM wParam, LPARAM lParam);


protected:
	virtual void RenderFrame();
	virtual void Shutdown();

	void CreateRenderTarget();
	void CleanupRenderTarget();
	HRESULT CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	int InitializeAndShowWindow();
	void SetupDearIMGUI();

protected:
	ID3D11Device*            g_pd3dDevice = NULL;
	ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
	IDXGISwapChain*          g_pSwapChain = NULL;
	ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;
};
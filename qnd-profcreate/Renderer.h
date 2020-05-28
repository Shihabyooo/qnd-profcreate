#pragma once
#include <Windows.h>
#include "MainWindow.h"
#include "LogWindow.h"



extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//OpenGL implementation has its own WnProc, this one is only used only for D3d-based renderers.
LRESULT WINAPI D3dWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


class Renderer
{
public:
	Renderer() {};
	~Renderer() {};

	virtual int Start() { return 0; };
	virtual int RenderLoop() { return 0; };
	
protected: //methods
	virtual void RenderFrame() {}; //Implemented in derived classes.
	virtual void Shutdown() {}; //Implemented in derived classes.

	virtual void UpdateSubWindowsSizes(); //SubWindows = ImGUI windows.
	
	virtual void DrawWindowContent(); //Since the [current] Im:Gui window drawing method are angostic to renderering API, we make them base methods.

protected: //variables
	HWND windowHandle;
	WNDCLASSEX windowClass;
	ImVec4 clearColour;
};


class D3d_Renderer : protected Renderer
{
public:
	D3d_Renderer() {};
	~D3d_Renderer() {};

	virtual void HandleSizeChange(WPARAM wParam, LPARAM lParam) {}; //For use by D3dWndProc(). Because this method uses variables differing from D3d10 to D3d11, we leave implementation to derived classes.

};

extern D3d_Renderer * activeD3dRenderer;
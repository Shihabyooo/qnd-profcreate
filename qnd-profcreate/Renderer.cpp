#pragma once
#include "Renderer.h"

D3d_Renderer * activeD3dRenderer = NULL;

LRESULT WINAPI D3dWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:

		if (activeD3dRenderer != NULL)
			activeD3dRenderer->HandleSizeChange(wParam, lParam);

		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void Renderer::UpdateSubWindowsSizes()
{
	RECT clientSize;

	bool result = GetClientRect(windowHandle, &clientSize);

	if (!result)
	{
		Log(std::string("ERROR! Failed to get updated window size."), LOG_ERROR);
		return;
	}

	//std::cout << "updating (top*right*bottom*left)" << clientSize.top << " x " << clientSize.right << " x " << clientSize.bottom << " x " << clientSize.left << std::endl;

	UpdateMainWindowSizeAndPos(clientSize.right, clientSize.bottom);
	UpdateLogWindowSizeAndPos(clientSize.right, clientSize.bottom);
}

void Renderer::DrawWindowContent()
{
	bool showDemo = true;
	ImGui::ShowDemoWindow(&showDemo);

	DrawMainWindow();
	DrawLogWindow();
}

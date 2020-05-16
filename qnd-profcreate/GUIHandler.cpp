//A significant portion of this code is copied verbatem off Dear IMGUI's own examples.

#include "GUIHandler.h"

HWND windowHandle;
WNDCLASSEX windowClass;
ImVec4 clearColour;

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

HRESULT CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return E_FAIL;

	CreateRenderTarget();

	return S_OK;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void UpdateSubWindowsSizes() //SubWindows = ImGUI windows.
{
	RECT clientSize;

	bool result = GetClientRect(windowHandle, &clientSize);

	if (!result)
	{
		Log(std::string("ERROR! Failed to get updated window size."), LOG_ERROR);
		return;
	}

	std::cout << "updating (top*right*bottom*left)" << clientSize.top << " x " << clientSize.right << " x " << clientSize.bottom << " x " << clientSize.left << std::endl;

	UpdateMainWindowSizeAndPos(clientSize.right, clientSize.bottom);
	UpdateLogWindowSizeAndPos(clientSize.right, clientSize.bottom);

	

}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();
			if (wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED) //we only need to update subwindows if we change the size of the parent window. Minimizing (and sizes of other windows) are irrelevant.
				UpdateSubWindowsSizes();
		}
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

int InitializeAndShowWindow()
{
	// Create application window
	WNDCLASSEX windowClass = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, PROGRAMNAME, NULL };
	RegisterClassEx(&windowClass);
	windowHandle = CreateWindow(windowClass.lpszClassName, WINDOWNAME, WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, windowClass.hInstance, NULL);

	// Initialize Direct3D
	if (CreateDeviceD3D(windowHandle) < 0)
	{
		CleanupDeviceD3D();
		UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return 1;
	}

	SetWindowPos(windowHandle, HWND_TOP, 0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 0);

	// Show the window
	ShowWindow(windowHandle, SW_SHOWDEFAULT);
	UpdateWindow(windowHandle);
	
	return 0;
}

void SetupDearIMGUI()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(windowHandle);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	clearColour = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
}

int ProgramLoop()
{

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		// Poll and handle messages (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		//if (show_demo_window)
		bool showDemo = true;
		ImGui::ShowDemoWindow(&showDemo);
		
		DrawMainWindow();
		DrawLogWindow();

		// Rendering
		ImGui::Render();
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
		
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clearColour);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		g_pSwapChain->Present(1, 0); // Present with vsync
		//g_pSwapChain->Present(0, 0); // Present without vsync

	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	DestroyWindow(windowHandle);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

	return 0;
}

int StartGUI(ProfileMaker * _profileMaker)
{

	//test
	Log(std::string("To bait fish withal! If it would serve anything, it would feed my revenge! Or at least, that's how I think the quote goes. My memory is as reliable as a cheap chinese product tho, so don't hold me to it...."), LogEntryType::success);
	for (int i = 0; i < 50; i++)
		Log(std::string("test"), i % 4 == 0 ? LogEntryType::normal : (i % 3 == 0 ? LogEntryType::warning : (i % 2 == 0 ? LogEntryType::error : LogEntryType::success)));
		

	//endtest

	if (InitializeAndShowWindow() != 0)
		return WINDOW_INIT_FAIL;

	SetupDearIMGUI();

	//Set the ProfileMaker *  in MainWindow.
	profileMaker = _profileMaker;

	//Start the program loop (this will not return until window quits or faces a fatal error).
	int result = ProgramLoop();


	return result;
}


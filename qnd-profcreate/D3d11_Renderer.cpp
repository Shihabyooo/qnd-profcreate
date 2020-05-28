#include "D3d11_Renderer.h"

D3d11::D3d11()
{
}

D3d11::~D3d11()
{
}

int D3d11::Start()
{
	if (InitializeAndShowWindow() != 0)
		return WINDOW_INIT_FAIL;
	SetupDearIMGUI();

}

int D3d11::RenderLoop()
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

		DrawWindowContent();

		RenderFrame();
	}

	Shutdown();

	return 0;
}

void D3d11::HandleSizeChange(WPARAM wParam, LPARAM lParam)
{
	if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
	{
		CleanupRenderTarget();
		g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
		CreateRenderTarget();
		
		if (wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED) //we only need to update subwindows if we change the size of the parent window. Minimizing (and sizes of other windows) are irrelevant.
			UpdateSubWindowsSizes();
	}
}

void D3d11::RenderFrame()
{
	// Rendering
	ImGui::Render();
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);

	g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clearColour);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	g_pSwapChain->Present(1, 0); // Present with vsync
	//g_pSwapChain->Present(0, 0); // Present without vsync
}

void D3d11::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	DestroyWindow(windowHandle);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

void D3d11::CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void D3d11::CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

HRESULT D3d11::CreateDeviceD3D(HWND hWnd)
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

void D3d11::CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

int D3d11::InitializeAndShowWindow()
{
	// Create application window
	WNDCLASSEX windowClass = { sizeof(WNDCLASSEX), CS_CLASSDC, D3dWndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, PROGRAMNAME, NULL };
	RegisterClassEx(&windowClass);
	windowHandle = CreateWindow(windowClass.lpszClassName, WINDOWNAME, WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, windowClass.hInstance, NULL);

	// Initialize Direct3D
	if (CreateDeviceD3D(windowHandle) < 0)
	{
		CleanupDeviceD3D();
		UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return 1;
	}

	SetWindowPos(windowHandle, HWND_TOP, 0, 0, settings.windowWidth, settings.windowHeight, 0);

	// Show the window
	ShowWindow(windowHandle, SW_SHOWDEFAULT);
	UpdateWindow(windowHandle);

	return 0;
}

void D3d11::SetupDearIMGUI()
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

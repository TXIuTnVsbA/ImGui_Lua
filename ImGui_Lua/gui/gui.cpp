#include "gui.h"
#include "imgui\imgui.h"
#include "imgui\imgui_impl_dx9.h"
#include "imgui\imgui_impl_win32.h"
#include "imgui/imgui_internal.h"
#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#include "../config/config.h"
#include "../utils/console/console.h"
#include "../lua/includes.h"

//constexpr auto windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
//| ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

constexpr auto windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

// Data
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};
static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return false;

	// Create the D3DDevice
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
		return false;

	return true;
}

void CleanupDeviceD3D()
{
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
	if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}

// Win32 message handler
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
			g_d3dpp.BackBufferWidth = LOWORD(lParam);
			g_d3dpp.BackBufferHeight = HIWORD(lParam);
			ResetDevice();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

void dmt(std::string key) {
	if (GET_BOOL["lua_devmode"] && ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::Text(key.c_str());
		ImGui::EndTooltip();
	}
}

void draw_lua_items(std::string tab, std::string container) {
	for (auto i : lua::menu_items[tab][container]) {
		if (!i.is_visible)
			continue;

		auto type = i.type;
		switch (type)
		{
		case lua::MENUITEM_CHECKBOX:
			if (ImGui::Checkbox(i.label.c_str(), &(GET_BOOL[i.key]))) {
				if (i.callback != sol::nil)
					i.callback(GET_BOOL[i.key]);
			}

			dmt(i.key);
			break;
		case lua::MENUITEM_SLIDERINT:
			if (ImGui::SliderInt(i.label.c_str(), &(GET_INT[i.key]), i.i_min, i.i_max, i.format.c_str())) {
				if (i.callback != sol::nil)
					i.callback(GET_INT[i.key]);
			}

			dmt(i.key);
			break;
		case lua::MENUITEM_SLIDERFLOAT:
			if (ImGui::SliderFloat(i.label.c_str(), &(GET_FLOAT[i.key]), i.f_min, i.f_max, i.format.c_str())) {
				if (i.callback != sol::nil)
					i.callback(GET_FLOAT[i.key]);
			}

			dmt(i.key);
			break;
			/*
		case lua::MENUITEM_KEYBIND:
			if (ImGui::Keybind(i.label.c_str(), &cfg->i[i.key], i.allow_style_change ? &cfg->i[i.key + "style"] : NULL)) {
				if (i.callback != sol::nil)
					i.callback(cfg->i[i.key], cfg->i[i.key + "style"]);
			}

			dmt(i.key + (i.allow_style_change ? " | " + i.key + "style" : ""));
			break;
			*/
		case lua::MENUITEM_TEXT:
			ImGui::Text(i.label.c_str());
			break;

			/*
		case lua::MENUITEM_SINGLESELECT:
			if (ImGui::SingleSelect(i.label.c_str(), &cfg->i[i.key], i.items)) {
				if (i.callback != sol::nil)
					i.callback(cfg->i[i.key]);
			}

			dmt(i.key);
			break;

		case lua::MENUITEM_MULTISELECT:
			if (ImGui::MultiSelect(i.label.c_str(), &cfg->m[i.key], i.items)) {
				if (i.callback != sol::nil)
					i.callback(cfg->m[i.key]);
			}

			dmt(i.key);
			break;
			*/
		case lua::MENUITEM_COLORPICKER:
			if (ImGui::ColorEdit4(i.label.c_str(), GET_COLOR[i.key])) {
				if (i.callback != sol::nil)
					i.callback(GET_COLOR[i.key][0] * 255, GET_COLOR[i.key][1] * 255, GET_COLOR[i.key][2] * 255, GET_COLOR[i.key][3] * 255);
			}

			dmt(i.key);
			break;
		case lua::MENUITEM_BUTTON:
			if (ImGui::Button(i.label.c_str())) {
				if (i.callback != sol::nil)
					i.callback();
			}
			break;
		default:
			break;
		}
	}
}

void draw_config_window(bool* isOpened) {
	ImGui::SetNextWindowSize({ 290.0f, 190.0f });
	ImGui::Columns(2, nullptr, false);
	ImGui::SetColumnOffset(1, 170.0f);

	ImGui::PushItemWidth(500.f);
	constexpr auto& configItems = g_config.getConfigs();
	static int currentConfig = -1;

	if (static_cast<size_t>(currentConfig) >= configItems.size())
		currentConfig = -1;

	static char buffer[16];

	if (ImGui::ListBox("", &currentConfig, [](void* data, int idx, const char** out_text) {
		auto& vector = *static_cast<std::vector<std::string>*>(data);
		*out_text = vector[idx].c_str();
		return true;
		}, &configItems, configItems.size(), 5) && currentConfig != -1)
		strcpy(buffer, configItems[currentConfig].c_str());

		ImGui::PushID(0);
		if (ImGui::InputText("", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
			if (currentConfig != -1)
				g_config.rename(currentConfig, buffer);
		}
		ImGui::PopID();
		ImGui::PopItemWidth();
		ImGui::NextColumn();

		ImGui::PushItemWidth(100.0f);

		if (ImGui::Button("Create config", { 100.0f, 25.0f }))
			g_config.add(buffer);

		if (ImGui::Button("Reset config", { 100.0f, 25.0f }))
			g_config.reset();

		if (currentConfig != -1) {
			if (ImGui::Button("Load selected", { 100.0f, 25.0f }))
				g_config.load(currentConfig);
			if (ImGui::Button("Save selected", { 100.0f, 25.0f }))
				g_config.save(currentConfig);
			if (ImGui::Button("Delete selected", { 100.0f, 25.0f }))
				g_config.remove(currentConfig);
		}
		ImGui::PopItemWidth();
}

void draw_lua_window(bool* isOpened) {
	ImGui::SetNextWindowSize({ 290.0f, 190.0f });
	ImGui::Columns(2, nullptr, false);
	ImGui::SetColumnOffset(1, 170.0f);

	ImGui::PushItemWidth(500.f);
	constexpr auto& configItems = lua::scripts;
	static int currentConfig = -1;

	if (static_cast<size_t>(currentConfig) >= configItems.size())
		currentConfig = -1;

	static char buffer[16];

	if (ImGui::ListBox("", &currentConfig, [](void* data, int idx, const char** out_text) {
		auto& vector = *static_cast<std::vector<std::string>*>(data);
		*out_text = vector[idx].c_str();
		return true;
		}, &configItems, configItems.size(), 5) && currentConfig != -1)
		strcpy(buffer, configItems[currentConfig].c_str());

	ImGui::PopItemWidth();
	ImGui::NextColumn();

	ImGui::PushItemWidth(100.0f);
	if (ImGui::Button("Refresh", { 100.0f, 25.0f }))
	{
		lua::unload_all_scripts();
		lua::refresh_scripts();
		lua::load_script(lua::get_script_id("autorun.lua"));
	}

	if (ImGui::Button("Reload All", { 100.0f, 25.0f }))
		lua::reload_all_scripts();

	if (ImGui::Button("Unload All", { 100.0f, 25.0f }))
		lua::unload_all_scripts();

	if (currentConfig != -1) {
		if (lua::loaded[currentConfig])
		{
			if (ImGui::Button("Unload selected", { 100.0f, 25.0f }))
			{
				lua::unload_script(currentConfig);
			}
		}
		else {
			if (ImGui::Button("Load selected", { 100.0f, 25.0f }))
			{
				lua::load_script(currentConfig);
			}
		}
		
	}
	ImGui::PopItemWidth();
}

void draw_console_window(bool* isOpened) {
	char InputBuf[512] = {};
	ImGui::PushID(0);
	if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
		try
		{
			bool tmp = luaL_dostring(lua::g_lua_state, InputBuf);
			if (tmp) {
				std::printf("lua::Error: %s\n", lua_tostring(lua::g_lua_state, -1));
			}
		}
		catch (const std::exception & Error)
		{
			std::printf("std::Error: %s\n", Error.what());
			//std::printf("lua::Error: %s\n", lua_tostring(g_lua_state, 0));
			//g_console.log("std::Error: %s", Error.what());
			//g_console.log("lua::Error: %s", lua_tostring(g_lua_state, 0));
		}
	}
	ImGui::PopID();

	std::string OutputBuffer;
	if (OutputBuffer.size() >= OutputBuffer.max_size() - 1) {
		OutputBuffer.clear();
	}
	if (sizeof(g_console.out_buf) >= BLOCK_SIZE - 1) {
		g_console.out_buf[BLOCK_SIZE] = {};
	}
	for (int i = 0; i < BLOCK_SIZE; i++) {
		if (g_console.out_buf[i] != '\0') {
			OutputBuffer.append(1, g_console.out_buf[i]);
		}
		else {
			break;
		}
	}
	ImGui::PushID(1);
	ImGui::TextUnformatted(OutputBuffer.c_str());
	ImGui::PopID();

}

void draw_lua_items_window() {
	for (auto tab : GET_STRINGS[g_config.gui.tab_name]) {
		auto tab_name = tab.second;
		auto tab_name_str = tab_name.c_str();
		bool* window = &(GET_BOOLS_MAP[g_config.gui.tab_bool][tab_name]);
		if (!(*window))
			continue;

		if (tab_name == "Demo")
		{
			ImGui::ShowDemoWindow(window);
			continue;
		}

		ImGui::Begin(tab_name_str, window, windowFlags);

		if (tab_name == "Config")
			draw_config_window(window);
		else if (tab_name == "Lua")
			draw_lua_window(window);
		else if (tab_name == "Console")
			draw_console_window(window);

		float w = GET_FLOATS_MAP[g_config.gui.w][tab_name];
		float h = GET_FLOATS_MAP[g_config.gui.h][tab_name];
		ImGui::SetWindowSize(ImVec2(w, h));

		int container_count = GET_INTS_MAP[g_config.gui.container_count][tab_name];
		if (container_count > 0) {
			for (int i = 0; i < container_count; i++) {
				char buffer[8] = { 0 }; _itoa(i, buffer, 10);
				draw_lua_items(tab_name, buffer);
			}
		}
		ImGui::End();
	}
}

void draw() {
	// 1. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	/*
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		//ImGui::Checkbox("Demo Window", show_demo_window);      // Edit bools storing our window open/close state
		//ImGui::Checkbox("Another Window", show_another_window);
		for (auto tab : GET_STRINGS[g_config.gui.tab_name]) {
			auto tab_name = tab.second.c_str();
			ImGui::Checkbox(tab_name, &(GET_BOOLS_MAP[g_config.gui.tab_bool][tab_name]));
		}
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	*/
	if (ImGui::BeginMainMenuBar()) {
		for (auto tab : GET_STRINGS[g_config.gui.tab_name]) {
			auto tab_name = tab.second.c_str();
			//ImGui::Checkbox(tab_name, &(GET_BOOLS_MAP[g_config.gui.tab_bool][tab_name]));
			ImGui::MenuItem(tab_name, nullptr, &(GET_BOOLS_MAP[g_config.gui.tab_bool][tab_name]));
		}
		ImGui::EndMainMenuBar();
	}
	// 2.draw_other
	draw_lua_items_window();
}

DWORD WINAPI imgui_main(LPVOID base) {
	// Create application window
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
	::RegisterClassEx(&wc);
	HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Dear ImGui DirectX9 Example"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX9_Init(g_pd3dDevice);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	// Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		// Poll and handle messages (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			continue;
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		draw();

		// Rendering
		ImGui::EndFrame();
		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
		D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * 255.0f), (int)(clear_color.y * 255.0f), (int)(clear_color.z * 255.0f), (int)(clear_color.w * 255.0f));
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
		if (g_pd3dDevice->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			g_pd3dDevice->EndScene();
		}
		HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

		// Handle loss of D3D9 device
		if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			ResetDevice();
	}

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);

	return TRUE;
}

c_gui g_gui;
bool c_gui::init() noexcept {
	HANDLE init_thread = CreateThread(nullptr, NULL, imgui_main, NULL, NULL, nullptr);
	if (!init_thread)
		return false;

	CloseHandle(init_thread);
	return TRUE;
}


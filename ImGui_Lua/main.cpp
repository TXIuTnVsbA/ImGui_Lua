#include <iostream>
#include <Windows.h>
#include <thread>
#include "lua/includes.h"
#include "utils/KeyState.h"
#include "utils/console/console.h"
#include "config/config.h"
#include "gui/gui.h"

void init() {
#ifdef _DEBUG
	if (!g_console.allocate("Debug"))
		std::abort();

	g_console.log("INIT");

	// redirect warnings to a window similar to errors.
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_WNDW);

#else
	g_console.allocate();
	FreeConsole();
#endif
	lua::init_state();

	g_config.init();
	g_config.default();

	if (!g_gui.init())
	{
#ifdef _DEBUG
		g_console.log("Error_Gui_Init");
#endif
		std::abort();
	}

	lua::init_command();
}
void load() {
	for (auto hk : lua::hooks->getHooks("on_load"))
	{
		try
		{
			hk.func();
		}
		catch (const std::exception&)
		{

		}
	}
}
void wait() {
#ifdef _DEBUG
	g_console.log("WAIT");
	lua::init_console();
#else
	while (!lua::g_unload_flag)
	{
		if (KEYDOWN(VK_END)) {
			lua::g_unload_flag = true;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
#endif
}
void unload() {
	for (auto hk : lua::hooks->getHooks("on_unload"))
	{
		try
		{
			hk.func();
		}
		catch (const std::exception&)
		{

		}
	}
#ifdef _DEBUG
	g_console.log("UNLOAD");
	g_console.detach();
#else
#endif
}

int main()
{
	try
	{
		//INIT
		init();

		//LOAD
		load();

		//WAIT
		wait();

		//UNLOAD
		unload();
	}
	catch (const std::exception&)
	{
#ifdef _DEBUG
		g_console.log("ERROR");
		g_console.detach();
#else
#endif
	}
    return TRUE;
}

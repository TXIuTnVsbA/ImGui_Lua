#pragma once
//#include "../utils/singleton.h"
#include "CLuaHook.h"
#include <filesystem>
namespace lua {
	enum MENUITEMTYPE {
		MENUITEM_CHECKBOX = 0,
		MENUITEM_SLIDERINT,
		MENUITEM_SLIDERFLOAT,
		MENUITEM_KEYBIND,
		MENUITEM_TEXT,
		MENUITEM_SINGLESELECT,
		MENUITEM_MULTISELECT,
		MENUITEM_COLORPICKER,
		MENUITEM_BUTTON
	};

	struct MenuItem_t {
		MENUITEMTYPE type;
		int script = -1;
		std::string label = "";
		std::string key = "";

		bool is_visible = true;

		// defaults
		bool b_default = false;
		int i_default = 0;
		float f_default = 0.f;
		float c_default[4] = { 1.f, 1.f, 1.f, 1.f };
		std::map<int, bool> m_default = {};

		// keybinds
		bool allow_style_change = true;

		// singleselect & multiselect
		std::vector<const char*> items = {};

		// slider_int
		int i_min = 0;
		int i_max = 100;

		// slider_float
		float f_min = 0.f;
		float f_max = 1.f;

		// sliders
		std::string format = "%d";

		// callbacks
		sol::function callback;
	};

	void init_state();
	void init_command();
	void init_console();
	void unload();

	void load_script(int id);
	void unload_script(int id);
	void reload_all_scripts();
	void unload_all_scripts();
	void refresh_scripts();
	int get_script_id(std::string name);
	int get_script_id_by_path(std::string path);
	std::string get_script_path(std::string name);
	std::string get_script_path(int id);
	
	extern std::map<std::string, std::map<std::string, std::vector<MenuItem_t>>> menu_items;
	extern std::vector<std::filesystem::path> pathes;
	extern std::vector<bool> loaded;
	extern std::vector<std::string> scripts;
	extern c_lua_hookManager* hooks;
	extern lua_State* g_lua_state;
	extern bool g_unload_flag;
}

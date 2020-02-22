#include <fstream>
#include <ShlObj.h>
#include "CLua.h"
#include "../utils/console/console.h"
#include "../utils/KeyState.h"
#include "../config/config.h"
namespace lua {
	bool g_unload_flag = false;
	lua_State* g_lua_state = NULL;
	c_lua_hookManager* hooks = new c_lua_hookManager();
	std::vector<bool> loaded;
	std::vector<std::string> scripts;
	std::vector<std::filesystem::path> pathes;
	std::map<std::string, std::map<std::string, std::vector<MenuItem_t>>> menu_items = {};

	int extract_owner(sol::this_state st) {
		sol::state_view lua_state(st);
		sol::table rs = lua_state["debug"]["getinfo"](2, "S");
		std::string source = rs["source"];
		std::string filename = std::filesystem::path(source.substr(1)).filename().string();
		return get_script_id(filename);
	}

	namespace ns_client {
		void set_event_callback(sol::this_state s, std::string eventname, sol::function func) {
			sol::state_view lua_state(s);
			sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
			std::string source = rs["source"];
			std::string filename = std::filesystem::path(source.substr(1)).filename().string();

			hooks->registerHook(eventname, get_script_id(filename), func);

			g_console.log("%s: subscribed to event %s", filename.c_str(), eventname.c_str());
		}

		void run_script(std::string scriptname) {
			load_script(get_script_id(scriptname));
		}

		void reload_active_scripts() {
			reload_all_scripts();
		}

		void refresh() {
			unload_all_scripts();
			refresh_scripts();
			load_script(get_script_id("autorun.lua"));
		}
	};

	namespace ns_config {
		/*
		config.get(key)
		Returns value of given key or nil if key not found.
		*/
		std::tuple<sol::object, sol::object, sol::object, sol::object> get(sol::this_state s, std::string key) {
			std::tuple<sol::object, sol::object, sol::object, sol::object> retn = std::make_tuple(sol::nil, sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.b)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.c)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, (int)(kv.second[0] * 255)), sol::make_object(s, (int)(kv.second[1] * 255)), sol::make_object(s, (int)(kv.second[2] * 255)), sol::make_object(s, (int)(kv.second[3] * 255)));

			for (auto kv : g_config.f)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);


			for (auto kv : g_config.i)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.s)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.i_b)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.i_f)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.i_i)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.i_s)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.s_b)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.s_f)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.s_i)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.s_s)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			return retn;
		}

		/*
		config.set(key, value)
		Sets value for key
		*/
		void set_bool(std::string key, bool v) {
			g_config.b[key] = v;
		}

		void set_string(std::string key, std::string v) {
			g_config.s[key] = v;
		}

		void set_float(std::string key, float v) {
			if (ceilf(v) != v)
				g_config.f[key] = v;
			else
				g_config.i[key] = (int)v;
		}

		/*
		config.set(key, r,g,b,a)
		*/
		void set_color(std::string key, int r, int g, int b, int a) {
			g_config.c[key][0] = r / 255.f;
			g_config.c[key][1] = g / 255.f;
			g_config.c[key][2] = b / 255.f;
			g_config.c[key][3] = a / 255.f;
		}

		/*
		config.set(key, index, value)
		*/
		void set_select_tuple(std::string key, int pos, bool v) {
			g_config.i_b[key][pos] = v;
		}

		void set_string_tuple(std::string key, int pos, std::string v) {
			g_config.i_s[key][pos] = v;
		}

		void set_number_tuple(std::string key, int pos, float v) {
			if (ceilf(v) != v)
				g_config.i_f[key][pos] = v;
			else
				g_config.i_i[key][pos] = (int)v;

		}

		void set_select_map(std::string key, std::string pos, bool v) {
			g_config.s_b[key][pos] = v;
		}

		void set_string_map(std::string key, std::string pos, std::string v) {
			g_config.s_s[key][pos] = v;
		}

		void set_number_map(std::string key, std::string pos, float v) {
			if (ceilf(v) != v)
				g_config.s_f[key][pos] = v;
			else
				g_config.s_i[key][pos] = (int)v;

		}

		/*
		config.load()
		Loads selected config
		*/
		void load(size_t id) {
			g_config.load(id);
		}

		/*
		config.save()
		Saves selected config
		*/
		void save(size_t id) {
			g_config.save(id);
		}
	};

	namespace ns_ui {
		std::string new_checkbox(sol::this_state s, std::string tab, std::string container, std::string label, std::string key, std::optional<bool> def, std::optional<sol::function> cb) {
			MenuItem_t item;
			item.type = MENUITEM_CHECKBOX;
			item.script = extract_owner(s);
			item.label = label;
			item.key = key;
			item.b_default = def.value_or(false);
			item.callback = cb.value_or(sol::nil);

			menu_items[tab][container].push_back(item);
			return key;
		}

		std::string new_slider_int(sol::this_state s, std::string tab, std::string container, std::string label, std::string key, int min, int max, std::optional<std::string> format, std::optional<int> def, std::optional<sol::function> cb) {
			MenuItem_t item;
			item.type = MENUITEM_SLIDERINT;
			item.script = extract_owner(s);
			item.label = label;
			item.key = key;
			item.i_default = def.value_or(0);
			item.i_min = min;
			item.i_max = max;
			item.format = format.value_or("%d");
			item.callback = cb.value_or(sol::nil);

			menu_items[tab][container].push_back(item);
			return key;
		}

		std::string new_slider_float(sol::this_state s, std::string tab, std::string container, std::string label, std::string key, float min, float max, std::optional<std::string> format, std::optional<float> def, std::optional<sol::function> cb) {
			MenuItem_t item;
			item.type = MENUITEM_SLIDERFLOAT;
			item.script = extract_owner(s);
			item.label = label;
			item.key = key;
			item.f_default = def.value_or(0.f);
			item.f_min = min;
			item.f_max = max;
			item.format = format.value_or("%.0f");
			item.callback = cb.value_or(sol::nil);

			menu_items[tab][container].push_back(item);
			return key;
		}

		std::string new_text(sol::this_state s, std::string tab, std::string container, std::string label, std::string key) {
			MenuItem_t item;
			item.type = MENUITEM_TEXT;
			item.script = extract_owner(s);
			item.label = label;
			item.key = key;

			menu_items[tab][container].push_back(item);
			return key;
		}

		std::string new_colorpicker(sol::this_state s, std::string tab, std::string container, std::string id, std::string key, std::optional<int> r, std::optional<int> g, std::optional<int> b, std::optional<int> a, std::optional<sol::function> cb) {
			MenuItem_t item;
			item.type = MENUITEM_COLORPICKER;
			item.script = extract_owner(s);
			item.label = id;
			item.key = key;
			item.c_default[0] = r.value_or(255) / 255.f;
			item.c_default[1] = g.value_or(255) / 255.f;
			item.c_default[2] = b.value_or(255) / 255.f;
			item.c_default[3] = a.value_or(255) / 255.f;
			item.callback = cb.value_or(sol::nil);

			menu_items[tab][container].push_back(item);
			return key;
		}

		std::string new_button(sol::this_state s, std::string tab, std::string container, std::string id, std::string key, std::optional<sol::function> cb) {
			MenuItem_t item;
			item.type = MENUITEM_BUTTON;
			item.script = extract_owner(s);
			item.label = id;
			item.key = key;
			item.callback = cb.value_or(sol::nil);

			menu_items[tab][container].push_back(item);
			return key;
		}

		void set_visibility(std::string key, bool v) {
			for (auto t : menu_items) {
				for (auto c : t.second) {
					for (auto& i : c.second) {
						if (i.key == key)
							i.is_visible = v;
					}
				}
			}
		}

		void set_items(std::string key, std::vector<const char*> items) {
			for (auto t : menu_items) {
				for (auto c : t.second) {
					for (auto& i : c.second) {
						if (i.key == key)
							i.items = items;
					}
				}
			}
		}

	};

	void test_func() {
		g_console.log("RunTestFunc");
		for (auto hk : hooks->getHooks("on_test"))
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

	void init_state() {
		lua_writestring(LUA_COPYRIGHT, strlen(LUA_COPYRIGHT));
		lua_writeline();
		lua::unload();
		g_lua_state = luaL_newstate();
		luaL_openlibs(g_lua_state);
	}

	void init_command() {
		sol::state_view lua_state(g_lua_state);
		lua_state["exit"] = []() { g_unload_flag = true; };
		lua_state["test_func"] = []() { test_func(); };

		auto config = lua_state.create_table();
		config["get"] = ns_config::get;
		config["set"] = sol::overload(
			ns_config::set_bool,
			ns_config::set_color,
			ns_config::set_float,
			ns_config::set_string,
			ns_config::set_select_tuple,
			ns_config::set_number_tuple,
			ns_config::set_string_tuple,
			ns_config::set_select_map,
			ns_config::set_number_map,
			ns_config::set_string_map
		);
		config["save"] = ns_config::save;
		config["load"] = ns_config::load;
		lua_state["config"] = config;

		auto client = lua_state.create_table();
		client["set_event_callback"] = ns_client::set_event_callback;
		client["run_script"] = ns_client::run_script;
		client["reload_active_scripts"] = ns_client::reload_active_scripts;
		client["refresh"] = ns_client::refresh;
		lua_state["client"] = client;

		auto ui = lua_state.create_table();
		ui["new_checkbox"] = ns_ui::new_checkbox;
		ui["new_colorpicker"] = ns_ui::new_colorpicker;
		//ui["new_keybind"] = ns_ui::new_keybind;
		//ui["new_multiselect"] = ns_ui::new_multiselect;
		//ui["new_singleselect"] = ns_ui::new_singleselect;
		ui["new_slider_float"] = ns_ui::new_slider_float;
		ui["new_slider_int"] = ns_ui::new_slider_int;
		ui["new_text"] = ns_ui::new_text;
		ui["new_button"] = ns_ui::new_button;
		//ui["set_callback"] = ns_ui::set_callback;
		ui["set_items"] = ns_ui::set_items;
		//ui["set_label"] = ns_ui::set_label;
		ui["set_visibility"] = ns_ui::set_visibility;
		//ui["is_bind_active"] = ns_ui::is_bind_active;
		lua_state["ui"] = ui;

		refresh_scripts();
		load_script(get_script_id("autorun.lua"));

	}

	void init_console() {
		std::string str;
		//printf("lua_ptr: 0x%x \n", &g_console);
		g_console.log("LUA");
		std::printf(">");
		while (!g_unload_flag && std::getline(std::cin, str))
		{
			try
			{
				bool tmp = luaL_dostring(g_lua_state, str.c_str());
				if (tmp) {
					std::printf("lua::Error: %s\n", lua_tostring(g_lua_state, -1));
				}
			}
			catch (const std::exception & Error)
			{
				std::printf("std::Error: %s\n", Error.what());
				//std::printf("lua::Error: %s\n", lua_tostring(g_lua_state, 0));
				//g_console.log("std::Error: %s", Error.what());
				//g_console.log("lua::Error: %s", lua_tostring(g_lua_state, 0));
			}
			g_console.log("LUA");
			std::printf(">");
		}
	}

	void unload() {
		if (g_lua_state != NULL) {
			lua_close(g_lua_state);
			g_lua_state = NULL;
		}
	}

	void load_script(int id) {
		if (id == -1)
			return;

		if (loaded.at(id))
			return;

		auto path = get_script_path(id);
		if (path == (""))
			return;

		sol::state_view state(g_lua_state);
		state.script_file(path, [](lua_State*, sol::protected_function_result result) {
			if (!result.valid()) {
				sol::error err = result;
				//Utilities->Game_Msg(err.what());
				g_console.log(err.what());
			}

			return result;
			});

		loaded.at(id) = true;
	}

	void unload_script(int id) {
		if (id == -1)
			return;

		if (!loaded.at(id))
			return;

		std::map<std::string, std::map<std::string, std::vector<MenuItem_t>>> updated_items;
		for (auto i : menu_items) {
			for (auto k : i.second) {
				std::vector<MenuItem_t> updated_vec;

				for (auto m : k.second)
					if (m.script != id)
						updated_vec.push_back(m);

				updated_items[k.first][i.first] = updated_vec;
			}
		}
		menu_items = updated_items;

		hooks->unregisterHooks(id);
		loaded.at(id) = false;
	}

	void reload_all_scripts() {
		for (auto s : scripts) {
			if (loaded.at(get_script_id(s))) {
				unload_script(get_script_id(s));
				load_script(get_script_id(s));
			}
		}
	}

	void unload_all_scripts() {
		for (auto s : scripts)
			if (loaded.at(get_script_id(s)))
				unload_script(get_script_id(s));
	}

	void refresh_scripts() {
		std::filesystem::path path;
		path = g_config.get_path();
		path /= "Lua";

		if (!std::filesystem::is_directory(path)) {
			std::filesystem::remove(path);
			std::filesystem::create_directory(path);
		}

		auto oldLoaded = loaded;
		auto oldScripts = scripts;

		loaded.clear();
		pathes.clear();
		scripts.clear();

		for (auto& entry : std::filesystem::directory_iterator((path))) {
			if (entry.path().extension() == (".lua")) {
				auto path = entry.path();
				auto filename = path.filename().string();

				bool didPut = false;
				int oldScriptsSize = 0;
				oldScriptsSize = oldScripts.size();
				if (oldScriptsSize <= 0)
					continue;

				for (int i = 0; i < oldScriptsSize; i++) {
					if (filename == oldScripts.at(i)) {
						loaded.push_back(oldLoaded.at(i));
						didPut = true;
					}
				}

				if (!didPut)
					loaded.push_back(false);

				pathes.push_back(path);
				scripts.push_back(filename);
			}
		}
	}

	int get_script_id(std::string name) {
		int scriptsSize = 0;
		scriptsSize = scripts.size();
		if (scriptsSize <= 0)
			return -1;

		for (int i = 0; i < scriptsSize; i++) {
			if (scripts.at(i) == name)
				return i;
		}

		return -1;
	}

	int get_script_id_by_path(std::string path) {
		int pathesSize = 0;
		pathesSize = pathes.size();
		if (pathesSize <= 0)
			return -1;

		for (int i = 0; i < pathesSize; i++) {
			if (pathes.at(i).string() == path)
				return i;
		}

		return -1;
	}

	std::string get_script_path(std::string name) {
		return get_script_path(get_script_id(name));
	}

	std::string get_script_path(int id) {
		if (id == -1)
			return  "";

		return pathes.at(id).string();
	}
};
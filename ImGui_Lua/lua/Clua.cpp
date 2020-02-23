#include <fstream>
#include <ShlObj.h>
#include "CLua.h"
#include "../utils/console/console.h"
#include "../utils/KeyState.h"
#include "../config/config.h"

#include <d3d9.h>
#include "../gui/imgui/imgui.h"
#include "../gui/imgui/imgui_impl_dx9.h"
#include "../gui/imgui/imgui_impl_win32.h"
#include "../gui/imgui/imgui_internal.h"
#define M(x,y) {##x[#y] = ns_##x::##y;}
#define N(x,y) {##x[#y] = ##y;}

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

		void load(size_t id) {
			g_config.load(id);
		}

		void save(size_t id) {
			g_config.save(id);
		}

		void add(std::string name) {
			g_config.add(name.c_str());
		}

		void remove(size_t id) {
			g_config.remove(id);
		}

		void rename(size_t id, std::string name) {
			g_config.rename(id, name.c_str());
		}

		void reset() {
			g_config.reset();
		}

		void init() {
			g_config.init();
		}

		sol::object configs(sol::this_state this_) {
			auto configs = g_config.getConfigs();
			if (!configs.empty())
			{
				return sol::make_object(this_, configs);
			}
			else {
				return sol::nil;
			}
		}

	};

	namespace ns_log {
		void enable_log_file(std::string filename) {
			g_console.enable_log_file(filename);
		}
	};

	namespace ns_imgui {
		// window
		bool begin_window1(std::string name) {
			return ImGui::Begin(name.c_str());
		}
		bool begin_window2(std::string name, std::string field_from_config) {
			return ImGui::Begin(name.c_str(), &(GET_BOOL[field_from_config]));
		}
		bool begin_window3(std::string name, std::string field_from_config, int flag) {
			return ImGui::Begin(name.c_str(), &(GET_BOOL[field_from_config]), flag);
		}
		void end_window() {
			ImGui::End();
		}

		bool begin_child1(int id) {
			return ImGui::BeginChild(id);
		}
		bool begin_child2(std::string id) {
			return ImGui::BeginChild(id.c_str());
		}
		bool begin_child3(int id, float x, float y) {
			return ImGui::BeginChild(id, ImVec2(x, y));
		}
		bool begin_child4(std::string id, float x, float y) {
			return ImGui::BeginChild(id.c_str(), ImVec2(x, y));
		}
		bool begin_child5(int id, float x, float y, bool border) {
			return ImGui::BeginChild(id, ImVec2(x, y), border);
		}
		bool begin_child6(std::string id, float x, float y, bool border) {
			return ImGui::BeginChild(id.c_str(), ImVec2(x, y), border);
		}
		bool begin_child7(int id, float x, float y, bool border, int flag) {
			return ImGui::BeginChild(id, ImVec2(x, y), border, flag);
		}
		bool begin_child8(std::string id, float x, float y, bool border, int flag) {
			return ImGui::BeginChild(id.c_str(), ImVec2(x, y), border, flag);
		}
		void end_child() {
			ImGui::EndChild();
		}

		void set_window_pos1(float x, float y) {
			ImGui::SetWindowPos(ImVec2(x, y));
		}
		void set_window_pos2(float x, float y, int cond) {
			ImGui::SetWindowPos(ImVec2(x, y), cond);
		}
		void set_window_size1(float x, float y) {
			ImGui::SetWindowSize(ImVec2(x, y));
		}
		void set_window_size2(float x, float y, int cond) {
			ImGui::SetWindowSize(ImVec2(x, y), cond);
		}
		void set_window_focus1() {
			ImGui::SetWindowFocus();
		}
		void set_window_focus2(std::string name) {
			ImGui::SetWindowFocus(name.c_str());
		}
		void set_window_collapsed1(bool collapsed) {
			ImGui::SetWindowCollapsed(collapsed);
		}
		void set_window_collapsed2(bool collapsed, int cond) {
			ImGui::SetWindowCollapsed(collapsed, cond);
		}
		void set_window_collapsed3(std::string name, bool collapsed, int cond) {
			ImGui::SetWindowCollapsed(name.c_str(), collapsed, cond);
		}
		void set_next_window_pos1(float pos_x, float pos_y) {
			ImGui::SetNextWindowPos(ImVec2(pos_x, pos_y));
		}
		void set_next_window_pos2(float pos_x, float pos_y, int cond) {
			ImGui::SetNextWindowPos(ImVec2(pos_x, pos_y), cond);
		}
		void set_next_window_pos3(float pos_x, float pos_y, int cond, float pivot_x, float pivot_y) {
			ImGui::SetNextWindowPos(ImVec2(pos_x, pos_y), cond, ImVec2(pivot_x, pivot_y));
		}
		void set_next_window_content_size(float x, float y) {
			ImGui::SetNextWindowContentSize(ImVec2(x, y));

		}
		void set_next_window_size1(float x, float y) {
			ImGui::SetNextWindowSize(ImVec2(x, y));
		}
		void set_next_window_size2(float x, float y, int cond) {
			ImGui::SetNextWindowSize(ImVec2(x, y), cond);
		}
		void set_next_window_size_constraints(float min_x, float min_y, float max_x, float max_y) {
			ImGui::SetNextWindowSizeConstraints(ImVec2(min_x, min_y), ImVec2(max_x, max_y));
		}
		void set_next_window_collapsed1(bool collapsed) {
			ImGui::SetNextWindowCollapsed(collapsed);
		}
		void set_next_window_collapsed2(bool collapsed, int cond) {
			ImGui::SetNextWindowCollapsed(collapsed, cond);
		}
		void set_next_window_focus() {
			ImGui::SetNextWindowFocus();
		}
		void set_next_window_bg_alpha(float alpha) {
			ImGui::SetNextWindowBgAlpha(alpha);
		}

		// Cursor / Layout
		void begin_group() {
			ImGui::BeginGroup();
		}
		void end_group() {
			ImGui::EndGroup();
		}
		void separator() {
			ImGui::Separator();
		}
		void same_line1() {
			ImGui::SameLine();
		}
		void same_line2(float pos_x) {
			ImGui::SameLine(pos_x);
		}
		void same_line3(float pos_x, float spacing_w) {
			ImGui::SameLine(pos_x, spacing_w);
		}
		void spacing() {
			ImGui::Spacing();
		}
		void dummy(float x, float y) {
			ImGui::Dummy(ImVec2(x, y));
		}
		void indent1() {
			ImGui::Indent();
		}
		void indent2(float w) {
			ImGui::Indent(w);
		}
		void unident1() {
			ImGui::Unindent();
		}
		void unident2(float w) {
			ImGui::Unindent(w);
		}
		ImVec2 get_cursor_pos() {
			return ImGui::GetCursorPos();
		}
		ImVec2 get_cursor_screen_pos() {
			return ImGui::GetCursorScreenPos();
		}

		// Widgets
		void text(std::string str) {
			ImGui::Text(str.c_str());
		}
		void text_colored(std::string str, float x, float y, float z, float w) {
			ImGui::TextColored(ImVec4(x, y, z, w), str.c_str());
		}
		void text_disabled(std::string str) {
			ImGui::TextDisabled(str.c_str());
		}
		void text_wrapped(std::string str) {
			ImGui::TextWrapped(str.c_str());
		}
		void label_text(std::string lable, std::string str) {
			ImGui::LabelText(lable.c_str(), str.c_str());
		}
		void bullet() {
			ImGui::Bullet();
		}
		void bullet_text(std::string str) {
			ImGui::BulletText(str.c_str());
		}
		bool button1(std::string lable) {
			return ImGui::Button(lable.c_str());
		}
		bool button2(std::string lable, float x, float y) {
			return ImGui::Button(lable.c_str(), ImVec2(x, y));
		}
		bool small_button(std::string lable) {
			return ImGui::SmallButton(lable.c_str());
		}
		bool color_button1(std::string desc_id, float x, float y, float z, float w) {
			ImGui::ColorButton(desc_id.c_str(), ImVec4(x, y, z, w));
		}
		bool color_button2(std::string desc_id, float x, float y, float z, float w, int flag) {
			ImGui::ColorButton(desc_id.c_str(), ImVec4(x, y, z, w), flag);
		}
		bool color_button3(std::string desc_id, float x, float y, float z, float w, int flag, float x1, float y1) {
			ImGui::ColorButton(desc_id.c_str(), ImVec4(x, y, z, w), flag, ImVec2(x1, y1));
		}
		bool collapsing_header1(std::string lable) {
			return ImGui::CollapsingHeader(lable.c_str());
		}
		bool collapsing_header2(std::string lable, std::string field_from_config) {
			return ImGui::CollapsingHeader(lable.c_str(), &(GET_BOOL[field_from_config]));
		}
		bool collapsing_header3(std::string lable, std::string field_from_config, int flags) {
			return ImGui::CollapsingHeader(lable.c_str(), &(GET_BOOL[field_from_config]), flags);
		}
		bool checkbox(std::string lable, std::string field_from_config) {
			return ImGui::Checkbox(lable.c_str(), &(GET_BOOL[field_from_config]));
		}
		bool radio_button1(std::string lable, bool active) {
			return ImGui::RadioButton(lable.c_str(), active);
		}
		bool radio_button2(std::string lable, std::string field_from_config, int v_button) {
			return ImGui::RadioButton(lable.c_str(), &(GET_INT[field_from_config]), v_button);
		}
		bool combo1(std::string lable, std::string field_from_config, std::string items) {
			return ImGui::Combo(lable.c_str(), &(GET_INT[field_from_config]), items.c_str());
		}
		bool combo2(std::string lable, std::string field_from_config, std::string items, int _t) {
			return ImGui::Combo(lable.c_str(), &(GET_INT[field_from_config]), items.c_str(), _t);
		}

		// Menus
		bool begin_main_menu_bar() {
			return ImGui::BeginMainMenuBar();
		}
		void end_main_menu_bar() {
			ImGui::EndMainMenuBar();
		}
		bool begin_menu_bar() {
			return ImGui::BeginMenuBar();
		}
		void end_menu_bar() {
			ImGui::EndMenuBar();
		}
		bool begin_menu1(std::string label) {
			return ImGui::BeginMenu(label.c_str());
		}
		bool begin_menu2(std::string label, bool enabled) {
			return ImGui::BeginMenu(label.c_str(), enabled);
		}
		void end_menu() {
			ImGui::EndMenu();
		}
		bool menu_item1(std::string label) {
			return ImGui::MenuItem(label.c_str());
		}
		bool menu_item2(std::string label, std::string shortcut) {
			return ImGui::MenuItem(label.c_str(), shortcut.c_str());
		}
		bool menu_item3(std::string label, std::string shortcut, bool selected) {
			return ImGui::MenuItem(label.c_str(), shortcut.c_str(), selected);
		}
		bool menu_item4(std::string label, std::string shortcut, bool selected, bool enabled) {
			return ImGui::MenuItem(label.c_str(), shortcut.c_str(), selected, enabled);
		}
		bool menu_item5(std::string label, std::string shortcut, std::string field_from_config) {
			return ImGui::MenuItem(label.c_str(), shortcut.c_str(), &(GET_BOOL[field_from_config]));
		}
		bool menu_item6(std::string label, std::string shortcut, std::string field_from_config, bool enabled) {
			return ImGui::MenuItem(label.c_str(), shortcut.c_str(), &(GET_BOOL[field_from_config]), enabled);
		}

		// Widgets: Drags
		// Widgets: Input with Keyboard
		// Widgets: Sliders
		// ID scopes
	};

	void test() {
		g_console.log("RunTestFunc");
		for (auto hk : hooks->getHooks("on_test"))
		{
			try
			{
				auto result = hk.func();
				if (!result.valid()) {
					sol::error err = result;
					g_console.log(err.what());
				}
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
		lua_state["test"] = []() { test(); };
		lua_state.new_usertype<ImVec2>("c_vec2",
			sol::constructors <ImVec2(), ImVec2(float, float)>(),
			"x", &ImVec2::x,
			"y", &ImVec2::y
			);
		lua_state.new_usertype<ImVec4>("c_vec4",
			sol::constructors <ImVec4(), ImVec4(float, float, float, float)>(),
			"x", &ImVec4::x,
			"y", &ImVec4::y,
			"z", &ImVec4::z,
			"w", &ImVec4::w
			);
		auto config = lua_state.create_table();
		M(config, get);
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
		M(config, init);
		M(config, save);
		M(config, load);
		M(config, reset);
		M(config, add);
		M(config, remove);
		M(config, rename);
		M(config, configs);
		N(lua_state, config);

		auto client = lua_state.create_table();
		M(client, set_event_callback);
		M(client, run_script);
		M(client, reload_active_scripts);
		M(client, refresh);
		N(lua_state, client);

		auto imgui = lua_state.create_table();

		// window
		imgui["begin_window"] = sol::overload(
			ns_imgui::begin_window1,
			ns_imgui::begin_window2,
			ns_imgui::begin_window3
		);
		M(imgui, end_window);
		imgui["begin_child"] = sol::overload(
			ns_imgui::begin_child1,
			ns_imgui::begin_child2,
			ns_imgui::begin_child3,
			ns_imgui::begin_child4,
			ns_imgui::begin_child5,
			ns_imgui::begin_child6,
			ns_imgui::begin_child7,
			ns_imgui::begin_child8
		);
		M(imgui, end_child);
		imgui["set_window_pos"] = sol::overload(
			ns_imgui::set_window_pos1,
			ns_imgui::set_window_pos2
		);
		imgui["set_window_size"] = sol::overload(
			ns_imgui::set_window_size1,
			ns_imgui::set_window_size2
		);
		imgui["set_window_focus"] = sol::overload(
			ns_imgui::set_window_focus1,
			ns_imgui::set_window_focus2
		);
		imgui["set_window_collapsed"] = sol::overload(
			ns_imgui::set_window_collapsed1,
			ns_imgui::set_window_collapsed2,
			ns_imgui::set_window_collapsed3
		);

		imgui["set_next_window_pos"] = sol::overload(
			ns_imgui::set_next_window_pos1,
			ns_imgui::set_next_window_pos2,
			ns_imgui::set_next_window_pos3
		);
		M(imgui, set_next_window_content_size);
		imgui["set_next_window_size"] = sol::overload(
			ns_imgui::set_next_window_size1,
			ns_imgui::set_next_window_size2
		);
		M(imgui, set_next_window_size_constraints);
		imgui["set_next_window_collapsed"] = sol::overload(
			ns_imgui::set_next_window_collapsed1,
			ns_imgui::set_next_window_collapsed2
		);
		M(imgui, set_next_window_focus);
		M(imgui, set_next_window_bg_alpha);

		// Cursor / Layout
		M(imgui, begin_group);
		M(imgui, end_group);
		M(imgui, separator);
		imgui["same_line"] = sol::overload(
			ns_imgui::same_line1,
			ns_imgui::same_line2,
			ns_imgui::same_line3
		);
		M(imgui, spacing);
		M(imgui, dummy);
		imgui["indent"] = sol::overload(
			ns_imgui::indent1,
			ns_imgui::indent2
		);
		imgui["unident"] = sol::overload(
			ns_imgui::unident1,
			ns_imgui::unident2
		);
		M(imgui, get_cursor_pos);
		M(imgui, get_cursor_screen_pos);

		// Widgets
		M(imgui, text);
		M(imgui, text_colored);
		M(imgui, text_disabled);
		M(imgui, text_wrapped);
		M(imgui, label_text);
		M(imgui, bullet);
		M(imgui, bullet_text);
		imgui["button"] = sol::overload(
			ns_imgui::button1,
			ns_imgui::button1
		);
		imgui["color_button"] = sol::overload(
			ns_imgui::color_button1,
			ns_imgui::color_button2,
			ns_imgui::color_button3
		);
		M(imgui, small_button);
		imgui["collapsing_header"] = sol::overload(
			ns_imgui::collapsing_header1,
			ns_imgui::collapsing_header2,
			ns_imgui::collapsing_header3
		);
		M(imgui, checkbox);
		imgui["radio_button"] = sol::overload(
			ns_imgui::radio_button1,
			ns_imgui::radio_button2
		);
		imgui["combo"] = sol::overload(
			ns_imgui::combo1,
			ns_imgui::combo2
		);
		// Menus
		M(imgui, begin_main_menu_bar);
		M(imgui, end_main_menu_bar);
		M(imgui, begin_menu_bar);
		M(imgui, end_menu_bar);
		imgui["begin_menu"] = sol::overload(
			ns_imgui::begin_menu1,
			ns_imgui::begin_menu2
		);
		M(imgui, end_menu);
		imgui["menu_item"] = sol::overload(
			ns_imgui::menu_item1,
			ns_imgui::menu_item2,
			ns_imgui::menu_item3,
			ns_imgui::menu_item4,
			ns_imgui::menu_item5,
			ns_imgui::menu_item6
		);
		N(lua_state, imgui);

		auto log = lua_state.create_table();
		M(log, enable_log_file);
		N(lua_state, log);

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
				if (oldScriptsSize < 0)
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

	void c_lua_hookManager::registerHook(std::string eventName, int scriptId, sol::protected_function func) {
		c_lua_hook hk = { scriptId, func };

		this->hooks[eventName].push_back(hk);
	}

	void c_lua_hookManager::unregisterHooks(int scriptId) {
		for (auto& ev : this->hooks) {
			int pos = 0;

			for (auto& hk : ev.second) {
				if (hk.scriptId == scriptId)
					ev.second.erase(ev.second.begin() + pos);

				pos++;
			}
		}
	}

	std::vector<c_lua_hook> c_lua_hookManager::getHooks(std::string eventName) {
		return this->hooks[eventName];
	}

};
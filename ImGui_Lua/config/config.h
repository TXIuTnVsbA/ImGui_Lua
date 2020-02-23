#pragma once
#include <array>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <algorithm>
class c_config {
public:
	std::unordered_map<std::string, bool> b;
	std::unordered_map<std::string, int> i;
	std::unordered_map<std::string, float> f;
	std::unordered_map<std::string, float[4]> c;
    std::unordered_map<std::string, std::string> s;
	std::unordered_map<std::string, std::unordered_map<int, bool>> i_b;
	std::unordered_map<std::string, std::unordered_map<int, int>> i_i;
	std::unordered_map<std::string, std::unordered_map<int, float>> i_f;
    std::unordered_map<std::string, std::unordered_map<int, std::string>> i_s;
    std::unordered_map<std::string, std::unordered_map<std::string, bool>> s_b;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> s_i;
    std::unordered_map<std::string, std::unordered_map<std::string, float>> s_f;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> s_s;

	void init() noexcept;
    void load(size_t) noexcept;
    void save(size_t) const noexcept;
    void add(const char*) noexcept;
    void remove(size_t) noexcept;
    void rename(size_t, const char*) noexcept;
    void default() noexcept;
    void reset() noexcept;

    constexpr auto& getConfigs() noexcept
    {
        return configs;
    }
    struct
    {
        //float w,h
		const char* w{ "gui_w" };
        const char* h{ "gui_h" };

        //tuple_string tab_name
        const char* tab_name{ "gui_tab_name" };

        //map_bool tab_bool
        const char* tab_bool{ "gui_tab_bool" };

        //map_int container_count
        const char* container_count{ "gui_container_count" };

    }gui;

    std::filesystem::path get_path() noexcept;
private:
    std::filesystem::path _path;
    std::filesystem::path path;
    std::vector<std::string> configs;
};

extern c_config g_config;

#define GET_BOOL g_config.b
#define GET_INT g_config.i
#define GET_FLOAT g_config.f
#define GET_COLOR g_config.c
#define GET_STRING g_config.s
#define GET_BOOLS g_config.i_b
#define GET_INTS g_config.i_i
#define GET_FLOATS g_config.i_f
#define GET_STRINGS g_config.i_s
#define GET_BOOLS_MAP g_config.s_b
#define GET_INTS_MAP g_config.s_i
#define GET_FLOATS_MAP g_config.s_f
#define GET_STRINGS_MAP g_config.s_s
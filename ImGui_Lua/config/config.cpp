#include <fstream>
#include <ShlObj.h>
#include "../utils/json/json.h"
#include "config.h"
#include "../utils/console/console.h"
c_config g_config{};

void c_config::init() noexcept {
    if (PWSTR pathToDocuments; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &pathToDocuments))) {
        _path = pathToDocuments;
        _path /= "ImGui";
        path = _path;
        path /= "Config";
        CoTaskMemFree(pathToDocuments);
    }

    if (!std::filesystem::is_directory(_path)) {
        std::filesystem::remove(_path);
        std::filesystem::create_directory(_path);
    }

    if (!std::filesystem::is_directory(path)) {
        std::filesystem::remove(path);
        std::filesystem::create_directory(path);
    }

    std::transform(std::filesystem::directory_iterator{ path },
        std::filesystem::directory_iterator{ },
        std::back_inserter(configs),
        [](const auto& entry) { return std::string{ (const char*)entry.path().filename().u8string().c_str() }; });
}

std::filesystem::path c_config::get_path() noexcept {
    if (!std::filesystem::is_directory(_path)) {
        std::filesystem::remove(_path);
        std::filesystem::create_directory(_path);
    }
    return _path;
}

void c_config::load(size_t id) noexcept {
    Json::Value json;

    if (std::ifstream in{ path / (const char8_t*)configs[id].c_str() }; in.good())
        in >> json;
    else
        return;

    auto& json_bool = json["bool"];
    auto& json_int = json["int"];
    auto& json_float = json["float"];
    auto& json_color = json["color"];
    auto& json_string = json["string"];

    auto& json_tuple_bool = json["tuple_bool"];
    auto& json_tuple_int = json["tuple_int"];
    auto& json_tuple_float = json["tuple_float"];
    auto& json_tuple_string = json["tuple_string"];

    auto& json_map_bool = json["map_bool"];
    auto& json_map_int = json["map_int"];
    auto& json_map_float = json["map_float"];
    auto& json_map_string = json["map_string"];

    if (!json_bool.isNull()) {
        auto json_members = json_bool.getMemberNames();
        for (auto iter = json_members.begin(); iter != json_members.end(); ++iter) {
            std::string member_name = *iter;
            auto member = json_bool[member_name];
            if (!member.isNull()) {
                b[member_name.c_str()] = member.asBool();
            }
        }
    }
    if (!json_int.isNull()) {
        auto json_members = json_int.getMemberNames();
        for (auto iter = json_members.begin(); iter != json_members.end(); ++iter) {
            std::string member_name = *iter;
            auto member = json_int[member_name];
            if (!member.isNull()) {
                i[member_name.c_str()] = member.asInt();
            }
        }
    }
    if (!json_float.isNull()) {
        auto json_members = json_float.getMemberNames();
        for (auto iter = json_members.begin(); iter != json_members.end(); ++iter) {
            std::string member_name = *iter;
            auto member = json_float[member_name];
            if (!member.isNull()) {
                f[member_name.c_str()] = member.asFloat();
            }
        }
    }
    if (!json_color.isNull()) {
        auto json_members = json_color.getMemberNames();
        for (auto iter = json_members.begin(); iter != json_members.end(); ++iter) {
            std::string member_name = *iter;
            auto member = json_color[member_name];
            if (!member.isNull()) {
                c[member_name.c_str()][0] = member[0].asFloat();
                c[member_name.c_str()][1] = member[1].asFloat();
                c[member_name.c_str()][2] = member[2].asFloat();
                c[member_name.c_str()][3] = member[3].asFloat();
            }
        }
    }

    if (!json_string.isNull()) {
        auto json_members = json_string.getMemberNames();
        for (auto iter = json_members.begin(); iter != json_members.end(); ++iter) {
            std::string member_name = *iter;
            auto member = json_string[member_name];
            if (!member.isNull()) {
                s[member_name.c_str()] = member.asString();
            }
        }
    }

    if (!json_tuple_bool.isNull()) {
        auto members_1 = json_tuple_bool.getMemberNames();
        for (auto iter_1 = members_1.begin(); iter_1 != members_1.end(); ++iter_1) {
            std::string member_name_1 = *iter_1;
            auto member_1 = json_tuple_bool[member_name_1];
            if (!member_1.isNull()) {
                auto members_2 = member_1.getMemberNames();
                for (auto iter_2 = members_2.begin(); iter_2 != members_2.end(); ++iter_2) {
                    std::string member_name_2 = *iter_2;
                    auto member_2 = member_1[member_name_2];
                    if (!member_2.isNull()) {
                        int x = atoi(member_name_2.c_str());
                        i_b[member_name_1][x] = member_2.asBool();
                    }
                }
            }
        }
    }
    if (!json_tuple_int.isNull()) {
        auto members_1 = json_tuple_int.getMemberNames();
        for (auto iter_1 = members_1.begin(); iter_1 != members_1.end(); ++iter_1) {
            std::string member_name_1 = *iter_1;
            auto member_1 = json_tuple_int[member_name_1];
            if (!member_1.isNull()) {
                auto members_2 = member_1.getMemberNames();
                for (auto iter_2 = members_2.begin(); iter_2 != members_2.end(); ++iter_2) {
                    std::string member_name_2 = *iter_2;
                    auto member_2 = member_1[member_name_2];
                    if (!member_2.isNull()) {
                        int x = atoi(member_name_2.c_str());
                        i_i[member_name_1][x] = member_2.asInt();
                    }
                }
            }
        }
    }
    if (!json_tuple_float.isNull()) {
        auto members_1 = json_tuple_float.getMemberNames();
        for (auto iter_1 = members_1.begin(); iter_1 != members_1.end(); ++iter_1) {
            std::string member_name_1 = *iter_1;
            auto member_1 = json_tuple_float[member_name_1];
            if (!member_1.isNull()) {
                auto members_2 = member_1.getMemberNames();
                for (auto iter_2 = members_2.begin(); iter_2 != members_2.end(); ++iter_2) {
                    std::string member_name_2 = *iter_2;
                    auto member_2 = member_1[member_name_2];
                    if (!member_2.isNull()) {
                        int x = atoi(member_name_2.c_str());
                        i_f[member_name_1][x] = member_2.asFloat();
                    }
                }
            }
        }
    }
    if (!json_tuple_string.isNull()) {
        auto members_1 = json_tuple_string.getMemberNames();
        for (auto iter_1 = members_1.begin(); iter_1 != members_1.end(); ++iter_1) {
            std::string member_name_1 = *iter_1;
            auto member_1 = json_tuple_string[member_name_1];
            if (!member_1.isNull()) {
                auto members_2 = member_1.getMemberNames();
                for (auto iter_2 = members_2.begin(); iter_2 != members_2.end(); ++iter_2) {
                    std::string member_name_2 = *iter_2;
                    auto member_2 = member_1[member_name_2];
                    if (!member_2.isNull()) {
                        int x = atoi(member_name_2.c_str());
                        i_s[member_name_1][x] = member_2.asString();
                    }
                }
            }
        }
    }

    if (!json_map_bool.isNull()) {
        auto members_1 = json_map_bool.getMemberNames();
        for (auto iter_1 = members_1.begin(); iter_1 != members_1.end(); ++iter_1) {
            std::string member_name_1 = *iter_1;
            auto member_1 = json_map_bool[member_name_1];
            if (!member_1.isNull()) {
                auto members_2 = member_1.getMemberNames();
                for (auto iter_2 = members_2.begin(); iter_2 != members_2.end(); ++iter_2) {
                    std::string member_name_2 = *iter_2;
                    auto member_2 = member_1[member_name_2];
                    if (!member_2.isNull()) {
                        s_b[member_name_1][member_name_2] = member_2.asBool();
                    }
                }
            }
        }
    }
    if (!json_map_int.isNull()) {
        auto members_1 = json_map_int.getMemberNames();
        for (auto iter_1 = members_1.begin(); iter_1 != members_1.end(); ++iter_1) {
            std::string member_name_1 = *iter_1;
            auto member_1 = json_map_int[member_name_1];
            if (!member_1.isNull()) {
                auto members_2 = member_1.getMemberNames();
                for (auto iter_2 = members_2.begin(); iter_2 != members_2.end(); ++iter_2) {
                    std::string member_name_2 = *iter_2;
                    auto member_2 = member_1[member_name_2];
                    if (!member_2.isNull()) {
                        s_i[member_name_1][member_name_2] = member_2.asInt();
                    }
                }
            }
        }
    }
    if (!json_map_float.isNull()) {
        auto members_1 = json_tuple_float.getMemberNames();
        for (auto iter_1 = members_1.begin(); iter_1 != members_1.end(); ++iter_1) {
            std::string member_name_1 = *iter_1;
            auto member_1 = json_map_float[member_name_1];
            if (!member_1.isNull()) {
                auto members_2 = member_1.getMemberNames();
                for (auto iter_2 = members_2.begin(); iter_2 != members_2.end(); ++iter_2) {
                    std::string member_name_2 = *iter_2;
                    auto member_2 = member_1[member_name_2];
                    if (!member_2.isNull()) {
                        s_f[member_name_1][member_name_2] = member_2.asFloat();
                    }
                }
            }
        }
    }
    if (!json_map_string.isNull()) {
        auto members_1 = json_tuple_string.getMemberNames();
        for (auto iter_1 = members_1.begin(); iter_1 != members_1.end(); ++iter_1) {
            std::string member_name_1 = *iter_1;
            auto member_1 = json_map_string[member_name_1];
            if (!member_1.isNull()) {
                auto members_2 = member_1.getMemberNames();
                for (auto iter_2 = members_2.begin(); iter_2 != members_2.end(); ++iter_2) {
                    std::string member_name_2 = *iter_2;
                    auto member_2 = member_1[member_name_2];
                    if (!member_2.isNull()) {
                        s_s[member_name_1][member_name_2] = member_2.asString();
                    }
                }
            }
        }
    }

}

void c_config::save(size_t id) const noexcept {
    Json::Value json;
    auto& json_bool = json["bool"];
    auto& json_int = json["int"];
    auto& json_float = json["float"];
    auto& json_color = json["color"];
    auto& json_string = json["string"];

    auto& json_tuple_bool = json["tuple_bool"];
    auto& json_tuple_int = json["tuple_int"];
    auto& json_tuple_float = json["tuple_float"];
    auto& json_tuple_string = json["tuple_string"];

    auto& json_map_bool = json["map_bool"];
    auto& json_map_int = json["map_int"];
    auto& json_map_float = json["map_float"];
    auto& json_map_string = json["map_string"];

    for (auto kv : b)
    {
        json_bool[kv.first] = kv.second;
    }
    for (auto kv : i)
    {
        json_int[kv.first] = kv.second;
    }
    for (auto kv : f)
    {
        json_float[kv.first] = kv.second;
    }
    for (auto kv : c)
    {
        json_color[kv.first][0] = kv.second[0];
        json_color[kv.first][1] = kv.second[1];
        json_color[kv.first][2] = kv.second[2];
        json_color[kv.first][3] = kv.second[3];
    }
    for (auto kv : s)
    {
        json_string[kv.first] = kv.second;
    }
    
    for (auto kv : i_b)
    {
        Json::Value json_map;
        for (auto v : kv.second)
        {
            //json_map[v.first] = v.second;

            //char buffer[8] = { 0 };
            //sprintf(buffer, "%d", v.first);
            char buffer[8] = { 0 }; _itoa(v.first, buffer, 10);
            json_map[buffer] = v.second;
        }
        json_tuple_bool[kv.first] = json_map;
    }

    for (auto kv : i_i)
    {
        Json::Value json_map;
        for (auto v : kv.second)
        {
            //json_map[v.first] = v.second;

            //char buffer[8] = { 0 };
            //sprintf(buffer, "%d", v.first);
            char buffer[8] = { 0 }; _itoa(v.first, buffer, 10);
            json_map[buffer] = v.second;
        }
        json_tuple_int[kv.first] = json_map;
    }

    for (auto kv : i_f)
    {
        Json::Value json_map;
        for (auto v : kv.second)
        {
            //json_map[v.first] = v.second;

            //char buffer[8] = { 0 };
            //sprintf(buffer, "%d", v.first);
            char buffer[8] = { 0 }; _itoa(v.first, buffer, 10);
            json_map[buffer] = v.second;
        }
        json_tuple_float[kv.first] = json_map;
    }

    for (auto kv : i_s)
    {
        Json::Value json_map;
        for (auto v : kv.second)
        {
            //json_map[v.first] = v.second;

            //char buffer[8] = { 0 };
            //sprintf(buffer, "%d", v.first);
            char buffer[8] = { 0 }; _itoa(v.first, buffer, 10);
            json_map[buffer] = v.second;
        }
        json_tuple_string[kv.first] = json_map;
    }

    for (auto kv : s_b)
    {
        Json::Value json_map;
        for (auto v : kv.second)
        {
            json_map[v.first] = v.second;
        }
        json_map_bool[kv.first] = json_map;
    }

    for (auto kv : s_i)
    {
        Json::Value json_map;
        for (auto v : kv.second)
        {
            json_map[v.first] = v.second;
        }
        json_map_int[kv.first] = json_map;
    }

    for (auto kv : s_f)
    {
        Json::Value json_map;
        for (auto v : kv.second)
        {
            json_map[v.first] = v.second;
        }
        json_map_float[kv.first] = json_map;
    }

    for (auto kv : s_s)
    {
        Json::Value json_map;
        for (auto v : kv.second)
        {
            json_map[v.first] = v.second;
        }
        json_map_string[kv.first] = json_map;
    }

    if (!std::filesystem::is_directory(path)) {
        std::filesystem::remove(path);
        std::filesystem::create_directory(path);
    }

    if (std::ofstream out{ path / (const char8_t*)configs[id].c_str() }; out.good())
        out << json;
}

void c_config::add(const char* name) noexcept
{
    if (*name && std::find(std::cbegin(configs), std::cend(configs), name) == std::cend(configs))
        configs.emplace_back(name);
}

void c_config::remove(size_t id) noexcept
{
    std::filesystem::remove(path / (const char8_t*)configs[id].c_str());
    configs.erase(configs.cbegin() + id);
}

void c_config::rename(size_t item, const char* newName) noexcept
{
    std::filesystem::rename(path / (const char8_t*)configs[item].c_str(), path / (const char8_t*)newName);
    configs[item] = newName;
}

void c_config::default() noexcept {
    //default
  
    GET_STRINGS[gui.tab_name][0] = "Demo";
    GET_BOOLS_MAP[gui.tab_bool]["Demo"] = false;
    GET_FLOATS_MAP[gui.w]["Demo"] = 500.f;
    GET_FLOATS_MAP[gui.h]["Demo"] = 500.f;
    GET_INTS_MAP[gui.container_count]["Demo"] = 1;

    GET_STRINGS[gui.tab_name][1] = "Config";
	GET_BOOLS_MAP[gui.tab_bool]["Config"] = false;
    GET_FLOATS_MAP[gui.w]["Config"] = 500.f;
    GET_FLOATS_MAP[gui.h]["Config"] = 500.f;
    GET_INTS_MAP[gui.container_count]["Config"] = 1;

    GET_STRINGS[gui.tab_name][2] = "Lua";
    GET_BOOLS_MAP[gui.tab_bool]["Lua"] = false;
    GET_FLOATS_MAP[gui.w]["Lua"] = 500.f;
    GET_FLOATS_MAP[gui.h]["Lua"] = 500.f;
    GET_INTS_MAP[gui.container_count]["Lua"] = 1;

    GET_STRINGS[gui.tab_name][3] = "Console";
    GET_BOOLS_MAP[gui.tab_bool]["Console"] = false;
    GET_FLOATS_MAP[gui.w]["Console"] = 500.f;
    GET_FLOATS_MAP[gui.h]["Console"] = 500.f;
    GET_INTS_MAP[gui.container_count]["Console"] = 1;

}

void c_config::reset() noexcept
{
    //reset
}

void c_config::refresh() noexcept
{
    //refresh
}
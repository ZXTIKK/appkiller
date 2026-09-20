//
// Created by Саша on 19.09.2026.
//

#include "../lib/readSettings.h"
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <vector>
#include <sstream>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {
    std::vector<std::string> parseLine(std::string line) {
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line.erase(commentPos);
        }

        std::vector<std::string> tokens;
        std::stringstream ss(line);
        std::string word;

        while (ss >> word) {
            tokens.push_back(word);
        }

        return tokens;
    }
}

TrayUtils::ReadSettings::ReadSettings() {
    try {
        fs::path path;
#if defined(_WIN32)
        const wchar_t* appData = _wgetenv(L"APPDATA");
        if (appData) {
            path = fs::path(appData) / "Zxnt" / "config.json";
        }
#elif defined(__linux__)
        const char* home = std::getenv("HOME");
        if (home) {
            path = fs::path(home) / ".config" / "Zxnt" / "config.json";
        }
#endif

        if (path.empty()) {
            return;
        }

        fs::create_directories(path.parent_path());

        if (!fs::exists(path)) {
            std::ofstream newFile(path);
            if (newFile.is_open()) {
                json defaultConfig = json::array({
            {
                {"name", "Happ"},
                {"path", "C:/Program Files/FlyFrogLLC/Happ/Happ.exe"},
                {"commands_kill", {
                    "taskkill /F /IM Happ.exe",
                    "taskkill /F /IM happd.exe"
                }}
            }
        });
                newFile << defaultConfig.dump(4);
            }
        }

        std::ifstream fileConfig(path);
        if (!fileConfig.is_open()) return;

        json config = json::parse(fileConfig);

        if (!config.empty()) {
            if (config.is_array()) {
                std::vector<std::string> commands{};
                for (auto& item : config) {
                    std::string name = item.value("name", "Unknow");
                    std::string path = item.value("path", "");

                    if (item.contains("commands_kill") && item["commands_kill"].is_array()) {
                        commands = item["commands_kill"].get<std::vector<std::string>>();
                    }
                    this->restartPrograms.push_back(TrayUtils::ProgramsRestart{path, name, commands});
                }
            }
        }
    } catch (const std::exception& e) {}
}

TrayUtils::ReadSettings::~ReadSettings() {}

std::vector<TrayUtils::ProgramsRestart>& TrayUtils::ReadSettings::getPrograms() {
    return this->restartPrograms;
}
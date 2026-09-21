//
// Created by Саша on 19.09.2026.
//

#include "../lib/readSettings.h"
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <sstream>

using json = nlohmann::json;
namespace fs = std::filesystem;

TrayUtils::ReadSettings::ReadSettings() {
    init();
    startUpdateCheckAsync();
}

TrayUtils::ReadSettings::ReadSettings(std::function<void(const ResUpdater&)> onComplete) {
    init();
    startUpdateCheckAsync(onComplete);

}

void TrayUtils::ReadSettings::init() {
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

const std::vector<TrayUtils::ProgramsRestart>& TrayUtils::ReadSettings::getPrograms() const {
    return this->restartPrograms;
}

ResUpdater TrayUtils::ReadSettings::getUpdater() const {
    std::lock_guard<std::mutex> lock(updaterMutex);
    return updaterData;
}

void TrayUtils::ReadSettings::startUpdateCheckAsync(std::function<void(const ResUpdater&)> onComplete) {
    std::thread([this, onComplete]() {
        ResUpdater localResult{"","","","", "",false,false};
        localResult.nowVersion = VER_STRING;

        cpr::Response response = cpr::Get(
            cpr::Url{"https://api.github.com/repos/ZXTIKK/appkiller/releases/latest"},
            cpr::Header{{"User-Agent", "APPKILLER-App"}},
            cpr::Timeout{5000}
        );

        if (response.status_code != 200) {
            localResult.error = "HTTP Error: " + std::to_string(response.status_code);
        }
        else if (response.header["content-type"].find("application/json") == std::string::npos) {
            localResult.error = "Status " + std::to_string(response.status_code) +
                                " & type not supported: " + response.header["content-type"];
        }
        else {
            try {
                json res = json::parse(response.text);
                if (res.contains("tag_name")) {
                    localResult.newVersion = res.value("tag_name", "v0.0.0");

                    std::string cleanNew = localResult.newVersion;
                    if (!cleanNew.empty() && (cleanNew[0] == 'v' || cleanNew[0] == 'V')) {
                        cleanNew = cleanNew.substr(1);
                    }
                    std::cout << checkVersion(cleanNew) << std::endl;
                    std::cout << checkVersion(localResult.nowVersion) << std::endl;
                    if (checkVersion(cleanNew) > checkVersion(localResult.nowVersion)) {
                        localResult.canUpdate = true;
                        if (res.contains("html_url")) {
                            localResult.aboutUrl = res.value("html_url", "https://github.com/ZXTIKK/appkiller/releases");
                        }
                    }
                } else {
                    localResult.error = "Tag 'tag_name' missing in JSON response";
                }
                if (res.contains("assets") && res["assets"].is_array()) {
                    const auto& assetsArray = res["assets"];

                    if (!assetsArray.empty()) {
                        const auto& assets = assetsArray.at(0);

                        if (assets.contains("browser_download_url") && assets["browser_download_url"].is_string()) {
                            localResult.downloadUrl = assets.value("browser_download_url", "");
                        } else {
                            localResult.error = "Tag 'browser_download_url' missing in JSON response";
                        }
                    } else {
                        localResult.error = "Array 'assets' is empty (no release files attached)";
                    }
                }
            } catch (const std::exception& e) {
                localResult.error = std::string("JSON Parse exception: ") + e.what();
            }
        }

        localResult.isDone = true;

        std::lock_guard<std::mutex> lock(updaterMutex);
        updaterData = localResult;
        if (onComplete) {
            onComplete(localResult);
        }
    }).detach();
}

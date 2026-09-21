//
// Created by Саша on 20.09.2026.
//

#ifndef UTILS_UTILS_H
#define UTILS_UTILS_H
#include <windows.h>
#include <shellapi.h>
#include <thread>
#include <iostream>

#include <cpr/cpr.h>
#include "version.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct ResUpdater {
    std::string nowVersion;
    std::string newVersion;
    std::string aboutUrl;
    std::string error;
    std::string downloadUrl;
    bool canUpdate;
    bool isDone = false;
};

inline void preventSleep() {
    EXECUTION_STATE flags = ES_CONTINUOUS |
        ES_DISPLAY_REQUIRED |
        ES_SYSTEM_REQUIRED;

    SetThreadExecutionState(flags);
}

inline void allowSleep() {
    SetThreadExecutionState(ES_CONTINUOUS);
}

inline std::wstring utf8ToWstring(const std::string& str) {
    if (str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

inline bool executeSynchronous(const std::string& cmd) {
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    std::vector<char> cmdBuffer(cmd.begin(), cmd.end());
    cmdBuffer.push_back('\0');

    if (CreateProcessA(
            NULL,
            cmdBuffer.data(),
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW,
            NULL,
            NULL,
            &si,
            &pi))
    {
        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    return false;
}

inline int checkVersion(std::string version) {
    if (version.empty()) return 0;
    std::string cleanV;
    std::vector<char> allowChar{'0','1','2','3','4','5','6','7','8','9'};

    for (char c : version) {
        auto iterator = std::find(allowChar.begin(), allowChar.end(), c);
        if (iterator != allowChar.end()) {
            cleanV.push_back(c);
        }
    }
    return std::stoi(cleanV);
}

namespace fs = std::filesystem;

inline void editConfig() {
    const wchar_t* appData = _wgetenv(L"APPDATA");
    auto path = fs::path(appData) / "Zxnt" / "config.json";
    std::cout << path << std::endl;
    auto res = ShellExecuteW(NULL, L"open", L"notepad.exe", path.c_str(), NULL, SW_SHOWDEFAULT);
    std::cout << res << std::endl;
}

#endif //UTILS_UTILS_H

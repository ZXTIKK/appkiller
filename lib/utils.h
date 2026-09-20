//
// Created by Саша on 20.09.2026.
//

#ifndef UTILS_UTILS_H
#define UTILS_UTILS_H
#include <windows.h>
#include <shellapi.h>

inline void preventSleep(bool preventScreenOff = true) {
    EXECUTION_STATE flags = ES_CONTINUOUS | ES_SYSTEM_REQUIRED;

    if (preventScreenOff) {
        flags |= ES_DISPLAY_REQUIRED;
    }

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
#endif //UTILS_UTILS_H

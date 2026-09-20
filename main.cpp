#include "lib/readSettings.h"
#include "lib/trayRender.h"

#ifdef _WIN32
#include <windows.h>

bool isRunAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
                                 }
    return isAdmin;
}

void relaunchAsAdmin() {
    wchar_t szPath[MAX_PATH];
    if (GetModuleFileNameW(NULL, szPath, MAX_PATH)) {
        SHELLEXECUTEINFOW sei = { sizeof(sei) };
        sei.lpVerb = L"runas";
        sei.lpFile = szPath;
        sei.hwnd = NULL;
        sei.nShow = SW_NORMAL;

        if (ShellExecuteExW(&sei)) {
            exit(0);
        }
    }
}
# elif __linux__
#include <unistd.h>
#include <limits.h>

bool isRunAsAdmin() {
    return geteuid() == 0;
}

void relaunchAsAdmin() {
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (count != -1) {
        path[count] = '\0';
        execlp("pkexec", "pkexec", path, static_cast<char*>(nullptr));
    }
}
#endif

int main() {
    if (!isRunAsAdmin()) {
        relaunchAsAdmin();
    }

    std::unique_ptr<TrayUtils::ReadSettings> settings = std::make_unique<TrayUtils::ReadSettings>();

    TrayUtils::Tray tray(*settings);
    tray.trayRender();
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;

}
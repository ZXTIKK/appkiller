#include "../lib/trayRender.h"
#include <iostream>

#define WM_TRAYICON (WM_USER + 1)
#define ID_AFK_MODE_ON 1010
#define ID_AFK_MODE_OFF 1011
#define ID_TRAY_SHOW 1001
#define ID_TRAY_EXIT 1002
#define ID_CHECK_UPDATES 1003
#define ID_PROGRAM_BASE 1012

#define NO_ACTION 0

#define ABOUT_SEARCH 2000
#define UPDATE_NOW 2001

namespace TrayUtils {

    Tray::Tray(const ReadSettings& settings) : readSettings(settings) {
        ZeroMemory(&m_nid, sizeof(m_nid));
    }

    Tray::~Tray() {
        removeTray();
    }

    LRESULT CALLBACK Tray::WndProcSetup(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        Tray* pThis = nullptr;

        if (uMsg == WM_NCCREATE) {
            CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
            pThis = reinterpret_cast<Tray*>(pCreate->lpCreateParams);
            if (pThis) {
                pThis->m_hwnd = hwnd;
                SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
            }
        } else {
            pThis = reinterpret_cast<Tray*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }

        if (pThis) {
            return pThis->handleMessage(hwnd, uMsg, wParam, lParam);
        }

        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }

    LRESULT Tray::handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONDBLCLK) {
                trayOnClick();
            }
            break;

        case WM_COMMAND: {
            WORD cmdId = LOWORD(wParam);

            if (cmdId == ID_AFK_MODE_ON) {
                std::cout << "AFK ON\n" << std::endl;
                afkMode = !afkMode;
                preventSleep();
            }
            else if (cmdId == ID_AFK_MODE_OFF) {
                std::cout << "AFK OFF\n" << std::endl;
                afkMode = !afkMode;
                allowSleep();
            }
            else if (cmdId == ID_TRAY_EXIT) {
                removeTray();
                PostQuitMessage(0);
            }
            else if (cmdId >= ID_PROGRAM_BASE && cmdId < ID_PROGRAM_BASE + readSettings.getPrograms().size()) {
                size_t index = cmdId - ID_PROGRAM_BASE;
                const auto& program = readSettings.getPrograms().at(index);
                for (const auto& cmd : program.processName) {
                    executeSynchronous(cmd);
                }
                std::string command = "\"" + program.path + "\"";
                WinExec(command.c_str(), SW_HIDE);
            }
            else if (cmdId == ABOUT_SEARCH) {
                std::string url = this->readSettings.getUpdater().aboutUrl;
                ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
            }
            else if (cmdId == UPDATE_NOW) {
                updateNow();
            }
            break;
        }

        case WM_DESTROY:
            removeTray();
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }
        return 0;
    }

    void Tray::trayRender() {
        if (m_hwnd) return;

        HINSTANCE hInstance = GetModuleHandle(NULL);

        const wchar_t CLASS_NAME[] = L"TrayUtilsWindowClass";
        WNDCLASSW wc = {};
        wc.lpfnWndProc = Tray::WndProcSetup;
        wc.hInstance = hInstance;
        wc.lpszClassName = CLASS_NAME;

        RegisterClassW(&wc);

        m_hwnd = CreateWindowExW(
            0, CLASS_NAME, L"TrayUtilsHiddenWindow", 0,
            0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, this
        );

        if (!m_hwnd) {
            std::cerr << "Не удалось создать окно Win32. Ошибка: " << GetLastError() << std::endl;
            return;
        }

        HICON hIcon = (HICON)LoadImageW(
            hInstance,
            MAKEINTRESOURCEW(101),
            IMAGE_ICON,
            GetSystemMetrics(SM_CXSMICON),
            GetSystemMetrics(SM_CYSMICON),
            LR_DEFAULTCOLOR
        );

        if (!hIcon) {
            hIcon = LoadIcon(NULL, IDI_APPLICATION);
        }

        m_nid.cbSize = sizeof(NOTIFYICONDATAW);
        m_nid.hWnd = m_hwnd;
        m_nid.uID = 1;
        m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        m_nid.uCallbackMessage = WM_TRAYICON;
        m_nid.hIcon = hIcon;
        wcscpy_s(m_nid.szTip, L"APPKILLER");

        if (!Shell_NotifyIconW(NIM_ADD, &m_nid)) {
            std::cerr << "Не удалось добавить иконку в трей. Ошибка: " << GetLastError() << std::endl;
        }
    }

    void Tray::trayOnClick() {
        if (!m_hwnd) return;

        POINT pt;
        GetCursorPos(&pt);

        HMENU hMenu = CreatePopupMenu();
        HMENU hUpdateMenu = CreatePopupMenu();

        if (this->readSettings.getUpdater().isDone && this->readSettings.getUpdater().error.empty()) {
            if (this->readSettings.getUpdater().canUpdate) {
                std::wstring curVer = L"Current version: " + utf8ToWstring(this->readSettings.getUpdater().nowVersion.c_str());
                std::wstring newVer = L"New version: " + utf8ToWstring(this->readSettings.getUpdater().newVersion.c_str());

                AppendMenuW(hUpdateMenu, MF_STRING , NO_ACTION, curVer.c_str());
                AppendMenuW(hUpdateMenu, MF_STRING , NO_ACTION, newVer.c_str());
                AppendMenuW(hUpdateMenu, MF_STRING , ABOUT_SEARCH, L"What's new?");
                AppendMenuW(hUpdateMenu, MF_STRING, UPDATE_NOW, L"Update now");

            }else {
                std::wstring info = L"You have the latest version.";
                std::wstring aboutSearch = L"About";

                AppendMenuW(hUpdateMenu, MF_STRING , NO_ACTION, info.c_str());
                AppendMenuW(hUpdateMenu, MF_STRING , ABOUT_SEARCH, aboutSearch.c_str());
            }
        }else if (!this->readSettings.getUpdater().error.empty()) {
            std::wstring error = L"ERROR: " + utf8ToWstring(this->readSettings.getUpdater().error.c_str());
            AppendMenuW(hUpdateMenu, MF_STRING , NO_ACTION, error.c_str());
        }else{
            AppendMenuW(hUpdateMenu, MF_STRING, NO_ACTION, L"We are loading the data...");
            AppendMenuW(hUpdateMenu, MF_STRING, NO_ACTION, L"Please check back later.");
        }

        if (!afkMode) {
            AppendMenuW(hMenu, MF_STRING, ID_AFK_MODE_ON, L"AFK mode on");
        }else {
            AppendMenuW(hMenu, MF_STRING, ID_AFK_MODE_OFF, L"AFK mode off");
        }
        AppendMenuW(hMenu, MF_SEPARATOR, NO_ACTION, NULL);
        for (int i = 0; i < this->readSettings.getPrograms().size(); i++) {
            AppendMenuW(hMenu, MF_STRING, ID_PROGRAM_BASE+i, utf8ToWstring(this->readSettings.getPrograms().at(i).name.c_str()).c_str());
        }
        AppendMenuW(hMenu, MF_SEPARATOR, NO_ACTION, NULL);
        AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Выйти");
        AppendMenuW(hMenu, MF_SEPARATOR, NO_ACTION, NULL);
        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hUpdateMenu, L"Update");
        AppendMenuW(hMenu, MF_SEPARATOR, NO_ACTION, NULL);



        SetForegroundWindow(m_hwnd);
        TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, m_hwnd, NULL);
        DestroyMenu(hMenu);
    }

    void Tray::removeTray() {
        if (m_nid.hWnd) {
            Shell_NotifyIconW(NIM_DELETE, &m_nid);
            m_nid.hWnd = NULL;
        }
        if (m_hwnd) {
            DestroyWindow(m_hwnd);
            m_hwnd = NULL;
        }
    }


    void Tray::updateNow() {
        auto updater = this->readSettings.getUpdater();
        bool canRun = updater.isDone && updater.canUpdate && !updater.downloadUrl.empty();

        if (!canRun) {
            std::cout << "Error: " << updater.error << std::endl;
            return;
        }

        auto downloadUrl = updater.downloadUrl;

        std::cout << "Process Update..." << std::endl;
        std::thread([downloadUrl]() {
            namespace fs = std::filesystem;
            try {
                const wchar_t* appData = _wgetenv(L"APPDATA");
                fs::path pathOutput;
                if (appData) {
                    pathOutput = fs::path(appData) / "Zxnt" / "updateBin" ;
                }
                if (!fs::exists(pathOutput)) {
                    fs::create_directories(pathOutput);
                }
                fs::path fileName = "Update.exe";
                auto fullPath = pathOutput / fileName;
                std::ofstream outFile(fullPath, std::ios::trunc | std::ios::binary);
                if (!outFile.is_open()) {
                    std::cerr << "Unable to open file: " << fullPath << std::endl;
                    return;
                }
                std::cout << "Download..." << std::endl;
                cpr::Response r = cpr::Get(
                    cpr::Url(downloadUrl),
                    cpr::Header{{"User-Agent", "MyAppUpdater/1.0"}},
                    cpr::Redirect{true},
                    cpr::WriteCallback{[&outFile](std::string_view data, intptr_t userdata) -> bool {
                        outFile.write(data.data(), data.size());
                        return true;
                    }}
                );
                outFile.close();

                if (r.status_code != 200) {
                    std::cerr << "Ошибка скачивания. Код ответа HTTP: " << r.status_code << std::endl;
                    std::cerr << "CPR Error: " << r.error.message << std::endl;
                    fs::remove(fullPath);
                    return;
                }

                std::cout << "Файл успешно сохранен: " << fullPath << std::endl;

                std::wstring wFullPath = fullPath.wstring();

                INT_PTR res = (INT_PTR)ShellExecuteW(
                    NULL,
                    L"open",
                    fullPath.c_str(),
                    L"/S",
                    NULL,
                    SW_SHOWNORMAL
                );

                if (res > 32) {
                    ExitProcess(0);
                }

            }catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
        }).detach();

    }
}

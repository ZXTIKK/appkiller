#include "../lib/trayRender.h"
#include <iostream>

#define WM_TRAYICON (WM_USER + 1)
#define ID_AFK_MODE_ON 1010
#define ID_AFK_MODE_OFF 1011
#define ID_TRAY_SHOW 1001
#define ID_TRAY_EXIT 1002
#define ID_PROGRAM_BASE 1012

namespace TrayUtils {

    Tray::Tray(const ReadSettings& settings) : readSettings(settings) {
        ZeroMemory(&m_nid, sizeof(m_nid));
    }

    Tray::Tray() : readSettings(ReadSettings()) {
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
                preventSleep();
            }
            else if (cmdId == ID_AFK_MODE_OFF) {
                std::cout << "AFK OFF\n" << std::endl;
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
        AppendMenuW(hMenu, MF_STRING, ID_AFK_MODE_ON, L"AFK MODE");
        AppendMenuW(hMenu, MF_STRING, ID_AFK_MODE_OFF, L"Exit AFK MODE");
        AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
        for (int i = 0; i < this->readSettings.getPrograms().size(); i++) {
            AppendMenuW(hMenu, MF_STRING, ID_PROGRAM_BASE+i, utf8ToWstring(this->readSettings.getPrograms().at(i).name.c_str()).c_str());
        }
        AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Выйти");

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
}

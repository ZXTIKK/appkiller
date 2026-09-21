#ifndef UTILS_TRAYRENDER_H
#define UTILS_TRAYRENDER_H

#include "readSettings.h"
#include "utils.h"
#include <windows.h>
#include <shellapi.h>

namespace TrayUtils {
    class Tray {
    private:
        const ReadSettings& readSettings;
        HWND m_hwnd{nullptr};
        NOTIFYICONDATAW m_nid{};

        static LRESULT CALLBACK WndProcSetup(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        LRESULT handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        bool afkMode = false;

        void updateNow();

    public:
        explicit Tray(const ReadSettings& settings);
        Tray();
        ~Tray();
        void trayRender();
        void trayOnClick();
        void removeTray();
    };
}

#endif // UTILS_TRAYRENDER_H
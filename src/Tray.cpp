#include "Tray.hpp"

#include <shellapi.h>

#include "Constants.hpp"
#include "Startup.hpp"

namespace gemini {

namespace {

NOTIFYICONDATAW MakeBaseData(HWND hwnd) {
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1; // un seul icone pour cette app
    return nid;
}

} // namespace

Tray::~Tray() {
    Remove();
}

void Tray::Add(HWND hwnd, HICON icon) {
    hwnd_ = hwnd;

    NOTIFYICONDATAW nid = MakeBaseData(hwnd);
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAY_CALLBACK;
    nid.hIcon = icon;
    wcsncpy_s(nid.szTip, kWindowTitle, _TRUNCATE);

    if (Shell_NotifyIconW(NIM_ADD, &nid)) {
        added_ = true;
        // Adopte le comportement moderne des messages de version 4.
        nid.uVersion = NOTIFYICON_VERSION_4;
        Shell_NotifyIconW(NIM_SETVERSION, &nid);
    }
}

void Tray::Remove() {
    if (!added_) {
        return;
    }
    NOTIFYICONDATAW nid = MakeBaseData(hwnd_);
    Shell_NotifyIconW(NIM_DELETE, &nid);
    added_ = false;
}

void Tray::ShowBalloon(const wchar_t* title, const wchar_t* text) {
    if (!added_) {
        return;
    }
    NOTIFYICONDATAW nid = MakeBaseData(hwnd_);
    nid.uFlags = NIF_INFO;
    nid.dwInfoFlags = NIIF_INFO;
    wcsncpy_s(nid.szInfoTitle, title, _TRUNCATE);
    wcsncpy_s(nid.szInfo, text, _TRUNCATE);
    Shell_NotifyIconW(NIM_MODIFY, &nid);
}

bool Tray::HandleMessage(WPARAM /*wparam*/, LPARAM lparam, UINT* command) {
    // Avec NOTIFYICON_VERSION_4, l'evenement est dans le LOWORD du lParam.
    const UINT event = LOWORD(lparam);

    switch (event) {
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
        // Clic gauche = basculer l'affichage de la fenetre.
        *command = ID_TRAY_SHOW;
        return true;

    case WM_RBUTTONUP:
    case WM_CONTEXTMENU: {
        // Menu contextuel : Afficher / Quitter.
        HMENU menu = CreatePopupMenu();
        AppendMenuW(menu, MF_STRING, ID_TRAY_SHOW, L"Afficher");
        AppendMenuW(menu,
                    MF_STRING | (IsStartupEnabled() ? MF_CHECKED : MF_UNCHECKED),
                    ID_TRAY_STARTUP, L"Demarrer avec Windows");
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(menu, MF_STRING, ID_TRAY_QUIT, L"Quitter");

        POINT pt;
        GetCursorPos(&pt);
        // Correctif Microsoft pour les menus de tray : la fenetre doit etre au
        // premier plan avant TrackPopupMenu, et un WM_NULL doit etre poste juste
        // apres, sinon (fenetre masquee dans le tray) le clic sur un item est
        // ignore et la commande n'est jamais renvoyee.
        SetForegroundWindow(hwnd_);
        const UINT chosen = TrackPopupMenu(
            menu, TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
            pt.x, pt.y, 0, hwnd_, nullptr);
        PostMessageW(hwnd_, WM_NULL, 0, 0);
        DestroyMenu(menu);

        if (chosen == ID_TRAY_SHOW || chosen == ID_TRAY_QUIT ||
            chosen == ID_TRAY_STARTUP) {
            *command = chosen;
            return true;
        }
        return false;
    }

    default:
        return false;
    }
}

} // namespace gemini

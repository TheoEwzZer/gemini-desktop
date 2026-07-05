#pragma once

#include <windows.h>

namespace gemini {

// Gere l'icone de la zone de notification (system tray).
// Le proprietaire (App) route WM_TRAY_CALLBACK vers HandleMessage.
class Tray {
public:
    Tray() = default;
    ~Tray();

    Tray(const Tray&) = delete;
    Tray& operator=(const Tray&) = delete;

    // Ajoute l'icone au tray. hwnd recoit les callbacks, icon est l'icone affichee.
    void Add(HWND hwnd, HICON icon);

    // Retire l'icone (appele a la fermeture definitive).
    void Remove();

    // Affiche une balloon tip (utilisee au premier passage dans le tray).
    void ShowBalloon(const wchar_t* title, const wchar_t* text);

    // Traite le message de callback ; retourne true si un item de menu a ete
    // choisi, en renseignant command (ID_TRAY_SHOW / ID_TRAY_QUIT).
    bool HandleMessage(WPARAM wparam, LPARAM lparam, UINT* command);

private:
    HWND hwnd_ = nullptr;
    bool added_ = false;
};

} // namespace gemini

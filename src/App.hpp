#pragma once

#include <windows.h>
#include <wil/com.h>
#include <wrl.h>

#include "Tray.hpp"

// Declarations avancees du SDK WebView2.
interface ICoreWebView2;
interface ICoreWebView2Controller;

namespace gemini {

// Fenetre hote Win32 + WebView2 affichant Gemini, avec reduction dans le tray.
class App {
public:
    // Cree la fenetre et lance l'initialisation asynchrone de WebView2.
    // Si start_hidden est true (lancement au demarrage), l'app demarre reduite
    // dans le tray au lieu d'afficher la fenetre.
    // Retourne false en cas d'echec de creation de la fenetre.
    bool Create(HINSTANCE instance, int show_command, bool start_hidden);

    HWND Hwnd() const { return hwnd_; }

private:
    static LRESULT CALLBACK WndProcThunk(HWND, UINT, WPARAM, LPARAM);
    LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam);

    void InitializeWebView();
    void OnControllerReady(ICoreWebView2Controller* controller);
    void ResizeWebView();
    void ShowWindowRestored();
    void HideToTray();
    void ToggleVisibility();

    HINSTANCE instance_ = nullptr;
    HWND hwnd_ = nullptr;
    HICON icon_ = nullptr;

    wil::com_ptr<ICoreWebView2Controller> controller_;
    wil::com_ptr<ICoreWebView2> webview_;

    Tray tray_;
    bool tray_notice_shown_ = false; // balloon "dans le tray" deja affichee ?
};

} // namespace gemini

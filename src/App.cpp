#include "App.hpp"

#include <WebView2.h>
#include <wrl.h>

#include <string>

#include "Constants.hpp"
#include "Paths.hpp"
#include "SingleInstance.hpp"
#include "Startup.hpp"

using Microsoft::WRL::Callback;

namespace gemini {

namespace {

// Lit/ecrit un flag DWORD sous HKCU\Software\GeminiDesktop.
bool ReadFlag(const wchar_t* name) {
    DWORD value = 0;
    DWORD size = sizeof(value);
    return RegGetValueW(HKEY_CURRENT_USER, kRegistryKey, name, RRF_RT_REG_DWORD,
                        nullptr, &value, &size) == ERROR_SUCCESS &&
           value != 0;
}

void WriteFlag(const wchar_t* name) {
    HKEY key = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kRegistryKey, 0, nullptr, 0,
                        KEY_SET_VALUE, nullptr, &key, nullptr) == ERROR_SUCCESS) {
        DWORD value = 1;
        RegSetValueExW(key, name, 0, REG_DWORD,
                       reinterpret_cast<const BYTE*>(&value), sizeof(value));
        RegCloseKey(key);
    }
}

constexpr wchar_t kTrayNoticeFlag[] = L"TrayNoticeShown";

} // namespace

bool App::Create(HINSTANCE instance, int show_command, bool start_hidden) {
    instance_ = instance;
    icon_ = LoadIconW(instance, MAKEINTRESOURCEW(IDI_APPICON));
    if (!icon_) {
        icon_ = LoadIconW(nullptr, IDI_APPLICATION);
    }

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &App::WndProcThunk;
    wc.hInstance = instance;
    wc.hIcon = icon_;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kWindowClassName;
    wc.hIconSm = icon_;
    RegisterClassExW(&wc);

    hwnd_ = CreateWindowExW(
        0, kWindowClassName, kWindowTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1200, 820,
        nullptr, nullptr, instance, this);
    if (!hwnd_) {
        return false;
    }

    tray_notice_shown_ = ReadFlag(kTrayNoticeFlag);

    // L'icone du tray reste presente pendant toute la vie de l'app.
    tray_.Add(hwnd_, icon_);

    if (start_hidden) {
        // Lancement au demarrage de Windows : on reste dans le tray, sans
        // afficher la fenetre ni la balloon de notice.
        tray_notice_shown_ = true;
    } else {
        ShowWindow(hwnd_, show_command);
        UpdateWindow(hwnd_);
    }

    InitializeWebView();
    return true;
}

LRESULT CALLBACK App::WndProcThunk(HWND hwnd, UINT msg, WPARAM wparam,
                                   LPARAM lparam) {
    App* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lparam);
        self = static_cast<App*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                          reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<App*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self) {
        return self->WndProc(msg, wparam, lparam);
    }
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

LRESULT App::WndProc(UINT msg, WPARAM wparam, LPARAM lparam) {
    // Message d'instance unique : une seconde instance nous demande de nous
    // afficher au premier plan.
    if (msg == SingleInstance::ShowMessage()) {
        ShowWindowRestored();
        return 0;
    }

    switch (msg) {
    case WM_SIZE:
        ResizeWebView();
        return 0;

    case WM_GETMINMAXINFO: {
        auto* mmi = reinterpret_cast<MINMAXINFO*>(lparam);
        mmi->ptMinTrackSize.x = 640;
        mmi->ptMinTrackSize.y = 480;
        return 0;
    }

    case WM_TRAY_CALLBACK: {
        UINT command = 0;
        if (tray_.HandleMessage(wparam, lparam, &command)) {
            if (command == ID_TRAY_SHOW) {
                ToggleVisibility();
            } else if (command == ID_TRAY_STARTUP) {
                // Bascule le lancement au demarrage de Windows.
                SetStartupEnabled(!IsStartupEnabled());
            } else if (command == ID_TRAY_QUIT) {
                DestroyWindow(hwnd_);
            }
        }
        return 0;
    }

    case WM_CLOSE:
        // Fermer = reduire dans le tray plutot que quitter.
        HideToTray();
        return 0;

    case WM_DESTROY:
        tray_.Remove();
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcW(hwnd_, msg, wparam, lparam);
    }
}

void App::InitializeWebView() {
    const std::wstring userData = GetUserDataFolder();

    // Cree l'environnement WebView2 avec un dossier de donnees persistant.
    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
        nullptr, userData.c_str(), nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                if (FAILED(result) || !env) {
                    return result;
                }
                env->CreateCoreWebView2Controller(
                    hwnd_,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [this](HRESULT r, ICoreWebView2Controller* controller)
                            -> HRESULT {
                            if (SUCCEEDED(r) && controller) {
                                OnControllerReady(controller);
                            }
                            return S_OK;
                        })
                        .Get());
                return S_OK;
            })
            .Get());

    if (FAILED(hr)) {
        MessageBoxW(hwnd_,
                    L"Impossible d'initialiser WebView2.\n"
                    L"Verifiez que le runtime WebView2 est installe.",
                    kWindowTitle, MB_OK | MB_ICONERROR);
    }
}

void App::OnControllerReady(ICoreWebView2Controller* controller) {
    controller_ = controller;
    controller_->get_CoreWebView2(&webview_);

    ResizeWebView();

    if (webview_) {
        webview_->Navigate(kHomeUrl);
    }
}

void App::ResizeWebView() {
    if (!controller_) {
        return;
    }
    RECT bounds;
    GetClientRect(hwnd_, &bounds);
    controller_->put_Bounds(bounds);
}

void App::ShowWindowRestored() {
    if (IsIconic(hwnd_) || !IsWindowVisible(hwnd_)) {
        ShowWindow(hwnd_, SW_RESTORE);
    }
    SetForegroundWindow(hwnd_);
}

void App::HideToTray() {
    ShowWindow(hwnd_, SW_HIDE);

    if (!tray_notice_shown_) {
        tray_.ShowBalloon(
            kWindowTitle,
            L"Gemini continue de tourner dans la barre systeme. "
            L"Cliquez sur l'icone pour la rouvrir.");
        tray_notice_shown_ = true;
        WriteFlag(kTrayNoticeFlag);
    }
}

void App::ToggleVisibility() {
    if (IsWindowVisible(hwnd_)) {
        HideToTray();
    } else {
        ShowWindow(hwnd_, SW_SHOW);
        ShowWindowRestored();
    }
}

} // namespace gemini

#include <windows.h>
#include <objbase.h>
#include <shellapi.h>
#include <shlobj.h>
#include <WebView2.h>
#include <wil/com.h>

#include "App.hpp"
#include "Constants.hpp"
#include "SingleInstance.hpp"

using namespace gemini;

namespace {

// Verifie que le runtime WebView2 est disponible. Si absent, affiche un
// message et ouvre la page de telechargement Microsoft.
bool EnsureWebView2Runtime() {
    wil::unique_cotaskmem_string version;
    HRESULT hr = GetAvailableCoreWebView2BrowserVersionString(nullptr, &version);
    if (SUCCEEDED(hr) && version) {
        return true;
    }

    const int choice = MessageBoxW(
        nullptr,
        L"Le runtime Microsoft WebView2 est requis mais introuvable.\n\n"
        L"Voulez-vous ouvrir la page de telechargement maintenant ?",
        kWindowTitle, MB_YESNO | MB_ICONWARNING);

    if (choice == IDYES) {
        ShellExecuteW(nullptr, L"open", kWebView2DownloadUrl, nullptr, nullptr,
                      SW_SHOWNORMAL);
    }
    return false;
}

} // namespace

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int show_command) {
    // 0. AppUserModelID explicite, avant toute creation de fenetre : sans lui,
    //    Windows derive un identifiant instable et la fenetre lancee ne se
    //    regroupe pas sous le raccourci epingle (icone en double dans la barre).
    SetCurrentProcessExplicitAppUserModelID(kAppUserModelId);

    // 1. Instance unique : si deja lancee, activer l'existante et sortir.
    SingleInstance guard;
    if (guard.AlreadyRunning()) {
        guard.ActivateExisting();
        return 0;
    }

    // COM en appartement mono-thread : requis pour les callbacks WebView2.
    HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(com)) {
        return 1;
    }

    // 2. Verifier le runtime WebView2.
    if (!EnsureWebView2Runtime()) {
        CoUninitialize();
        return 0;
    }

    // 3. Creer la fenetre + WebView2.
    //    --startup (lancement au demarrage de Windows) => demarrer dans le tray.
    const bool start_hidden = (wcsstr(GetCommandLineW(), L"--startup") != nullptr);

    App app;
    if (!app.Create(instance, show_command, start_hidden)) {
        CoUninitialize();
        return 1;
    }

    // 4. Boucle de messages.
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    CoUninitialize();
    return static_cast<int>(msg.wParam);
}

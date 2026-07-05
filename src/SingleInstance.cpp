#include "SingleInstance.hpp"

#include "Constants.hpp"

namespace gemini {

UINT SingleInstance::ShowMessage() {
    // La meme chaine produit le meme ID dans toutes les instances/process.
    static const UINT msg = RegisterWindowMessageW(kShowMessageName);
    return msg;
}

SingleInstance::SingleInstance() {
    // Un mutex nomme survit tant qu'au moins un handle est ouvert.
    mutex_.reset(CreateMutexW(nullptr, TRUE, kMutexName));
    already_running_ = (GetLastError() == ERROR_ALREADY_EXISTS);
}

void SingleInstance::ActivateExisting() const {
    // Autorise l'autre process a voler le focus (sinon Windows bloque et
    // l'icone ne fait que clignoter dans la barre des taches).
    AllowSetForegroundWindow(ASFW_ANY);

    // Chemin principal : diffuser le message enregistre. L'instance vivante
    // le recoit dans sa WndProc et se restaure elle-meme.
    PostMessageW(HWND_BROADCAST, ShowMessage(), 0, 0);

    // Fallback direct : retrouver la fenetre par sa classe et la restaurer.
    if (HWND existing = FindWindowW(kWindowClassName, nullptr)) {
        if (IsIconic(existing) || !IsWindowVisible(existing)) {
            ShowWindow(existing, SW_RESTORE);
        }
        SetForegroundWindow(existing);
    }
}

} // namespace gemini

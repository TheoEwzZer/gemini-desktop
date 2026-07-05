#pragma once

#include <windows.h>
#include <wil/resource.h>

namespace gemini {

// Garde-fou d'instance unique base sur un mutex nomme.
//
// Utilisation :
//   SingleInstance guard;
//   if (guard.AlreadyRunning()) {
//       guard.ActivateExisting();
//       return 0; // laisser l'instance vivante au premier plan
//   }
//
// Le message enregistre (RegisterWindowMessage) permet a l'instance vivante
// de se restaurer/mettre au premier plan quand une seconde tente de demarrer.
class SingleInstance {
public:
    SingleInstance();

    // true si une autre instance detient deja le mutex.
    bool AlreadyRunning() const { return already_running_; }

    // Demande a l'instance vivante de se montrer au premier plan.
    void ActivateExisting() const;

    // Message enregistre a intercepter dans la WndProc pour se restaurer.
    static UINT ShowMessage();

private:
    wil::unique_mutex_nothrow mutex_;
    bool already_running_ = false;
};

} // namespace gemini

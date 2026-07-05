#pragma once

namespace gemini {

// Gestion du lancement au demarrage de Windows via
// HKCU\Software\Microsoft\Windows\CurrentVersion\Run (per-user, sans admin).

// true si l'app est configuree pour demarrer avec Windows.
bool IsStartupEnabled();

// Active (ecrit la valeur Run avec le flag --startup) ou desactive (supprime).
void SetStartupEnabled(bool enabled);

} // namespace gemini

; ============================================================
;  Gemini Desktop - script d'installation Inno Setup
;  Produit installer\Output\GeminiDesktop-Setup.exe
;  La version peut etre surchargee par la CI : ISCC /DMyAppVersion=1.2.3
; ============================================================

#ifndef MyAppVersion
  #define MyAppVersion "1.0.0"
#endif

#define MyAppName "Gemini Desktop"
#define MyAppPublisher "TheoEwzZer"
#define MyAppURL "https://github.com/TheoEwzZer/gemini-desktop"
#define MyAppExeName "GeminiDesktop.exe"

[Setup]
AppId={{A7E3F1C2-9B4D-4E8A-9C5F-1D2E3F4A5B6C}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}/releases
DefaultDirName={autopf}\Gemini Desktop
DefaultGroupName=Gemini Desktop
DisableProgramGroupPage=yes
LicenseFile=..\LICENSE
OutputDir=Output
OutputBaseFilename=GeminiDesktop-Setup
SetupIconFile=..\res\icon.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
; Application 64 bits uniquement.
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
; Installation per-user (sans elevation UAC) : coherent avec une app legere
; dont les donnees vivent dans %LOCALAPPDATA%, et evite que la valeur Run de
; "demarrage avec Windows" (HKCU) atterrisse dans le mauvais profil.
PrivilegesRequired=lowest

[Languages]
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[CustomMessages]
french.StartupTask=Lancer Gemini au demarrage de Windows
english.StartupTask=Launch Gemini at Windows startup
french.OptionsGroup=Options :
english.OptionsGroup=Options:

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "startup"; Description: "{cm:StartupTask}"; GroupDescription: "{cm:OptionsGroup}"; Flags: unchecked

[Files]
Source: "..\build\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion

[Registry]
; Lancement au demarrage : meme valeur que le toggle du tray de l'app
; (le flag --startup fait demarrer l'app reduite dans le tray).
; uninsdeletevalue : la valeur est supprimee a la desinstallation.
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; \
    ValueType: string; ValueName: "GeminiDesktop"; \
    ValueData: """{app}\{#MyAppExeName}"" --startup"; \
    Flags: uninsdeletevalue; Tasks: startup

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; AppUserModelID: "TheoEwzZer.GeminiDesktop"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon; AppUserModelID: "TheoEwzZer.GeminiDesktop"

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Code]
// --- Detection du runtime WebView2 (Evergreen) ---
function IsWebView2Installed(): Boolean;
var
  Pv: string;
begin
  Result := False;
  // Cle machine 64 bits.
  if RegQueryStringValue(HKLM, 'SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}', 'pv', Pv) then
    if (Pv <> '') and (Pv <> '0.0.0.0') then
      Result := True;
  // Cle utilisateur (installation per-user du runtime).
  if not Result then
    if RegQueryStringValue(HKCU, 'SOFTWARE\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}', 'pv', Pv) then
      if (Pv <> '') and (Pv <> '0.0.0.0') then
        Result := True;
end;

// Telecharge et installe le runtime WebView2 s'il est absent.
function PrepareToInstall(var NeedsRestart: Boolean): String;
var
  BootstrapperPath: string;
  ResultCode: Integer;
begin
  Result := '';
  if IsWebView2Installed() then
    Exit;

  BootstrapperPath := ExpandConstant('{tmp}\MicrosoftEdgeWebview2Setup.exe');
  try
    // Bootstrapper Evergreen officiel Microsoft.
    DownloadTemporaryFile(
      'https://go.microsoft.com/fwlink/p/?LinkId=2124703',
      'MicrosoftEdgeWebview2Setup.exe', '', nil);
  except
    Result := 'Le runtime WebView2 est requis mais son telechargement a echoue.' + #13#10 +
              'Installez-le manuellement depuis https://developer.microsoft.com/microsoft-edge/webview2/ puis relancez l''installation.';
    Exit;
  end;

  if Exec(BootstrapperPath, '/silent /install', '', SW_SHOW, ewWaitUntilTerminated, ResultCode) then
  begin
    if ResultCode <> 0 then
      Result := 'L''installation du runtime WebView2 a echoue (code ' + IntToStr(ResultCode) + ').';
  end
  else
    Result := 'Impossible de lancer l''installation du runtime WebView2.';
end;

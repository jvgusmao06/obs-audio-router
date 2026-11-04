; Script generated for OBS Audio Router Plugin Installer
#ifndef AppVersion
  #define AppVersion "1.0.0"
#endif

#ifndef SourceDir
  #define SourceDir "..\build\install"
#endif

#ifndef OutputDir
  #define OutputDir "..\dist"
#endif

[Setup]
AppName=OBS Audio Router
AppVersion={#AppVersion}
AppPublisher=Antigravity Community
AppPublisherURL=https://github.com/jvgusmao06/obs-audio-router-plugin
AppSupportURL=https://github.com/jvgusmao06/obs-audio-router-plugin/issues
DefaultDirName={code:GetOBSDir}
DirExistsWarning=no
DisableDirPage=no
OutputBaseFilename=OBS-Audio-Router-Setup-v{#AppVersion}
OutputDir={#OutputDir}
Compression=lzma2/ultra64
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
PrivilegesRequired=admin
CloseApplications=force
RestartApplications=no
UninstallDisplayName=OBS Audio Router

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "pt_br"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"

[Code]
var
  OBSPath: String;

function GetOBSDir(Param: String): String;
begin
  if OBSPath = '' then
  begin
    // Check 64-bit registry first
    if not RegQueryStringValue(HKLM64, 'SOFTWARE\OBS Studio', '', OBSPath) then
    begin
      if not RegQueryStringValue(HKLM, 'SOFTWARE\OBS Studio', '', OBSPath) then
      begin
        // Default standard path
        OBSPath := ExpandConstant('{pf}\obs-studio');
      end;
    end;
  end;
  Result := OBSPath;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  ResultCode: Integer;
begin
  if CurStep = ssPostInstall then
  begin
    // Start bridge process in background
    Exec(ExpandConstant('{localappdata}\OBSAudioRouter\obs-audio-router-bridge.exe'), '', '', SW_HIDE, ewNoWait, ResultCode);
  end;
end;

[Files]
; OBS Plugin DLL
Source: "{#SourceDir}\obs-plugins\64bit\obs-audio-router.dll"; DestDir: "{app}\obs-plugins\64bit"; Flags: ignoreversion restartreplace
; Localization files
Source: "{#SourceDir}\data\obs-plugins\obs-audio-router\locale\*"; DestDir: "{app}\data\obs-plugins\obs-audio-router\locale"; Flags: ignoreversion recursesubdirs createallsubdirs
; Lightweight Audio Passthrough Bridge
Source: "{#SourceDir}\bin\obs-audio-router-bridge.exe"; DestDir: "{localappdata}\OBSAudioRouter"; Flags: ignoreversion restartreplace
; Virtual Audio Device Driver (VB-Cable WHQL)
Source: "{#SourceDir}\driver\*"; DestDir: "{localappdata}\OBSAudioRouter\driver"; Flags: ignoreversion recursesubdirs createallsubdirs

[Registry]
; Start bridge on Windows startup
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "OBSAudioRouterBridge"; ValueData: """{localappdata}\OBSAudioRouter\obs-audio-router-bridge.exe"""; Flags: uninsdeletevalue

[Run]
; Install Virtual Audio Device Driver silently
Filename: "{localappdata}\OBSAudioRouter\driver\VBCABLE_Setup_x64.exe"; Parameters: "-i -h"; StatusMsg: "Instalando dispositivo de áudio virtual..."; Flags: runhidden waituntilterminated
; Start bridge process immediately
Filename: "{localappdata}\OBSAudioRouter\obs-audio-router-bridge.exe"; Flags: nowait
; Option to launch OBS Studio
Filename: "{app}\bin\64bit\obs64.exe"; Description: "Iniciar o OBS Studio agora"; Flags: nowait postinstall skipifsilent

[UninstallRun]
; Stop bridge process gracefully or forcefully before uninstall
Filename: "{cmd}"; Parameters: "/C taskkill /F /IM obs-audio-router-bridge.exe"; Flags: runhidden
; Uninstall Virtual Audio Device Driver silently
Filename: "{localappdata}\OBSAudioRouter\driver\VBCABLE_Setup_x64.exe"; Parameters: "-u -h"; Flags: runhidden waituntilterminated

[UninstallDelete]
Type: filesandordirs; Name: "{localappdata}\OBSAudioRouter"
Type: files; Name: "{app}\obs-plugins\64bit\obs-audio-router.dll"
Type: filesandordirs; Name: "{app}\data\obs-plugins\obs-audio-router"

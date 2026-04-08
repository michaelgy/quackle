#define MyAppName "Quackle"
#define MyAppVersion GetEnv("QUACKLE_VERSION")
#define MyAppPublisher "Quackle Contributors"
#define MyAppURL "https://github.com/quackle/quackle"
#define MyAppExeName "Quackle.exe"

[Setup]
AppId={{A3F2B7C1-4D9E-4A1B-8C3F-2E6D5A0B9F7E}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputDir=installer_output
OutputBaseFilename=Quackle_Setup_{#MyAppVersion}
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
UninstallDisplayIcon={app}\{#MyAppExeName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
; Main executable
Source: "quacker\build\Release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion

; Qt DLLs
Source: "quacker\build\Release\Qt5Core.dll";    DestDir: "{app}"; Flags: ignoreversion
Source: "quacker\build\Release\Qt5Gui.dll";     DestDir: "{app}"; Flags: ignoreversion
Source: "quacker\build\Release\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "quacker\build\Release\Qt5Svg.dll";     DestDir: "{app}"; Flags: ignoreversion

; Runtime DLLs
Source: "quacker\build\Release\d3dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "quacker\build\Release\libEGL.dll";         DestDir: "{app}"; Flags: ignoreversion
Source: "quacker\build\Release\libGLESv2.dll";      DestDir: "{app}"; Flags: ignoreversion
Source: "quacker\build\Release\opengl32sw.dll";     DestDir: "{app}"; Flags: ignoreversion

; Qt plugin folders
Source: "quacker\build\Release\platforms\*";    DestDir: "{app}\platforms";    Flags: ignoreversion recursesubdirs
Source: "quacker\build\Release\styles\*";       DestDir: "{app}\styles";       Flags: ignoreversion recursesubdirs
Source: "quacker\build\Release\iconengines\*";  DestDir: "{app}\iconengines";  Flags: ignoreversion recursesubdirs
Source: "quacker\build\Release\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs
Source: "quacker\build\Release\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs

; Game data
Source: "data\*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs

[Icons]
Name: "{group}\{#MyAppName}";           Filename: "{app}\{#MyAppExeName}"
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}";     Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

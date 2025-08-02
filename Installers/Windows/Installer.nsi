!include x64.nsh
!include LogicLib.nsh
!include MUI2.nsh
!include nsDialogs.nsh
!include LogicLib.nsh

!define PLUG_NAME "Terrain"

!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\nsis3-install-alt.ico"
!define MUI_BGCOLOR "888888"
!define MUI_COMPONENTSPAGE_NODESC
!define MUI_PAGE_HEADER_TEXT "${PLUG_NAME}"
!define MUI_PAGE_HEADER_SUBTEXT "Plugin Installer"
!insertmacro MUI_PAGE_COMPONENTS          ; Page 1: VST3 / CLAP
Page custom SelectInstallScope Leave_SelectInstallScope ; Page 2: system vs user
Page custom SelectInstallPresets Leave_SelectInstallPresets ; Page 3: presets
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_LANGUAGE English

BrandingText "Aaron Anderson"

Name "${PLUG_NAME} 1.0.3 Installer"
OutFile "${PLUG_NAME} 1.0.3 Installer.exe"

RequestExecutionLevel admin

Var INSTALL_SCOPE
Var INSTALL_PRESETS
Var RADIO_SYS
Var RADIO_USER
Var CHECKBOX_PRESETS

;-------------------------------------------------------------
Function SelectInstallScope
    nsDialogs::Create 1018
    Pop $0
    ${If} $0 == error
        Abort
    ${EndIf}

    ${NSD_CreateLabel} 0 0 100% 12u "Install for:"
    Pop $1

    ${NSD_CreateRadioButton} 10u 20u 100% 12u "All users (system-wide)"
    Pop $RADIO_SYS
    SendMessage $RADIO_SYS ${BM_SETCHECK} ${BST_CHECKED} 0

    ${NSD_CreateRadioButton} 10u 35u 100% 12u "Current user only (user-wide)"
    Pop $RADIO_USER

    nsDialogs::Show
FunctionEnd

Function Leave_SelectInstallScope
    ${NSD_GetState} $RADIO_SYS $R0
    ${If} $R0 == ${BST_CHECKED}
        StrCpy $INSTALL_SCOPE "system"
    ${Else}
        StrCpy $INSTALL_SCOPE "user"
    ${EndIf}
FunctionEnd

;-------------------------------------------------------------
Function SelectInstallPresets
    nsDialogs::Create 1018
    Pop $0
    ${If} $0 == error
        Abort
    ${EndIf}

    ${NSD_CreateLabel} 0 0 100% 12u "Optional content:"
    Pop $1

    ${NSD_CreateCheckbox} 10u 20u 100% 12u "Install Presets"
    Pop $CHECKBOX_PRESETS
    ${NSD_Check} $CHECKBOX_PRESETS ; Checked by default

    nsDialogs::Show
FunctionEnd

Function Leave_SelectInstallPresets
    ${NSD_GetState} $CHECKBOX_PRESETS $R0
    ${If} $R0 == ${BST_CHECKED}
        StrCpy $INSTALL_PRESETS "yes"
    ${Else}
        StrCpy $INSTALL_PRESETS "no"
    ${EndIf}
FunctionEnd

;-------------------------------------------------------------
Section "${PLUG_NAME} VST3" VST3Section
    ${If} $INSTALL_SCOPE == "system"
        SetOutPath "$PROGRAMFILES64\Common Files\VST3\"
    ${Else}
        CreateDirectory "$APPDATA\VST3"
        SetOutPath "$APPDATA\VST3\"
    ${EndIf}
    File "VST3\${PLUG_NAME}.vst3"
SectionEnd

Section "${PLUG_NAME} CLAP" CLAPSection
    ${If} $INSTALL_SCOPE == "system"
        SetOutPath "$PROGRAMFILES64\Common Files\CLAP\"
    ${Else}
        CreateDirectory "$APPDATA\CLAP"
        SetOutPath "$APPDATA\CLAP\"
    ${EndIf}
    File "CLAP\${PLUG_NAME}.clap"
SectionEnd

Section "-InstallPresets"
    ${If} $INSTALL_PRESETS == "yes"
        CreateDirectory "$APPDATA\Aaron Anderson\Terrain"
        SetOutPath "$APPDATA\Aaron Anderson\Terrain"
        File /r "Presets\*.*"
    ${EndIf}
SectionEnd

Section "-InstallVCRedist"
    SetOutPath $TEMP
    File VC_redist_x64.exe
    ExecWait '"$TEMP\VC_redist_x64.exe" /passive /norestart /silent'
    Delete "$TEMP\VC_redist_x64.exe"
SectionEnd

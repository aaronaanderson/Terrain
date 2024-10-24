!include x64.nsh
!include LogicLib.nsh
!include MUI2.nsh

!define PLUG_NAME "Terrain"

!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\nsis3-install-alt.ico"
!define MUI_BGCOLOR "888888"
!define MUI_COMPONENTSPAGE_NODESC
!define MUI_PAGE_HEADER_TEXT "${PLUG_NAME}"
!define MUI_PAGE_HEADER_SUBTEXT "VST3 Installer"
!insertmacro MUI_PAGE_COMPONENTS

!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_LANGUAGE English

BrandingText "Aaron Anderson"

Name "${PLUG_NAME} 1.0.3 Installer"
OutFile "${PLUG_NAME} 1.0.3 Installer.exe"

RequestExecutionLevel admin

;------------------------------------------------------------------------
Section "${PLUG_NAME} VST3" VST3Section
SetOutPath "$PROGRAMFILES64\Common Files\VST3\"
File "VST3\${PLUG_NAME}.vst3"
SectionEnd

;------------------------------------------------------------------------
Section "-InstallVCRedist"
SetOutPath $TEMP
File VC_redist_x64.exe
ExecWait '"$TEMP\VC_redist_x64.exe"  /passive /norestart /silent'
Delete "$TEMP\VC_redist_x64.exe"
SectionEnd
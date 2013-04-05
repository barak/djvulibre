
; New version of the djvulibre+djview installer by Kravtsov Konstantin

!define DJVULIBRE_VERSION "3.5.25.3"
!define DJVIEW_VERSION "4.9.2"

!define PRODUCT_NAME "DjVuLibre+DjView"
!define PRODUCT_VERSION "${DJVULIBRE_VERSION}+${DJVIEW_VERSION}"
!define PRODUCT_PUBLISHER "DjVuZone"
!define PRODUCT_WEB_SITE "http://djvu.sourceforge.net"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\djview.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

!define INSTALLER_NAME "DjVuLibre (${DJVULIBRE_VERSION}) DjView (${DJVIEW_VERSION})"
!define INSTALLER_FILE "DjVuLibre-${DJVULIBRE_VERSION}_DjView-${DJVIEW_VERSION}_Setup.exe"
!define MENU_NAME "DjVuLibre"

Var DjvuDefaultOpen
Var DjvDefaultOpen

VIProductVersion ${PRODUCT_VERSION}
VIAddVersionKey "ProductName" ${PRODUCT_NAME}
VIAddVersionKey "FileDescription" "Viewer and utilities for DjVu files"
VIAddVersionKey "FileVersion" ${PRODUCT_VERSION}
VIAddVersionKey "DjVuLibreVersion" ${DJVULIBRE_VERSION}
VIAddVersionKey "DjViewVersion" ${DJVIEW_VERSION}
VIAddVersionKey "LegalCopyright" "GPL2+"
VIAddVersionKey "CompanyName" "DjVuZone"

SetCompressor /SOLID lzma

; WinVer -----

!include "LogicLib.nsh"
!include "WinVer.nsh"

; MUI 1.67 compatible ------

!include "MUI.nsh"
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define MUI_COMPONENTSPAGE_SMALLDESC
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "COPYING.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\djview.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch DjView"
!define MUI_FINISHPAGE_RUN_NOTCHECKED
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; Utils ------



; Sections ------

Name "${INSTALLER_NAME}"
OutFile "..\${INSTALLER_FILE}"
InstallDir "$PROGRAMFILES\DjVuZone\DjVuLibre"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

SectionGroup /e "!DjVuLibre"

 Section  "-filecopy" 
  ; install everything in fact
  SetOutPath "$INSTDIR"
  SetOverwrite try
  File "*.exe"
  File "*.dll"
  File "*.lib"
  File /r "include"
  File /r "man"
  File /r "plugins"
  File /r "share"
  File "qt.conf"
  File "djvulibre.nsi"
 SectionEnd

SectionGroupEnd

SectionGroup /e "!DjView"

 Section "-registry"
  ; -- app registration
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\djview.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "Path" "$INSTDIR"
  WriteRegStr HKCR "Applications\djview.exe" "FriendlyAppName" "DjView"
  WriteRegStr HKCR "Applications\djview.exe\SupportedTypes\.djvu" "" ""
  WriteRegStr HKCR "Applications\djview.exe\SupportedTypes\.djv" "" ""
  WriteRegStr HKCR "Applications\djview.exe\shell\open\command" "" '"$INSTDIR\djview.exe" "%1"'
  ${If} ${AtLeastWin7}
    WriteRegDWORD HKLM "${PRODUCT_DIR_REGKEY}" "UseUrl" 1
    WriteRegStr HKCR "Applications\djview.exe\SupportedProtocols\http" "" ""
    WriteRegStr HKCR "Applications\djview.exe\SupportedProtocols\https" "" ""
  ${EndIf}
  ; -- progid
  DeleteRegKey HKCR "Djview.DjVuFile"
  WriteRegStr HKCR "Djview.DjVuFile" "" "Djview.DjVuFile"
  WriteRegStr HKCR "Djview.DjVuFile\DefaultIcon" "" "$INSTDIR\djview.exe,1"
  WriteRegStr HKCR "Djview.DjVuFile\shell\open\command" "" '"$INSTDIR\djview.exe" "%1"'
  ; -- openwith
  WriteRegStr HKCR ".djv\OpenWithProgids" "Djview.DjVuFile" ""
  WriteRegStr HKCR ".djvu\OpenWithProgids" "Djview.DjVuFile" ""
 SectionEnd

 Section "-startmenu"
  CreateDirectory "$SMPROGRAMS\${MENU_NAME}"
  WriteIniStr "$INSTDIR\${MENU_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\Website.lnk" "$INSTDIR\${MENU_NAME}.url"
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\Documentation.lnk" "$INSTDIR\man"
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\Uninstall.lnk" "$INSTDIR\uninst.exe"
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\DjView.lnk" "$INSTDIR\djview.exe"
 SectionEnd

 Section "Associate DjVu files with DjView"
  ReadRegStr $DjvuDefaultOpen HKCR ".djvu" ""
  ReadRegStr $DjvDefaultOpen HKCR ".djv" ""
  WriteRegStr HKCR ".djvu" "" "Djview.DjVuFile"
  WriteRegStr HKCR ".djv" "" "Djview.DjVuFile"
  ${If} $DjvuDefaultOpen != "Djview.DjVuFile"
    WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "sav.djvu" $DjvuDefaultOpen
  ${EndIf}
  ${If} $DjvuDefaultOpen != "Djview.DjVuFile"
    WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "sav.djv" $DjvDefaultOpen
  ${EndIf}
 SectionEnd

 SectionGroup /e "Create shortcuts" 
  Section "Desktop"
   CreateShortCut "$DESKTOP\DjView.lnk" "$INSTDIR\djview.exe"
  SectionEnd
  Section /o "Quick Launch"
   CreateShortCut "$QUICKLAUNCH\DjView.lnk" "$INSTDIR\djview.exe"
  SectionEnd
 SectionGroupEnd

SectionGroupEnd

Section "-post"
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}  ${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\djview.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

Section Uninstall
  ; -- remove files
  SetOutPath $TEMP
  RMDir /r /REBOOTOK "$INSTDIR"
  ; -- remove shortcuts
  RMDir /r /REBOOTOK "$SMPROGRAMS\${MENU_NAME}"
  Delete "$DESKTOP\DjView.lnk"
  Delete "$QUICKLAUNCH\DjView.lnk"
  ; -- restore associations
  ClearErrors
  ReadRegStr $DjvuDefaultOpen ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "sav.djvu"
  ${IfNot} ${Errors}
  ${AndIf} $DjvuDefaultOpen != ""
    WriteRegStr HKCR ".djvu" "" $DjvuDefaultOpen
  ${EndIf}
  ClearErrors
  ReadRegStr $DjvDefaultOpen ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "sav.djv"
  ${IfNot} ${Errors}
  ${AndIf} $DjvDefaultOpen != ""
    WriteRegStr HKCR ".djv" "" $DjvDefaultOpen
  ${EndIf}
  ; -- delete progid
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  DeleteRegKey HKLM "Software\Classes\Djview.DjVuFile"
  DeleteRegKey HKCR "Applications\djview.exe"
  DeleteRegValue HKCR ".djvu\OpenWithProgids" "Djview.DjVuFile"
  DeleteRegValue HKCR ".djv\OpenWithProgids" "Djview.DjVuFile"
  ; -- delete uninst
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  SetAutoClose true
SectionEnd

; Language strings ------

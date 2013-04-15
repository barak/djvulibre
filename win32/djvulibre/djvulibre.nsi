;---------------------------------------------------------;
;Created 08 april 2013 by                                 ;
;(C) Kravtsov Konstantin Ivanovich, Novokuznetsk, Russia  ;
;State of Art edition 8.SoA                               ;
;----------------------------------------------------------

;Used software:
;--------------
;NSIS 2.46+log (http://sourceforge.net/projects/nsis/files/NSIS%202/2.46/nsis-2.46-log.zip/download)
;HM NIS Edit 2.0.3 -- it's good IDE for NSI building
;Notepad ++ 5.9 UNICODE
;Used docs
;---------
;NSIS docs and manuals at http://forum.oszone.net/thread-248731.html
;NSIS help at http://forum.oszone.net/thread-168287.html
;NSIS 2.46 User Manual shipped with
;

;INSTALLER START
;---------------
;prepare names

RequestExecutionLevel admin

!define DJVULIBRE_NAME "DjVuLibre"
!define DJVULIBRE_VERSION "3.5.25.3"

!define DJVIEW_NAME "DjView"
!define DJVIEW_VERSION "4.9.2"

!define PRODUCT_NAME "${DJVULIBRE_NAME} ${DJVIEW_NAME}"
!define PRODUCT_NAME2 "${DJVULIBRE_NAME}+${DJVIEW_NAME}" ; for uninstaller
!define MENU_NAME "${DJVULIBRE_NAME}"
!define PRODUCT_VERSION "${DJVULIBRE_VERSION}+${DJVIEW_VERSION}"


;provide other info
!define PRODUCT_PUBLISHER "DjVuZone"
!define DJVULIBRE_WEB_SITE "http://djvu.sourceforge.net"
!define DJVUORG_WEB_SITE "http://djvu.org"
!define PRODUCT_DOWNLOAD_PAGE "http://sourceforge.net/projects/djvu/files/DjVuLibre_Windows/"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\djview.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME2}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"


;label for HCR
!define FILEID "Djview.DjVuFile"
!define NO_EXT "no key"
!define BACKUP_EXT "DjView.Backup"

Var Djvu_EXT ; ext var
Var COUNT    ; counter var
Var KEY_VAL ; read key
Var TMP_EXT  ; for temp reading ext val from reg

;fill installer description
VIProductVersion "${PRODUCT_VERSION}"

;end-user doesnot see
;djvulibre version anywhere except installer
;dll's and utils doesn't report any
VIAddVersionKey "DjvuLibreVersion" ${DJVULIBRE_VERSION}
VIAddVersionKey "DjViewVersion" ${DJVIEW_VERSION}
VIAddVersionKey "Installer by" "Konstantin Kravtsov (C) 2013"
VIAddVersionKey "FileVersion" ${PRODUCT_VERSION}
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "LegalCopyright" "GPL v2+"
VIAddVersionKey "CompanyName" ${PRODUCT_PUBLISHER}
VIAddVersionKey "FileDescription" "DjVu view, edit and create tools for Windows"

;----------------
;Includes section
;----------------

!include "LogicLib.nsh"
!include "WinVer.nsh"


;---------------
;Install section
;---------------

SetCompressor /SOLID lzma
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "..\DjVuLibre-${DJVULIBRE_VERSION}_DjView-${DJVIEW_VERSION}_Setup.exe"
InstallDir "$PROGRAMFILES\DjVuLibre"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

; MUI 1.67 compatible ------
!include "MUI.nsh"
;sometimes need to install diff lang
!define MUI_LANGDLL_ALWAYSSHOW
; MUI Settings
!define MUI_ABORTWARNING
;!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\win.bmp"
;custom header img
!define MUI_HEADERIMAGE
;!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\win.bmp"
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\win-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\win-uninstall.ico"
; Language Selection Dialog Settings
!define MUI_LANGDLL_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "NSIS:Language"
;Show small desc
!define MUI_COMPONENTSPAGE_SMALLDESC
; Welcome page 3 lines
!define MUI_WELCOMEPAGE_TITLE_3LINES
; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "COPYING.txt"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
;Install Options page
!insertmacro MUI_PAGE_COMPONENTS
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_NOAUTOCLOSE
; Finish page
!define MUI_FINISHPAGE_RUN " "
!define MUI_FINISHPAGE_RUN_TEXT $(Launch_LAB)
!define MUI_FINISHPAGE_RUN_NOTCHECKED
!define MUI_FINISHPAGE_RUN_FUNCTION "LaunchReadme"
; Finish page 3 lines
!define MUI_FINISHPAGE_TITLE_3LINES
!insertmacro MUI_PAGE_FINISH
; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES
; Reserve files
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
; Language files
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Russian"
; end MUI 1.67 compatible ------


Function .onInit
  ;check language
  !insertmacro MUI_LANGDLL_DISPLAY
  ;check existing install
  ReadRegStr $KEY_VAL  ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString"
  ${If} "$KEY_VAL" != ""
    ReadRegStr $COUNT ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion"
    ${If} "$COUNT" == "${PRODUCT_VERSION}"
      MessageBox MB_YESNO|MB_ICONEXCLAMATION $(Message_AlreadyInstalled) IDNO done
    ${Else}
      MessageBox MB_YESNO|MB_ICONEXCLAMATION "$(Message_AnotherInstalled) $COUNT ?" IDNO done
    ${EndIf}
    ;Run the uninstaller
    ClearErrors
    ExecWait '"$KEY_VAL" /S' ;; silent
  ${EndIf}
done:
FunctionEnd

Function RegisterExt
  ; Kravtsov Konstantin Ivanovich (C) 2013
  ; for register ext. Set $Djvu_EXT with ext and call
  ClearErrors
  ReadRegStr $TMP_EXT HKCR $Djvu_EXT ""
  ${If} ${Errors}
    WriteRegStr HKCR "$Djvu_EXT" ${BACKUP_EXT} "${NO_EXT}"
  ${ElseIf} "$TMP_EXT" != "${FILEID}"
    WriteRegStr HKCR "$Djvu_EXT" "${BACKUP_EXT}" "$TMP_EXT"
  ${EndIf}
  WriteRegStr HKCR "$Djvu_EXT" "" "${FILEID}"
FunctionEnd

Function LaunchReadme
  Exec '"$INSTDIR\djview.exe" --outline --continuous "$INSTDIR\doc\djvulibre-book-en.djvu"'
FunctionEnd

!define WriteUrlFile "!insertmacro WriteUrlFile"
!macro WriteUrlFile file url
  Push "${file}"
  Push "${url}"
  call WriteUrlFile
!macroend
Function WriteUrlFile
  Pop $2
  Pop $1
  DetailPrint "Creating $1"
  WriteIniStr "$1" "InternetShortcut" "URL" "$2"
  WriteIniStr "$1" "InternetShortcut" "IconFile" "$WINDIR\system32\url.dll"
  WriteIniStr "$1" "InternetShortcut" "IconIndex" "0"
FunctionEnd


SectionGroup "!DjVuLibre" scDjVuLibre

Section "-filecopy"
  SectionIn RO
  ;; all files are installed here in fact
  SetOutPath "$INSTDIR"
  SetOverwrite try
  Logset on
  File "*.exe"
  File "*.dll"
  File "*.lib"
  File /r "include"
  File /r "doc"
  File /r "plugins"
  File /r "share"
  File "qt.conf"
  File "djvulibre.nsi"
  File "COPYING.txt"
  ${WriteUrlFile} "$INSTDIR\DjVu.url" "${DJVUORG_WEB_SITE}"
  ${WriteUrlFile} "$INSTDIR\DjVuLibre.url" "${DJVULIBRE_WEB_SITE}"
  ${WriteUrlFile} "$INSTDIR\Download page.url" "${PRODUCT_DOWNLOAD_PAGE}"
  Logset off
SectionEnd

SectionGroupEnd ; djvulibre

SectionGroup /e "!DjView" scDjView

Section "-registry"
  SectionIn RO
  DetailPrint "Updating registry"
  ;; app registration
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
  ;; fileid
  DeleteRegKey HKCR "${FILEID}"
  WriteRegStr HKCR "${FILEID}" "" "DjVu File"
  WriteRegStr HKCR "${FILEID}\DefaultIcon" "" "$INSTDIR\djview.exe,1"
  WriteRegStr HKCR "${FILEID}\shell\open\command" "" '"$INSTDIR\djview.exe" "%1"'
  ;; open with
  WriteRegStr HKCR ".djv\OpenWithProgids" "Djview.DjVuFile" ""
  WriteRegStr HKCR ".djvu\OpenWithProgids" "Djview.DjVuFile" ""
  ClearErrors
  ReadRegStr $TMP_EXT HKCR "DjVu.Document" ""
  ${IfNot} ${Errors}
    WriteRegStr HKCR ".djv\OpenWithProgids" "DjVu.Document" ""
    WriteRegStr HKCR ".djvu\OpenWithProgids" "DjVu.Document" ""
  ${EndIf}
SectionEnd

Section "$(secAssoc)" scAssoc
  DetailPrint "Creating associations"
  StrCpy $Djvu_EXT ".djvu"
  Call RegisterExt
  StrCpy $Djvu_EXT ".djv"
  Call RegisterExt
SectionEnd

SectionGroup /e "$(secShort)" scShort

Section "-menuentries"
  SectionIn RO
  DetailPrint "Creating menu entries"
  ;; all menu entries
  LogSet on
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\${MENU_NAME}"
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\DjView.lnk" "$INSTDIR\djview.exe"
  CreateDirectory "$SMPROGRAMS\${MENU_NAME}\$(Uninst_DIR)"
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Uninst_DIR)\$(Uninst_LNK).lnk" "$INSTDIR\uninst.exe"
  CreateDirectory "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)"
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\$(Djv_LNK).lnk" "$INSTDIR\DjVu.url" "" "$WINDIR\system32\url.dll" 0
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\$(Web_LNK).lnk" "$INSTDIR\DjVuLibre.url" "" "$WINDIR\system32\url.dll" 0
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\$(WebDL_LNK).lnk" "$INSTDIR\Download page.url" "" "$WINDIR\system32\url.dll" 0
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\$(Lic_TXT).lnk" "$INSTDIR\COPYING.txt"
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\DjVuLibre documentation.lnk" "$INSTDIR\djview.exe" '--outline --continuous "$INSTDIR\doc\djvulibre-book-en.djvu"'
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\Specification of the DjVu Format.lnk" "$INSTDIR\djview.exe" '--outline --continuous "$INSTDIR\doc\djvu3spec.djvu"'
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\Specification changes.lnk" "$INSTDIR\doc\djvuchanges.txt"
  ${If} $Language == ${LANG_RUSSIAN}
    CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\$(Doc_LNK) ${DJVULIBRE_NAME}.lnk" "$INSTDIR\doc\djvulibre-book-ru.djvu"
  ${EndIf}
  LogSet off
SectionEnd

Section "$(secDesk)" scDesk
  DetailPrint "Creating desktop shortcut"
  SetShellVarContext all
  LogSet on
  CreateShortCut "$DESKTOP\DjView.lnk" "$INSTDIR\djview.exe"
  Logset off
SectionEnd
  
Section /o "$(secQuick)" scQuick
  DetailPrint "Creating quick launch shortcut"
  SetShellVarContext all
  LogSet on
  CreateShortCut "$QUICKLAUNCH\DjView.lnk" "$INSTDIR\djview.exe"
  LogSet off
SectionEnd

SectionGroupEnd ; shortcuts

SectionGroupEnd ; djview

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\djview.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "Path" "$INSTDIR"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}  ${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\djview.exe,0"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${DJVULIBRE_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  DetailPrint "DjVuLibre installer: Kravtsov Konstantin Ivanovich, Novokuznetsk, Russia."
SectionEnd



;------------
; Uninstaller 
;------------


!define UnTrim "!insertmacro UnTrim"
!macro UnTrim ResultVar String
  Push ${String}
  Call un.Trim
  Pop ${ResultVar}
!macroend

Function un.Trim
  Exch $R0
  Push $R1
  Push $R2
  ; trim from start
  StrCpy $R1 0
  ${Do}
    StrCpy $R2 $R0 1 $R1
    ${If} "$R2" != " "
    ${AndIf} "$R2" != "$\t"
    ${AndIf} "$R2" != "$\r"
    ${AndIf} "$R2" != "$\n"
      ${ExitDo}
    ${EndIf}
    IntOp $R1 $R1 + 1
  ${Loop}
  ${If} $R1 != 0
    StrCpy $R0 $R0 "" $R1
  ${EndIf}
  ; trim from end
  StrCpy $R1 0
  ${Do}
    IntOp $R1 $R1 - 1
    StrCpy $R2 $R0 1 $R1
    ${If} $R2 != "$\r"
    ${AndIf} $R2 != "$\n"
    ${AndIf} $R2 != "$\t"
    ${AndIf} $R2 != " "
      IntOp $R1 $R1 + 1
      ${ExitDo}
    ${EndIf}
  ${Loop}
  ${If} $R1 != 0
   StrCpy $R0 $R0 $R1
  ${EndIf}
  ; remove quotes
  StrCpy $R1 $R0 1
  StrCpy $R2 $R0 1 -1
  ${If} $R1 == "$\""
  ${AndIf} $R2 == "$\""
    StrCpy $R0 $R0 "" 1
    StrCpy $R0 $R0 -1
  ${EndIf}
  ; return
  Pop $R2
  Pop $R1
  Exch $R0
FunctionEnd

!define UnStrLoc "!insertmacro UnStrLoc"
!macro UnStrLoc ResultVar String SubString StartPoint
  Push "${String}"
  Push "${SubString}"
  Push "${StartPoint}"
  Call un.StrLoc
  Pop "${ResultVar}"
!macroend

Function un.StrLoc
/*After this point:
  ------------------------------------------
   $R0 = StartPoint (input)
   $R1 = SubString (input)
   $R2 = String (input)
   $R3 = SubStringLen (temp)
   $R4 = StrLen (temp)
   $R5 = StartCharPos (temp)
   $R6 = TempStr (temp)*/

  ;Get input from user
  Exch $R0
  Exch
  Exch $R1
  Exch 2
  Exch $R2
  Push $R3
  Push $R4
  Push $R5
  Push $R6
  ;Get "String" and "SubString" length
  StrLen $R3 $R1
  StrLen $R4 $R2
  ;Start "StartCharPos" counter
  StrCpy $R5 0
  ;Loop until "SubString" is found or "String" reaches its end
  ${Do}
    ;Remove everything before and after the searched part ("TempStr")
    StrCpy $R6 $R2 $R3 $R5
    ;Compare "TempStr" with "SubString"
    ${If} $R6 == $R1
      ${If} $R0 == `<`
        IntOp $R6 $R3 + $R5
        IntOp $R0 $R4 - $R6
      ${Else}
        StrCpy $R0 $R5
      ${EndIf}
      ${ExitDo}
    ${EndIf}
    ;If not "SubString", this could be "String"'s end
    ${If} $R5 >= $R4
      StrCpy $R0 ``
      ${ExitDo}
    ${EndIf}
    ;If not, continue the loop
    IntOp $R5 $R5 + 1
  ${Loop}
  ;Return output to user
  Pop $R6
  Pop $R5
  Pop $R4
  Pop $R3
  Pop $R2
  Exch
  Pop $R1
  Exch $R0
FunctionEnd

Function un.RemoveFiles
  DetailPrint "Removing Files.."
  StrCpy $R7 "$INSTDIR\install.log"
  ${If} ${FileExists} "$R7"
    ClearErrors
    FileOpen $0 "$R7" "r"
    ${Do}
      ClearErrors
      FileRead $0 $1
      ${If} ${Errors}
         ${ExitDo}
      ${EndIf}
      StrCpy $2 $1 11
      ${If} $2 == "File: wrote"
        ${UnStrloc} $3 $1 "to " ">"
        Intop $3 $3 + 2
        StrCpy $5 $1 "" $3
        ${UnTrim} $6 $5
        Delete /REBOOTOK "$6"
      ${ElseIf} $2 == "WriteINIStr"
        ${UnStrloc} $3 $1 "in " ">"
        Intop $3 $3 + 2
        StrCpy $5 $1 "" $3
        ${UnTrim} $6 $5
        Delete /REBOOTOK "$6"
      ${EndIf}
    ${Loop}
    FileClose $0
  ${Else}
    ;; RMDir /r "$INSTDIR"  ; too bad
  ${EndIf}
FunctionEnd

Function un.RemoveEmptyDirsSub
  Pop $9
  FindFirst $0 $1 "$INSTDIR$9*"
  ${IfNot} "$0" == ""
    ${Do}
      ${If} "$1" == ""
         ${ExitDo}
      ${EndIf}
      ${If} "$1" != "."
      ${AndIf} "$1" != ".."
        Push $0
        Push $1
        Push $9
        Push "$9$1\"
        Call un.RemoveEmptyDirsSub
        Pop $9
        Pop $1
        Pop $0
        RMDir "$INSTDIR$9$1"
      ${EndIf}
      FindNext $0 $1
    ${Loop}
    FindClose $0
  ${EndIf}
FunctionEnd

Function un.RegisterExt
; Kravtsov Konstantin Ivanovich (C) 2013
; restore assoc from uninst key
; check if restore really need
  ClearErrors
  ReadRegStr $TMP_EXT HKCR $Djvu_EXT ""
  ${IfNot} ${Errors}
     ${If} "$TMP_EXT" == ${FILEID}
       ClearErrors
       ReadRegStr $TMP_EXT HKCR $Djvu_EXT ${BACKUP_EXT}
       ${IfNot} ${Errors}
         ${If} "$TMP_EXT" == "${NO_EXT}"
           ; NBell had a complex loop to search alternatives
           ; But how to be sure that what the loop finds is legal
           DeleteRegKey HKCR $Djvu_EXT
         ${Else}
           WriteRegStr HKCR $Djvu_EXT "" $TMP_EXT
         ${EndIf}
         DeleteRegValue HKCR $Djvu_EXT ${BACKUP_EXT}
       ${EndIf}
     ${EndIf}
  ${EndIf}
FunctionEnd

Function un.onUninstSuccess
  IfSilent +2
  MessageBox MB_ICONINFORMATION|MB_OK $(UninstSux_MSG)
FunctionEnd

Function un.onInit
  IfSilent +3
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 $(Uninst_MSG) IDYES +2
  Abort
FunctionEnd

Section Uninstall
  SetShellVarContext all
  IfSilent +2
  SetDetailsPrint both
  ;; remove all files
  Call un.RemoveFiles
  Delete "$INSTDIR\install.log"
  Delete "$INSTDIR\uninst.exe"
  Push "\"
  Call un.RemoveEmptyDirsSub
  RMDir "$INSTDIR"
  ;; delete menu (save becauseee SMSPROGRAMS is well defined)
  RmDir /r "$SMPROGRAMS\${MENU_NAME}"
  ;; delete shortcuts
  Delete "$DESKTOP\DjView.lnk"
  Delete "$QUICKLAUNCH\DjView.lnk"
  ;; clean registry
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  DeleteRegKey HKCR "${FILEID}"
  DeleteRegKey HKCR "Applications\djview.exe"
  DeleteRegValue HKCR ".djvu\OpenWithProgIDs" "${FILEID}"
  DeleteRegValue HKCR ".djv\OpenWithProgIDs" "${FILEID}"
  StrCpy $Djvu_EXT ".djvu"
  Call un.RegisterExt
  StrCpy $Djvu_EXT ".djv"
  Call un.RegisterExt
  ;; clean uninstall key
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  SetAutoClose true
SectionEnd

;Lang strings RU

LangString DESC_DjVuLibre ${LANG_RUSSIAN} "утилитами командной строки DjVu ${DJVULIBRE_NAME} ${DJVULIBRE_VERSION}"
LangString DESC_DjView ${LANG_RUSSIAN} "Бесплатный просмотрщик DjVu ${DJVIEW_NAME} ${DJVIEW_VERSION}"
LangString DESC_Assoc ${LANG_RUSSIAN} "Установка ${PRODUCT_NAME} программой по умолчанию для djv и djvu"
LangString DESC_Short ${LANG_RUSSIAN} "Настройки создания ярлыков в меню Пуск-Программы, Панели быстрого запуска и на Рабочем столе"
LangString DESC_Quick ${LANG_RUSSIAN} "Создать ярлык DjView на Панели быстрого запуска"
LangString DESC_Desk ${LANG_RUSSIAN} "Создать ярлык DjView на Рабочем столе"
LangString secAssoc ${LANG_RUSSIAN} "Ассоциировать с файлами DjVu"
LangString secShort ${LANG_RUSSIAN} "Создать ярлыки"
LangString secQuick ${LANG_RUSSIAN} "На Панели быстрого запуска"
LangString secDesk ${LANG_RUSSIAN} "На Рабочем столе"
LangString Uninst_DIR ${LANG_RUSSIAN} "Удаление"
LangString Uninst_LNK ${LANG_RUSSIAN} "Удалить полностью ${PRODUCT_NAME}"
LangString Doc_DIR ${LANG_RUSSIAN} "Справочник"
LangString Djv_LNK ${LANG_RUSSIAN} "Веб-сайт DjVu.org"
LangString Web_LNK ${LANG_RUSSIAN} "Веб-сайт DjVuLibre"
LangString WebDL_LNK ${LANG_RUSSIAN} "Страница загрузки DjVuLibre"
LangString Doc_LNK ${LANG_RUSSIAN} "Справка (англ.)"
LangString Lic_TXT ${LANG_RUSSIAN} "Лицензия"
LangString Uninst_MSG ${LANG_RUSSIAN} "Вы точно хотите полностью удалить с компьютера $(^Name)?"
LangString UninstSux_MSG ${LANG_RUSSIAN} "Программа $(^Name) благополучно удалена."
LangString Launch_LAB ${LANG_RUSSIAN} "Запустить DjView"
LangString Message_AlreadyInstalled ${LANG_RUSSIAN} "${PRODUCT_NAME}  ${PRODUCT_VERSION} уже установлен. Удалить?"
LangString Message_AnotherInstalled ${LANG_RUSSIAN} "${PRODUCT_NAME} уже установлен. Удалить версию "

;Lang strings EN

LangString DESC_DjVuLibre ${LANG_ENGLISH} "DjVu command line utilities ${DJVULIBRE_NAME} ${DJVULIBRE_VERSION}"
LangString DESC_DjView ${LANG_ENGLISH} "DjVu viewer ${DJVIEW_NAME} ${DJVIEW_VERSION}"
LangString DESC_Assoc ${LANG_ENGLISH} "Set DjView as default DjVu viewer"
LangString DESC_Short ${LANG_ENGLISH} "Create DjView shortcuts on the Desktop and in the Quick Launch Bar"
LangString DESC_Quick ${LANG_ENGLISH} "Create DjView shortcut in the Quick Launch Bar"
LangString DESC_Desk ${LANG_ENGLISH} "Create DjView shortcut on the Desktop"
LangString secAssoc ${LANG_ENGLISH} "Set as default DjVu file viewer"
LangString secShort ${LANG_ENGLISH} "Shortcuts"
LangString secQuick ${LANG_ENGLISH} "Quick Launch Bar"
LangString secDesk ${LANG_ENGLISH} "Desktop"
LangString Uninst_DIR ${LANG_ENGLISH}  "Uninstall"
LangString Uninst_LNK ${LANG_ENGLISH} "Completely remove ${PRODUCT_NAME}"
LangString Doc_DIR ${LANG_ENGLISH} "Help"
LangString Djv_LNK ${LANG_ENGLISH}  "Visit DjVu.org"
LangString Web_LNK ${LANG_ENGLISH}  "Visit DjVuLibre"
LangString WebDL_LNK ${LANG_ENGLISH}  "Visit the DjVuLibre download page"
LangString Doc_LNK ${LANG_ENGLISH} "Documentation"
LangString Lic_TXT ${LANG_ENGLISH} "License"
LangString Uninst_MSG ${LANG_ENGLISH} "Are you sure you want to completely remove $(^Name) and all of its components?"
LangString UninstSux_MSG ${LANG_ENGLISH} "$(^Name) was successfully removed from your computer."
LangString Launch_LAB ${LANG_ENGLISH} "Launch DjView"
LangString Message_AnotherInstalled ${LANG_ENGLISH} "Another copy of ${PRODUCT_NAME} is installed. Uninstall?"
LangString Message_AlreadyInstalled ${LANG_ENGLISH} "${PRODUCT_NAME} ${PRODUCT_VERSION} is already installed. Uninstall?"

; Section descriptions set

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${scDjVuLibre} $(DESC_DjVuLibre)
!insertmacro MUI_DESCRIPTION_TEXT ${scDjView} $(DESC_DjView)
!insertmacro MUI_DESCRIPTION_TEXT ${scAssoc} $(DESC_Assoc)
!insertmacro MUI_DESCRIPTION_TEXT ${scShort} $(DESC_Short)
!insertmacro MUI_DESCRIPTION_TEXT ${scQuick} $(DESC_Quick)
!insertmacro MUI_DESCRIPTION_TEXT ${scDesk} $(DESC_Desk)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

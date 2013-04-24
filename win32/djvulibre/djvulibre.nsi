;---------------------------------------------------------;
;Created 08-24 april 2013 by                                 ;
;(C) Kravtsov Konstantin Ivanovich, Novokuznetsk, Russia  ;
;State of Art edition version - see below NSI_VER                                   ;
;LogicLib and Winver applied by Leon Bottou               ;             ;
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
;
;INSTALLER START
;---------------
;prepare names

RequestExecutionLevel admin
!define NSI_VER "9.2"
!define DJVULIBRE_NAME "DjVuLibre"
!define DJVULIBRE_VERSION "3.5.25.3"
!define CLASSES "Software\Classes\"
!define DJVIEW_NAME "DjView"
!define DJVIEW_VERSION "4.9.2"

!define PRODUCT_NAME "${DJVULIBRE_NAME} ${DJVIEW_NAME}"
!define UNINST_NAME "${DJVULIBRE_NAME}+${DJVIEW_NAME}" ; for uninstaller
!define MENU_NAME "${DJVULIBRE_NAME}"
!define PRODUCT_VERSION "${DJVULIBRE_VERSION}+${DJVIEW_VERSION}"

;provide other info
!define PRODUCT_PUBLISHER "DjVuZone"
!define DJVULIBRE_WEB_SITE "http://djvu.sourceforge.net"
!define DJVUORG_WEB_SITE "http://djvu.org"
!define PRODUCT_DOWNLOAD_PAGE "http://sourceforge.net/projects/djvu/files/DjVuLibre_Windows/"
!define PRODUCT_ONLINEHELP_PAGE "http://djvu.sourceforge.net/doc/index.html"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\djview.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINST_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"


;label for HCR
!define FILEID "Djview.DjVuFile"
!define NO_EXT "no key"
!define BACKUP_EXT "DjView.Backup"
;const for url.lnk
!define RUN_URL "$WINDIR\system32\rundll32.exe"
!define ICO_URL "$WINDIR\system32\url.dll"
!define URL_PAR " url.dll,FileProtocolHandler"
Var Djvu_EXT ; ext var
Var COUNT    ; counter var
Var KEY_VAL  ; readed key valuse
Var TMP_EXT  ; for temp reading ext val from reg
Var INST_LOG_REN ; flag of exist install.log

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
;workaround lang change effect
!define MUI_PAGE_CUSTOMFUNCTION_PRE preWelcomePage
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
FunctionEnd

Function preWelcomePage
  ReadRegStr $KEY_VAL  ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString"
  ${If} "$KEY_VAL" != ""
    ReadRegStr $COUNT ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion"
    ${If} "$COUNT" == "${PRODUCT_VERSION}"
      MessageBox MB_YESNO|MB_ICONEXCLAMATION $(Message_AlreadyInstalled) IDNO done
    ${ElseIf} "$COUNT" S< "${PRODUCT_VERSION}"
      MessageBox MB_YESNO|MB_ICONEXCLAMATION $(Message_OldFound) IDNO done
    ${ElseIf} "$COUNT" S> "${PRODUCT_VERSION}"
      MessageBox MB_YESNO|MB_ICONEXCLAMATION $(Message_NewFound) IDNO done
    ${EndIf}

    ;Run the uninstaller
    ClearErrors
    ExecWait '"$KEY_VAL" /S' ;; silent
  ${EndIf}

done:

FunctionEnd

Function RegisterExt
  ; Kravtsov Konstantin Ivanovich (C) 2013
  ; for register ext. Push EXT with ext and call
  Pop $Djvu_EXT
  ClearErrors
  ReadRegStr $TMP_EXT HKLM "${CLASSES}$Djvu_EXT" ""
  ${If} ${Errors}
    WriteRegStr HKLM "${CLASSES}$Djvu_EXT" ${BACKUP_EXT} "${NO_EXT}"
  ${ElseIf} "$TMP_EXT" != "${FILEID}"
    WriteRegStr HKLM "${CLASSES}$Djvu_EXT" "${BACKUP_EXT}" "$TMP_EXT"
  ${EndIf}
  WriteRegStr HKLM "${CLASSES}$Djvu_EXT" "" "${FILEID}"
FunctionEnd

Function LaunchReadme
  Exec '"$INSTDIR\djview.exe" --outline --continuous "$INSTDIR\doc\djvulibre-book-en.djvu"'
FunctionEnd

;!define WriteUrlFile "!insertmacro WriteUrlFile"
;!macro WriteUrlFile file url
;  Push "${file}"
;  Push "${url}"
;  call WriteUrlFile
;!macroend
;Function WriteUrlFile
;  Pop $2
;  Pop $1
;  DetailPrint "Creating $1"
;  WriteIniStr "$1" "InternetShortcut" "URL" "$2"
;  WriteIniStr "$1" "InternetShortcut" "IconFile" "$WINDIR\system32\url.dll"
;  WriteIniStr "$1" "InternetShortcut" "IconIndex" "0"
;FunctionEnd

;---rep
;SectionGroup "!DjVuLibre" scDjVuLibre
Section "-!DjVuLibre" scDjVuLibre
;---useless
;Section "-filecopy"
  SectionIn RO
  ;; all files are installed here in fact
  SetOutPath "$INSTDIR"
  SetOverwrite try
StrCpy $INST_LOG_REN 0 ;set var to zero
StrCpy $TMP_EXT "$INSTDIR\install.log"
${If} ${FileExists} $TMP_EXT
  Rename $TMP_EXT $INSTDIR\install.kik
  StrCpy $INST_LOG_REN 1 ;set var to rename was
${EndIf}

StrCpy $TMP_EXT "$INSTDIR\uninstall.${PRODUCT_VERSION}"
${If} ${FileExists} $TMP_EXT

  Rename $INSTDIR\uninstall.${PRODUCT_VERSION} $INSTDIR\install.log

${EndIf}



  Logset on
  File "*.exe"
  File "*.dll"
  File "*.lib"
  File /r "include"
  File /r "doc"
  File /r "plugins"
  File /r "share"
  File "qt.conf"
  File "djvulibre*.ns?"
  File "COPYING.txt"
;no need to dub start menu and instdir -->Section "-menuentries"
;  ${WriteUrlFile} "$INSTDIR\DjVu.url" "${DJVUORG_WEB_SITE}"
;  ${WriteUrlFile} "$INSTDIR\DjVuLibre.url" "${DJVULIBRE_WEB_SITE}"
;  ${WriteUrlFile} "$INSTDIR\Download page.url" "${PRODUCT_DOWNLOAD_PAGE}"
  Logset off
SectionEnd

;SectionGroupEnd ; djvulibre
;---rep - can be skipped
;start reg app

Section "-registry"
  SectionIn RO
  DetailPrint "Updating registry"
  ;; app registration
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\djview.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "Path" "$INSTDIR"
  WriteRegStr HKLM "${CLASSES}Applications\djview.exe" "FriendlyAppName" "DjView"
  WriteRegStr HKLM "${CLASSES}Applications\djview.exe\SupportedTypes\.djvu" "" ""
  WriteRegStr HKLM "${CLASSES}Applications\djview.exe\SupportedTypes\.djv" "" ""
  WriteRegStr HKLM "${CLASSES}Applications\djview.exe\shell\open\command" "" '"$INSTDIR\djview.exe" "%1"'
  ${If} ${AtLeastWin7}
    WriteRegDWORD HKLM "${PRODUCT_DIR_REGKEY}" "UseUrl" 1
    WriteRegStr HKLM "${CLASSES}Applications\djview.exe\SupportedProtocols\http" "" ""
    WriteRegStr HKLM "${CLASSES}Applications\djview.exe\SupportedProtocols\https" "" ""
  ${EndIf}
  ;; fileid
  DeleteRegKey HKLM "${CLASSES}${FILEID}"
  WriteRegStr HKLM "${CLASSES}${FILEID}" "" "DjVu File"
  WriteRegStr HKLM "${CLASSES}${FILEID}\DefaultIcon" "" "$INSTDIR\djview.exe,1"
  WriteRegStr HKLM "${CLASSES}${FILEID}\shell\open\command" "" '"$INSTDIR\djview.exe" "%1"'
  ;; open with
;  WriteRegStr HKCR ".djv\OpenWithProgids" "Djview.DjVuFile" ""
  WriteRegStr HKLM "${CLASSES}.djv\OpenWithProgids" ${FILEID} ""
;  WriteRegStr HKCR ".djvu\OpenWithProgids" "Djview.DjVuFile" ""
  WriteRegStr HKLM "${CLASSES}.djvu\OpenWithProgids" ${FILEID} ""
  ClearErrors
  ReadRegStr $TMP_EXT HKLM "${CLASSES}DjVu.Document" ""
  ${IfNot} ${Errors}
    WriteRegStr HKLM "${CLASSES}.djv\OpenWithProgids" "DjVu.Document" ""
    WriteRegStr HKLM "${CLASSES}.djvu\OpenWithProgids" "DjVu.Document" ""
  ${EndIf}
SectionEnd
;end reg app

;start reg ext
Section "$(secAssoc)" scAssoc
  DetailPrint "Creating associations"
  push ".djvu"
  Call RegisterExt
  push ".djv"
  Call RegisterExt
SectionEnd
;end reg ext
;start shortcuts
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
;  ${WriteUrlFile} "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\$(WEB_LNK) Djvu.org.url" "${DJVUORG_WEB_SITE}"
;  ${WriteUrlFile} "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\$(WEB_LNK) DjvuLibre.url" "${DJVULIBRE_WEB_SITE}"
;  ${WriteUrlFile} "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\${DJVULIBRE_NAME} $(WebDL_LNK).url" "${PRODUCT_DOWNLOAD_PAGE}"
;  ${WriteUrlFile} "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\On-line documentation.url" "${PRODUCT_ONLINEHELP_PAGE}"
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\$(WEB_LNK) Djvu.org.lnk" ${RUN_URL} "${URL_PAR} ${DJVUORG_WEB_SITE}" ${ICO_URL} 0
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\$(WEB_LNK) DjvuLibre.lnk" ${RUN_URL} "${URL_PAR} ${DJVULIBRE_WEB_SITE}" ${ICO_URL} 0
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\${DJVULIBRE_NAME} $(WebDL_LNK).lnk" ${RUN_URL} "${URL_PAR} ${PRODUCT_DOWNLOAD_PAGE}" ${ICO_URL} 0
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\On-line documentation.lnk" ${RUN_URL} "${URL_PAR} ${PRODUCT_ONLINEHELP_PAGE}" ${ICO_URL} 0


  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\$(Lic_TXT).lnk" "$INSTDIR\COPYING.txt"
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\DjVuLibre documentation.lnk" "$INSTDIR\djview.exe" '--outline --continuous "$INSTDIR\doc\djvulibre-book-en.djvu"'
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\Specification of the DjVu Format.lnk" "$INSTDIR\djview.exe" '--outline --continuous "$INSTDIR\doc\djvu3spec.djvu"'
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\Specification of the DjVu Format changes.lnk" "$INSTDIR\doc\djvuchanges.txt"
;new add
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\Specification of DjVu image compression format.lnk" "$INSTDIR\djview.exe" '--outline --continuous "$INSTDIR\doc\djvu2spec.djvu"'
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\Lizardtech License Notes 2002.lnk" "$INSTDIR\djview.exe" '--outline --continuous "$INSTDIR\doc\lizard2002.djvu"'
  CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\Lizardtech License Notes 2007.lnk" "$INSTDIR\djview.exe" '--outline --continuous "$INSTDIR\doc\lizard2007.djvu"'


  ${If} $Language == ${LANG_RUSSIAN}
    CreateShortCut "$SMPROGRAMS\${MENU_NAME}\$(Doc_DIR)\$(Doc_LNK) ${DJVULIBRE_NAME}.lnk" "$INSTDIR\djview.exe" '--outline --continuous "$INSTDIR\doc\djvulibre-book-ru.djvu"'
  ${EndIf}
;end new add
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

;end shortcuts


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
  ;protect install log
  Rename $INSTDIR\install.log $INSTDIR\uninstall.${PRODUCT_VERSION}
  SetFileAttributes $INSTDIR\uninstall.${PRODUCT_VERSION} \
  HIDDEN|READONLY|SYSTEM
${If} $INST_LOG_REN == "1"
  Rename $INSTDIR\install.kik $INSTDIR\install.log
${EndIf}

  DetailPrint "-----------------------------------------------------------------------------------------------"
  DetailPrint "Installer NSI V.${NSI_VER} by Kravtsov Konstantin Ivanovich, Novokuznetsk, Russia."
  DetailPrint "-----------------------------------------------------------------------------------------------"
  
SectionEnd



;------------
; Uninstaller 
;------------




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

Exch $R7 ;get  path+filename uninstall log
push $0 ;$0 - uninstall log file var
push $1 ; string readed from file
push $2 ; string cutted from $1
push $3 ; int - found str location by StrLoc
push $4 ; counter
push $5 ; extracted filename


  DetailPrint "Removing Files.."
;  StrCpy $R7 "$INSTDIR\install.log"
  ${If} ${FileExists} "$R7"
    ClearErrors
    FileOpen $0 "$R7" "r"
    ${Do}
      ClearErrors
      FileRead $0 $1
      ${If} ${Errors}
;check no file or EOF
         ${ExitDo}
      ${EndIf}
      StrCpy $2 $1 11
      ${If} $2 == "File: wrote"
        ${UnStrloc} $3 $1 " to " ">"
        Intop $3 $3 + 5
;        DetailPrint "file loc $3"
        StrCpy $5 $1 -3 $3

        Delete  /REBOOTOK "$5"
;        DetailPrint "Del file $5"
      ${ElseIf} $2 == "WriteINIStr"
        ${UnStrloc} $3 $1 " in " ">"
        Intop $3 $3 + 4
        StrCpy $5 $1 -2 $3
;        ${UnTrim} $6 $5
        Delete /REBOOTOK "$5"
;        DetailPrint "Removing url..$5"
      ${ElseIf} $2 == "CreateShort"
        ${UnStrloc} $3 $1 ', in: ' ">"
        Intop $3 $3 - 22
        StrCpy $5 $1 $3 22
        Delete /REBOOTOK "$5"
        DetailPrint "Removing ShrtC..$5"



      ${EndIf}
    ${Loop}
    SetDetailsPrint both
;DetailPrint "Removing Directories.."
;prepare file
    FileSeek $0 0
;set counter
    strcpy $4 0
   ${Do}
      ClearErrors
      FileRead $0 $1
      ${If} ${Errors}
;      DetailPrint "RmDir Abort on $1"

         ${ExitDo}
      ${EndIf}
      StrCpy $2 $1 -2 -10
;      detailprint "get $2"
       ;StrCpy $2 $3 8
;DetailPrint "S2=$2"

      ${If} $2 == " created"
        ${UnStrloc} $3 $1 " created" ">"
        Intop $3 $3 - 18
        StrCpy $5 $1 $3 18
       ; ${UnTrim} $6 $5

       push $5
;       detailprint "push $5"
       Intop $4 $4 + 1
      ${EndIf}

    ${Loop}

    ${ForEach} $COUNT $4 1 - 1
    POP $5
;       detailprint "POP $5 count=$COUNT"
    RmDir  /REBOOTOK "$5"

    ${Next}

    FileClose $0

  ${Else}
    ;; RMDir /r "$INSTDIR"  ; too bad
  ${EndIf}

pop $5 ; extracted filename
pop $4 ; counter
pop $3 ; int - found str location by StrLoc
pop $2 ; string cutted from $1
pop $1 ; string readed from file
pop $0 ;$0 - uninstall log file var
pop $R7 ; path+filename uninstall log

FunctionEnd


Function un.RegisterExt
; Kravtsov Konstantin Ivanovich (C) 2013
; restore assoc from uninst key
; check if restore really need
  Pop $Djvu_EXT
  ClearErrors
  ReadRegStr $TMP_EXT HKLM "${CLASSES}$Djvu_EXT" ""
  ${IfNot} ${Errors}
     ${If} "$TMP_EXT" == ${FILEID}
       ClearErrors
       ReadRegStr $TMP_EXT HKLM "${CLASSES}$Djvu_EXT" ${BACKUP_EXT}
       ${IfNot} ${Errors}
         ${If} "$TMP_EXT" == "${NO_EXT}"
           ; NBell had a complex loop to search alternatives
           ; But how to be sure that what the loop finds is legal
;NBell note:
;whatever do not created by this - leave as is - not our business
;assume 2 vals must be - default and backuped ext val
            ClearErrors
            EnumRegValue $TMP_EXT HKLM "${CLASSES}$Djvu_EXT" 2
;    DetailPrint "Enumed $TMP_EXT"
            ${If} ${Errors}
;no 3rd value
             DeleteRegKey HKLM "${CLASSES}$Djvu_EXT"
            ${Else}
;leave blank
;do not delete key - some other prog write info
             WriteRegStr HKLM "${CLASSES}$Djvu_EXT" "" ""

            ${EndIf}

         ${Else}
           WriteRegStr HKLM "${CLASSES}$Djvu_EXT" "" $TMP_EXT
         ${EndIf}
         DeleteRegValue HKLM "${CLASSES}$Djvu_EXT" ${BACKUP_EXT}
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
  SetFileAttributes $INSTDIR\uninstall.${PRODUCT_VERSION} NORMAL

  IfSilent +2
  SetDetailsPrint both
  ;; remove all files
  push $INSTDIR\uninstall.${PRODUCT_VERSION}
  Call un.RemoveFiles
  Delete "$INSTDIR\uninstall.${PRODUCT_VERSION}"
  Delete "$INSTDIR\uninst.exe"
;  Push "\"
;  Call un.RemoveEmptyDirsSub
  RMDir "$INSTDIR"
  ;; delete menu (save becauseee SMSPROGRAMS is well defined)
;  RmDir /r "$SMPROGRAMS\${MENU_NAME}"
  ;; delete shortcuts - unnessesary - removefiles delete all created any dir
;  Delete "$DESKTOP\DjView.lnk"
;  Delete "$QUICKLAUNCH\DjView.lnk"
  ;; clean registry
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  DeleteRegKey HKLM "${CLASSES}${FILEID}"
  DeleteRegKey HKLM "${CLASSES}Applications\djview.exe"
  DeleteRegValue HKLM "${CLASSES}.djvu\OpenWithProgIDs" "${FILEID}"
  DeleteRegValue HKLM "${CLASSES}.djv\OpenWithProgIDs" "${FILEID}"
;if no windjview key  - delete openwithprogid
  ClearErrors
  ReadRegStr $TMP_EXT HKCR "DjVu.Document" ""
  ${If} ${Errors}
  DeleteRegValue HKLM "${CLASSES}.djv\OpenWithProgids" "DjVu.Document"
  DeleteRegValue HKLM "${CLASSES}.djvu\OpenWithProgids" "DjVu.Document"
  ${EndIf}


  Push ".djvu"
  Call un.RegisterExt
  Push ".djv"
  Call un.RegisterExt
  ;; clean uninstall key
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
;  SetAutoClose true
  DetailPrint "-----------------------------------------------------------------------------------------------"
  DetailPrint "DjVuLibre installer script V.${NSI_VER} by Kravtsov Konstantin Ivanovich, Novokuznetsk, Russia."
  DetailPrint "-----------------------------------------------------------------------------------------------"
SectionEnd

!include "djvulibrelang-ru.nsh"
!include "djvulibrelang-en.nsh"

; Section descriptions set

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${scDjVuLibre} $(DESC_DjVuLibre)
!insertmacro MUI_DESCRIPTION_TEXT ${scDjView} $(DESC_DjView)
!insertmacro MUI_DESCRIPTION_TEXT ${scAssoc} $(DESC_Assoc)
!insertmacro MUI_DESCRIPTION_TEXT ${scShort} $(DESC_Short)
!insertmacro MUI_DESCRIPTION_TEXT ${scQuick} $(DESC_Quick)
!insertmacro MUI_DESCRIPTION_TEXT ${scDesk} $(DESC_Desk)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

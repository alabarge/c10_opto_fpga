;--------------------------------
;Include Modern UI

!include "MUI2.nsh"
!include "logiclib.nsh"

SetCompressor /SOLID lzma
BrandingText " "

; refresh document icons
; ie4uinit.exe -ClearIconCache
; ie4uinit.exe -show

;--------------------------------
;General

!define PRODUCT_NAME "DE0"
!define PRODUCT_EXT ".de0"
!define FILE_VERSION "1.4.1"
!define PRODUCT_PUBLISHER "Omniware"
!define PRODUCT_PUBLISHER_SHORT "Omniware"
!define PRODUCT_WEB_SITE "http://omniware.us"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\${PRODUCT_NAME}_${FILE_VERSION}.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}_${FILE_VERSION}"
!define PRODUCT_INST_CLASS "Software\Classes"

# These will be displayed by the "Click here for support information" link in "Add/Remove Programs"
# It is possible to use "mailto:" links in here to open the email client
!define HELPURL "http://omniware.us" # "Support Information" link
!define ABOUTURL "http://omniware.us" # "Publisher" link

Name "${PRODUCT_NAME} ${FILE_VERSION}"
OutFile "${PRODUCT_NAME}-${FILE_VERSION}-x64-setup.exe"
InstallDir "$PROGRAMFILES64\${PRODUCT_NAME}_${FILE_VERSION}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
DirText "Setup will install $(^Name) in the following folder.$\r$\n$\r$\nTo install in a different folder, click Browse and select another folder."
RequestExecutionLevel admin ;Require admin rights on NT6+ (When UAC is turned on)
ShowInstDetails show
ShowUnInstDetails show
CRCCheck on

;--------------------------------
;Interface Settings

!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "green_install.bmp"
!define MUI_COMPONENTSPAGE_NODESC
!define MUI_ICON "app.ico"
!define MUI_UNICON "app.ico"

!macro VerifyUserIsAdmin
UserInfo::GetAccountType
pop $0
${If} $0 != "admin" ;Require admin rights on NT4+
        messageBox mb_iconstop "Administrator rights required!"
        setErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
        quit
${EndIf}
!macroend

function .onInit
   setShellVarContext all
   !insertmacro VerifyUserIsAdmin
functionEnd

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Version Information

VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${PRODUCT_NAME} Application Installer"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${PRODUCT_NAME} Application"
VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" ""
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "${PRODUCT_PUBLISHER}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" ""
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Copyright (C) 2016 ${PRODUCT_PUBLISHER}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${FILE_VERSION}.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${FILE_VERSION}.0"

VIProductVersion "${FILE_VERSION}.0"

;--------------------------------
;Installer Sections

Section "!Program Files and DLLs" SEC01
  SectionIn RO
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "..\Release\${PRODUCT_NAME}.exe"
  File "..\Release\cmapi.dll"
  File "..\Release\comapi.dll"
  File "..\Release\optoapi.dll"
  File "..\Release\lanapi.dll"
  File "..\Release\uartapi.dll"
  File "..\Release\libspapi.dll"
  File "..\Release\ftd2xx64.dll"
  File "..\Release\PEGRP64F.DLL"
  File "appDoc.ico"
  File "History.txt"
  File "License.txt"
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}_${FILE_VERSION}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}_${FILE_VERSION}\Uninstall ${PRODUCT_NAME} ${FILE_VERSION}.lnk" "$INSTDIR\uninst.exe"
SectionEnd

SectionGroup /e "!Registry Items" SEC02
   Section "Desktop Shortcut and Start Menu Item" SEC03
     CreateShortCut "$DESKTOP\${PRODUCT_NAME} ${FILE_VERSION}.lnk" "$INSTDIR\${PRODUCT_NAME}.exe"
     CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}_${FILE_VERSION}\${PRODUCT_NAME} ${FILE_VERSION}.lnk" "$INSTDIR\${PRODUCT_NAME}.exe"
   SectionEnd
   Section "Remove Previous Program Items from Registry" SEC04
     DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
     DeleteRegKey HKCU "${PRODUCT_DIR_REGKEY}"
     DeleteRegKey HKCU "Software\${PRODUCT_PUBLISHER_SHORT}\${PRODUCT_NAME} ${FILE_VERSION}"
     DeleteRegKey HKCU "${PRODUCT_INST_CLASS}\${PRODUCT_NAME}${PRODUCT_EXT}"
     DeleteRegKey HKCR "${PRODUCT_INST_CLASS}\${PRODUCT_NAME}${PRODUCT_EXT}"
     DeleteRegKey HKCU "${PRODUCT_INST_CLASS}\${PRODUCT_EXT}"
   SectionEnd
SectionGroupEnd

SectionGroup "!Runtime Libraries" SEC05
   Section "Visual C++ 2022 Runtime Libraries, x64" SEC06
     SetOutPath "$INSTDIR"
     SetOverwrite ifnewer
     DetailPrint "Installing VC++ 2015 Runtime"
     File "vc_redist.x64.exe"
     ExecWait "vc_redist.x64.exe /q"
   SectionEnd
SectionGroupEnd

SectionGroup "!Communication Drivers" SEC08
   Section "FTDI 2.12.36 Driver" SEC09
     SetOverwrite ifnewer
     File /r "CDM21236"
   SectionEnd
   Section "Npcap 1.75" SEC010
     SetOutPath "$INSTDIR"
     SetOverwrite ifnewer
     DetailPrint "Installing Npcap 1.75"
     File "npcap-1.75.exe"
     ExecWait "npcap-1.75.exe"
   SectionEnd
SectionGroupEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\${PRODUCT_NAME}.exe"
  WriteRegStr HKCU "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr HKCU "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr HKCU "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\${PRODUCT_NAME}.exe"
  WriteRegStr HKCU "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${FILE_VERSION}"
  WriteRegStr HKCU "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr HKCU "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  ;File Type Association
  WriteRegStr HKCU "${PRODUCT_INST_CLASS}\${PRODUCT_NAME}${PRODUCT_EXT}\DefaultIcon" "" "$INSTDIR\appDoc.ico"
  WriteRegStr HKCR "${PRODUCT_INST_CLASS}\${PRODUCT_NAME}${PRODUCT_EXT}\DefaultIcon" "" "$INSTDIR\appDoc.ico"
  WriteRegStr HKCU "${PRODUCT_INST_CLASS}\${PRODUCT_NAME}${PRODUCT_EXT}\Shell\Open\Command" "" "$\"$INSTDIR\${PRODUCT_NAME}.exe$\" $\"%1$\""
  WriteRegStr HKCU "${PRODUCT_INST_CLASS}\${PRODUCT_EXT}" "" "${PRODUCT_NAME}${PRODUCT_EXT}"
SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SEC01 ${LANG_ENGLISH} "Executables and DLLs."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} $(DESC_SEC01)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

function un.onInit
   SetShellVarContext all

   #Verify the uninstaller - last chance to back out
   MessageBox MB_OKCANCEL "Permanantly remove ${PRODUCT_NAME}_${FILE_VERSION}?" IDOK next
      Abort
   next:
   !insertmacro VerifyUserIsAdmin
functionEnd

RequestExecutionLevel admin ;Require admin rights on NT6+ (When UAC is turned on)

Section Uninstall
  Delete "$DESKTOP\${PRODUCT_NAME} ${FILE_VERSION}.lnk"
  ExecWait "$INSTDIR\vc_redist.x64.exe /q /uninstall"
  RMDir /r "$INSTDIR"
  RMDir /r "$SMPROGRAMS\${PRODUCT_NAME}_${FILE_VERSION}"
  DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKCU "${PRODUCT_DIR_REGKEY}"
  DeleteRegKey HKCU "Software\${PRODUCT_PUBLISHER_SHORT}\${PRODUCT_NAME} ${FILE_VERSION}"
  DeleteRegKey HKCU "${PRODUCT_INST_CLASS}\${PRODUCT_NAME}${PRODUCT_EXT}"
  DeleteRegKey HKCU "${PRODUCT_INST_CLASS}\${PRODUCT_EXT}"
  DeleteRegKey HKCR "${PRODUCT_INST_CLASS}\${PRODUCT_EXT}"

  SetAutoClose false
SectionEnd

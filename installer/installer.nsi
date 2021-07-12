
# ---------------[TODO]---------------
# 		 Add "SAMP" folder
# ------------------------------------

# Unicode
Unicode true

# Includes
!include "MUI2.nsh"

# General
!searchparse /file "..\sdk\shared.h" `#define SAMP_VERSION "` VERSION `"`
Name "San Andreas Multiplayer ${VERSION}"
OutFile "sa-mp-${VERSION}-install.exe"
;InstallDirRegKey HKLM "Software\Rockstar Games\GTA San Andreas\Installation" ExePath
InstallDir "$PROGRAMFILES\Rockstar Games\GTA San Andreas"
AutoCloseWindow true
SetCompressor lzma
SetCompressorDictSize 8

# Interfaces
!define MUI_ABORTWARNING

# Pages
!define MUI_DIRECTORYPAGE_TEXT_TOP "Please select your Grand Theft Auto: San Andreas directory:"
!insertmacro MUI_PAGE_LICENSE "license.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

# Functions

Function .onVerifyInstDir

IfFileExists "$INSTDIR\gta_sa.exe" PathGood
Abort
		
PathGood:
FunctionEnd

# Sections

Section
	SetOutPath $INSTDIR
	File files\samp.exe
	File files\samp.dll
	File files\bass.dll
	File files\samp.saa
	File files\rcon.exe
	File files\samp_debug.exe
	File files\sampgui.png
	File files\mouse.png
	File files\gtaweap3.ttf
	File files\sampaux3.ttf
	File files\samp-license.txt
	/* Not yet used in SA-MP
	Delete $INSTDIR\models\samp.img
	Delete $INSTDIR\models\sampcol.img
	Delete $INSTDIR\models\samp.ide
	Delete $INSTDIR\models\samp.ipl
	SetOutPath $INSTDIR\SAMP
	File files\SAMP\SAMP.img
	File files\SAMP\SAMPCOL.img
	File files\SAMP\SAMP.ide
	File files\SAMP\SAMP.ipl
	File files\SAMP\samaps.txd
	File files\SAMP\blanktex.txd
	SetOverwrite off 
	File files\SAMP\custom.img
	File files\SAMP\CUSTOM.ide
	SetOverwrite on
	*/
	WriteUninstaller SAMPUninstall.exe
	CreateDirectory "$SMPROGRAMS\San Andreas Multiplayer"
	CreateShortCut "$SMPROGRAMS\San Andreas Multiplayer\San Andreas Multiplayer.lnk" $INSTDIR\samp.exe
	CreateShortCut "$SMPROGRAMS\San Andreas Multiplayer\Uninstall.lnk" $INSTDIR\SAMPUninstall.exe
	WriteRegStr HKCR samp "" "San Andreas Multiplayer"
	WriteRegStr HKCR samp "Url Protocol" ""
	WriteRegStr HKCR samp\shell\open\command "" "$\"$INSTDIR\samp.exe$\" $\"%1$\""
	WriteRegStr HKCU Software\SAMP gta_sa_exe $INSTDIR\gta_sa.exe
	CreateDirectory "$DOCUMENTS\GTA San Andreas User Files"
	CreateDirectory "$DOCUMENTS\GTA San Andreas User Files\SAMP"
	CopyFiles $INSTDIR\userdata.dat "$DOCUMENTS\GTA San Andreas User Files\SAMP"
	Delete $INSTDIR\userdata.dat
SectionEnd

Section "Uninstall"
	Delete $INSTDIR\samp.exe
	Delete $INSTDIR\samp.dll
	Delete $INSTDIR\bass.dll
	Delete $INSTDIR\samp.saa
	Delete $INSTDIR\rcon.exe
	Delete $INSTDIR\samp_debug.exe
	Delete $INSTDIR\SAMPUninstall.exe
	Delete $INSTDIR\sampgui.png
	Delete $INSTDIR\mouse.png
	Delete $INSTDIR\gtaweap3.ttf
	Delete $INSTDIR\sampaux3.ttf
	Delete $INSTDIR\samp-license.txt
	/* Not yet used in SA-MP
	Delete $INSTDIR\SAMP\samp.img
	Delete $INSTDIR\SAMP\sampcol.img	
	Delete $INSTDIR\SAMP\custom.img
	Delete $INSTDIR\SAMP\samp.ide
	Delete $INSTDIR\SAMP\samp.ipl
	Delete $INSTDIR\SAMP\custom.ide
	Delete $INSTDIR\SAMP\samaps.txd	
	Delete $INSTDIR\SAMP\blanktex.txd
	RMDir $INSTDIR\SAMP
	*/
	Delete "$SMPROGRAMS\San Andreas Multiplayer\San Andreas Multiplayer.lnk"
	Delete "$SMPROGRAMS\San Andreas Multiplayer\Uninstall.lnk"
	RMDir "$SMPROGRAMS\San Andreas Multiplayer"
SectionEnd
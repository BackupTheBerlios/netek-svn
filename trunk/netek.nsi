Name "neteK 0.8.1"
Outfile "netek-0.8.1.exe"
XPStyle on
InstallDir "$PROGRAMFILES\netek"

TODO COPYING.txt

Function uninstall
	ReadRegStr $0 HKLM "Software\neteK" "uninstaller"
	IfFileExists $0 0 continue
	Exec $0
	Abort
continue:
FunctionEnd

Page directory "" "" uninstall
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section Install
	SetShellVarContext all
	SetOutPath "$INSTDIR"
	File release\netek.exe
	File c:\qt\4.1.0\bin\mingwm10.dll
	WriteUninstaller "$INSTDIR\uninstaller.exe"
	WriteRegStr HKLM "Software\neteK" "uninstaller" "$INSTDIR\uninstaller.exe"

	CreateDirectory "$SMPROGRAMS\neteK"
	CreateShortCut "$SMPROGRAMS\neteK\neteK.lnk" "$INSTDIR\netek.exe"
	CreateShortCut "$SMPROGRAMS\neteK\Uninstall neteK.lnk" "$INSTDIR\uninstaller.exe"
SectionEnd

Section Uninstall
	SetShellVarContext all
	Delete "$INSTDIR\netek.exe"
	Delete "$INSTDIR\mingwm10.dll"
	Delete "$SMPROGRAMS\neteK\Uninstall neteK.lnk"
	Delete "$SMPROGRAMS\neteK\neteK.lnk"
	RMDir "$SMPROGRAMS\neteK"
	Delete "$INSTDIR\uninstaller.exe"
	DeleteRegKey HKLM "Software\neteK"
	RMDir "$INSTDIR"
SectionEnd

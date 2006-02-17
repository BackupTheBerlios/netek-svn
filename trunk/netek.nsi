Name "neteK 0.8.0"
Outfile "netek-0.8.0.exe"
InstallDir "$PROGRAMFILES\netek"

Function uninstall
	ReadRegStr $0 HKLM "Software\neteK" "uninstaller"
	IfFileExists $0 0 continue
	ExecWait $0
	Abort
continue:
FunctionEnd

Page directory "" "" uninstall
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section Install
	SetOutPath "$INSTDIR"
	File release\netek.exe
	File netek_trayicon.ico
	WriteUninstaller "$INSTDIR\uninstaller.exe"
	WriteRegStr HKLM "Software\neteK" "uninstaller" "$INSTDIR\uninstaller.exe"

	CreateDirectory "$SMPROGRAMS\neteK 0.8.0"
	CreateShortCut "$SMPROGRAMS\neteK 0.8.0\neteK.lnk" "$INSTDIR\netek.exe" "" "$INSTDIR\netek_trayicon.ico"
	CreateShortCut "$SMPROGRAMS\neteK 0.8.0\Uninstall.lnk" "$INSTDIR\uninstaller.exe"
SectionEnd

Section Uninstall
	Processes::KillProcess "netek.exe"
	DeleteRegKey HKLM "Software\neteK" 
	Delete "$SMPROGRAMS\neteK 0.8.0\Uninstall.lnk"
	Delete "$SMPROGRAMS\neteK 0.8.0\neteK.lnk"
	RMDir "$SMPROGRAMS\neteK 0.8.0"
	Delete "$INSTDIR\uninstaller.exe"
	Delete "$INSTDIR\netek.exe"
	Delete "$INSTDIR\netek_trayicon.ico"
	RMDir "$INSTDIR"
SectionEnd

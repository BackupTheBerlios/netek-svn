Name "neteK 0.8.0"
Outfile "netek-0.8.0.exe"
InstallDir "$PROGRAMFILES\netek"
Icon icons\netek.ico
Uninstallicon icons\netek.ico

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
	SetOutPath "$INSTDIR"
	File release\netek.exe
	File icons\netek.ico
	WriteUninstaller "$INSTDIR\uninstaller.exe"
	WriteRegStr HKLM "Software\neteK" "uninstaller" "$INSTDIR\uninstaller.exe"

	CreateDirectory "$SMPROGRAMS\neteK"
	CreateShortCut "$SMPROGRAMS\neteK\neteK.lnk" "$INSTDIR\netek.exe" "" "$INSTDIR\netek.ico"
	CreateShortCut "$SMPROGRAMS\neteK\Uninstall.lnk" "$INSTDIR\uninstaller.exe"
SectionEnd

Section Uninstall
	Delete "$INSTDIR\netek.exe"
	Delete "$INSTDIR\netek.ico"
	Delete "$SMPROGRAMS\neteK\Uninstall.lnk"
	Delete "$SMPROGRAMS\neteK\neteK.lnk"
	RMDir "$SMPROGRAMS\neteK"
	Delete "$INSTDIR\uninstaller.exe"
	DeleteRegKey HKLM "Software\neteK"
	RMDir "$INSTDIR"
SectionEnd

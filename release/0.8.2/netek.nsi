Name "neteK 0.8.2"
Outfile "netek-0.8.2.exe"
XPStyle on
InstallDir "$PROGRAMFILES\netek"

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
	File f:\qt\4.1.0\bin\mingwm10.dll
	File /oname=COPYING.txt COPYING
	WriteUninstaller "$INSTDIR\uninstaller.exe"
	WriteRegStr HKLM "Software\neteK" "uninstaller" "$INSTDIR\uninstaller.exe"
	WriteRegStr HKCR "*\shell\neteK" "" "Create share with neteK..."
	WriteRegStr HKCR "*\shell\neteK\command" "" '"$INSTDIR\netek.exe" createShare "%1"'
	WriteRegStr HKCR "Folder\shell\neteK" "" "Create share with neteK..."
	WriteRegStr HKCR "Folder\shell\neteK\command" "" '"$INSTDIR\netek.exe" createShare "%1"'
	
	CreateDirectory "$SMPROGRAMS\neteK"
	CreateShortCut "$SMPROGRAMS\neteK\neteK.lnk" "$INSTDIR\netek.exe"
	CreateShortCut "$SMPROGRAMS\neteK\Uninstall neteK.lnk" "$INSTDIR\uninstaller.exe"
SectionEnd

Section Uninstall
	SetShellVarContext all
	Delete "$INSTDIR\netek.exe"
	Delete "$INSTDIR\mingwm10.dll"
	Delete "$INSTDIR\COPYING.txt"
	Delete "$SMPROGRAMS\neteK\Uninstall neteK.lnk"
	Delete "$SMPROGRAMS\neteK\neteK.lnk"
	RMDir "$SMPROGRAMS\neteK"
	DeleteRegKey HKCR "*\shell\neteK"
	DeleteRegKey HKCR "Folder\shell\neteK"
	Delete "$INSTDIR\uninstaller.exe"
	DeleteRegKey HKLM "Software\neteK"
	RMDir "$INSTDIR"
SectionEnd

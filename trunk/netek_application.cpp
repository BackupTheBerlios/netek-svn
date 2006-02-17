#include "netek_application.h"

#ifdef Q_OS_UNIX
static QStringList autostarts()
{
	QStringList list;
	list.append(QDir::home().filePath(".kde/Autostart/netek"));
	// TODO: add gnome autostart

	return list;
}
#endif

#ifdef Q_OS_WIN32
#include <windows.h>
static const wchar_t *regkey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const wchar_t *subkey = L"netek";
#endif

neteK::Application::Application(int &argc, char **argv)
: QApplication(argc, argv)
{
	setOrganizationName("neteK");
	setApplicationName("neteK");
	setWindowIcon(QIcon(":/icons/netek.png"));
	setQuitOnLastWindowClosed(false);

#ifdef Q_OS_UNIX
	foreach(QString path, autostarts()) {
		QFile(path).remove();
		QFile(applicationFilePath()).link(path);
	}
#endif

#ifdef Q_OS_WIN32
    HKEY key;
    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, regkey, 0, KEY_WRITE, &key)) {
        std::wstring str(applicationFilePath().toStdWString());
        RegSetValueEx(key, subkey, 0, REG_SZ, (const BYTE*)str.c_str(), str.size() * sizeof(wchar_t));
        RegCloseKey(key);
    }
#endif
}

void neteK::Application::userQuit()
{
#ifdef Q_OS_UNIX
	foreach(QString path, autostarts())
		QFile(path).remove();
#endif

#ifdef Q_OS_WIN32
    HKEY key;
    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, regkey, 0, KEY_WRITE, &key)) {
        RegDeleteValue(key, subkey);
        RegCloseKey(key);
    }
#endif

	quit();
}

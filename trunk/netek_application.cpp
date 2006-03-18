#include "netek_application.h"

#ifdef Q_OS_UNIX
static const char kde_autostart[] = ".kde/Autostart/netek";
static const char gnome_autostart[] = ".config/autostart/netek.desktop";
#endif

#ifdef Q_OS_WIN32
#include <windows.h>
static const wchar_t regkey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const wchar_t subkey[] = L"netek";
#endif

neteK::Application::Application(int &argc, char **argv)
: QApplication(argc, argv)
{
	setOrganizationName("neteK");
	setApplicationName("neteK");
	setWindowIcon(QIcon(":/icons/netek.png"));
	setQuitOnLastWindowClosed(false);

#ifdef Q_OS_UNIX
	setStyle(new QPlastiqueStyle);

	{
		QString path = QDir::home().filePath(kde_autostart);
		QFile(path).remove();
		QFile(applicationFilePath()).link(path);
	}

	{
		QFile dfile(QDir::home().filePath(gnome_autostart));
		if(dfile.open(QIODevice::WriteOnly))
			dfile.write(QString("[Desktop Entry]\nType=Application\nEncoding=UTF-8\nName=neteK\nExec=%1\n")
					.arg(applicationFilePath()).toUtf8());

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
	QFile(QDir::home().filePath(kde_autostart)).remove();
	QFile(QDir::home().filePath(gnome_autostart)).remove();
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

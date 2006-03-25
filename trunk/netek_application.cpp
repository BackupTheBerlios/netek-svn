#include "netek_application.h"
#include "netek_settings.h"

// TODO: proper app data...

#ifdef Q_OS_UNIX
static const char kde_autostart[] = ".kde/Autostart/netek";
static const char gnome_autostart[] = ".config/autostart/netek.desktop";
static const char app_data[] = ".netek";
#endif

#ifdef Q_OS_WIN32
#include <windows.h>
static const wchar_t regkey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const wchar_t subkey[] = L"netek";
static const char app_data[] = "Application Data/netek";
#endif

QDir neteK::Application::applicationData()
{ return QDir(QDir::home().absoluteFilePath(app_data)); }

QString neteK::Application::applicationVersion()
{ return "0.8.1"; }

neteK::Application::Application(int &argc, char **argv)
: QApplication(argc, argv)
{
	setOrganizationName("neteK");
	setApplicationName("neteK");
	setWindowIcon(QIcon(":/icons/netek.png"));
	setQuitOnLastWindowClosed(false);
	
	QDir::home().mkpath(app_data);

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

neteK::Log *neteK::Application::g_log = 0;

neteK::Log *neteK::Application::log()
{
	if(!g_log)
		g_log = new Log;
	return g_log;
}

neteK::Log::Log()
: m_file(Application::applicationData().filePath("log.txt"))
{ }

void neteK::Log::logLine(QString line)
{
	if(line.size() == 0)
		return;
		
	Settings settings;
	
	qint64 max = Settings().logKBytes() * 1024;
	
	QString txt = QString("%1 [%2] %3\n")
		.arg(QDateTime::currentDateTime().toString(Qt::LocalDate))
		.arg(Application::applicationVersion())
		.arg(line);
				
	emit appendToLog(txt);
	
	QFile f(m_file);
	if(max == 0)
		f.remove();
	else {
		if(f.open(QIODevice::Append)) {
			f.write(txt.toUtf8());
			f.close();		
		}
		
		if(f.size() > settings.logKBytes() * 3 / 2) {
			QString log = readLog();
			if(f.open(QIODevice::WriteOnly)) {
				f.write(log.toUtf8());
				f.close();
			}
		}
	}
}

QString neteK::Log::readLog()
{
	qint64 max = Settings().logKBytes() * 1024;
	
	QFile f(m_file);
	if(f.open(QIODevice::ReadOnly)) {
		if(f.size() > max)
			f.seek(f.size() - max);
		return QString::fromUtf8(f.readAll());
	}
	
	return QString();
}

neteK::ObjectLog::ObjectLog(QObject *attachto, QString start, QString stop)
: QObject(attachto), m_stop(stop)
{
	Application::log()->logLine(start);
}

neteK::ObjectLog::~ObjectLog()
{
	Application::log()->logLine(m_stop);
}

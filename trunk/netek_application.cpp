// This file is part of neteK project http://netek.berlios.de
// Copyright (C) 2005-2006 Egon Kocjan <egon.kocjan@xlab.si>
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "netek_application.h"
#include "netek_settings.h"
#include "netek_netutils.h"

#ifdef Q_OS_UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <cerrno>
static const char g_app_data[] = ".netek";
#endif

#ifdef Q_WS_X11
#include <QX11Info>
static const char g_kde_autostart[] = ".kde/Autostart/netek";
static const char g_gnome_autostart[] = ".config/autostart/netek.desktop";
 // TODO: use nautilus actions (when it gets installed on all distros by default)
static const char g_nautilus_scripts[] = ".gnome2/nautilus-scripts/neteK";
#endif

#ifdef Q_OS_WIN
#include <windows.h>
static const wchar_t g_regkey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const wchar_t g_subkey[] = L"netek";
static const char g_app_data[] = "Application Data/netek";
#endif

namespace neteK {

#ifdef Q_OS_WIN
class IPC: public QWidget {
	Q_OBJECT;
	
	static QString name()
	{ return QString("%1 IPC").arg(qApp->applicationName()); }
	
signals:
	void commandReceived(QStringList args);

public:
	IPC(QObject *p)
	{
		connect(p, SIGNAL(destroyed()), SLOT(deleteLater()));
		setWindowTitle(name());
	}
	
	bool winEvent(MSG *message, long *result)
	{
		if(message->message == WM_COPYDATA) {
			COPYDATASTRUCT *cds = (COPYDATASTRUCT*)message->lParam;
			qDebug() << "Received" << cds->cbData << "bytes";
			
			QByteArray in((char*)cds->lpData, cds->cbData);
			QDataStream data(&in, QIODevice::ReadOnly);
			QStringList args;
			data >> args;
			emit commandReceived(args);

			*result = TRUE;
			return true;
		}
		
		return QWidget::winEvent(message, result);
	}
	
	static bool sendCommand(QStringList args)
	{
		HWND wnd = FindWindow(0, name().toStdWString().c_str());
		if(wnd) {
			qDebug() << "Found IPC window:" << wnd;
			
			QByteArray out;
			QDataStream data(&out, QIODevice::WriteOnly);
			data << args;
			
			COPYDATASTRUCT cds;
			ZeroMemory(&cds, sizeof(cds));
			cds.lpData = out.data();
			cds.cbData = out.size();
			return SendMessage(wnd, WM_COPYDATA, 0, (LPARAM)&cds);
		}
		return false;
	}
};
#endif

#ifdef Q_OS_UNIX
class IPC: public QObject {
	Q_OBJECT;
	
	static bool init(bool server, int &sock)
	{
		sockaddr_un addr;
		socklen_t addr_len;

		QByteArray path = Application::applicationData().filePath("cmd").toUtf8();
		size_t psize = qMin((size_t)path.size(), sizeof(addr.sun_path));
		
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		memcpy(addr.sun_path, path.data(), psize);
		
		addr_len = sizeof(addr) - sizeof(addr.sun_path) + psize;
		
		int nblk = 1;
		
		return -1 != (sock = ::socket(PF_UNIX, SOCK_STREAM, 0))
			&& (server
				? 0 == ::ioctl(sock, FIONBIO, &nblk)
					&& (::unlink(path.data()), true)
					&& 0 == ::bind(sock, (sockaddr*)&addr, addr_len)
					&& 0 == ::listen(sock, 20)
				: 0 == ::connect(sock, (sockaddr*)&addr, addr_len));
	}
	
	int m_sock;
	
signals:
	void commandReceived(QStringList args);
	
private slots:
	void accept(int)
	{
		int sock = ::accept(m_sock, 0, 0);
		if(sock != -1) {
			qDebug() << "IPC client connected";
			QByteArray in;
			for(;;) {
				char buf[networkBufferSize];
				ssize_t read = ::read(sock, buf, sizeof(buf));
				if(read <= 0) {
					if(read == 0) {
						QDataStream data(&in, QIODevice::ReadOnly);
						QStringList args;
						data >> args;
						emit commandReceived(args);
					} else
						qWarning() << "IPC read error:" << strerror(errno);
					break;
				} else
					in.append(QByteArray(buf, read));
			}
			
			::close(sock);
		}
	}
	
public:
	IPC(QObject *p)
	: QObject(p), m_sock(-1)
	{
		if(init(true, m_sock)) {
			QPointer<QSocketNotifier> n = new QSocketNotifier(m_sock, QSocketNotifier::Read, this);
			connect(n, SIGNAL(activated(int)), SLOT(accept(int)));
			n->setEnabled(true);
		} else {
			qWarning() << "IPC server failed to start:" << strerror(errno);
			deleteLater();
		}
	}
	
	~IPC()
	{
		if(m_sock != -1)
			::close(m_sock);
	}
	
	static bool sendCommand(QStringList args)
	{
		bool ret = false;
		
		int sock;
		if(init(false, sock)) {
			QByteArray out;
			QDataStream data(&out, QIODevice::WriteOnly);
			data << args;
			
			ret = out.size() == ::write(sock, out.data(), out.size());
		}
		
		if(sock != -1 && ::close(sock) != 0)
			ret = false;
		
		return ret;
	}
};
#endif

}

QDir neteK::Application::applicationData()
{ return QDir(QDir::home().absoluteFilePath(g_app_data)); }

QString neteK::Application::applicationVersion()
{ return "0.8.2"; }

neteK::Application::Application(int &argc, char **argv)
: QApplication(argc, argv)
{
	setOrganizationName("neteK");
	setApplicationName("neteK");
	setQuitOnLastWindowClosed(false);

	connect(this, SIGNAL(processCommandsSignal()), SLOT(processCommandsSlot()), Qt::QueuedConnection);
	
	QDir::home().mkpath(g_app_data);
	
	{
		QStringList args = arguments();
		if(args.size() >= 1)
			args.pop_front();
			
		if(IPC::sendCommand(args))
			::exit(0);
		else
			processCommand(args);
	}
	
	setWindowIcon(QIcon(":/icons/netek.png"));

#ifdef Q_WS_X11
	setStyle(new QPlastiqueStyle); // TODO: color is sometimes b0rked under GNOME

	{
		QString path = QDir::home().filePath(g_kde_autostart);
		QFile(path).remove();
		QFile(applicationFilePath()).link(path);
	}

	{
		QFile dfile(QDir::home().filePath(g_gnome_autostart));
		if(dfile.open(QIODevice::WriteOnly))
			dfile.write(QString("[Desktop Entry]\nType=Application\nEncoding=UTF-8\nName=neteK\nExec=%1\n")
					.arg(applicationFilePath()).toUtf8());

	}

	{
		QDir::home().mkpath(g_nautilus_scripts);
		QFile s(QDir(QDir::home().filePath(g_nautilus_scripts)).filePath("Create share..."));
		if(s.open(QIODevice::WriteOnly)) {
			s.write(QString("#! /bin/sh\necho \"$NAUTILUS_SCRIPT_SELECTED_FILE_PATHS\" | while read x; do (%1 createShare \"$x\" &); exit; done\n")
					.arg(applicationFilePath()).toUtf8());
			s.setPermissions(s.permissions() | QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther);
		}
	}
#endif

#ifdef Q_OS_WIN
	HKEY key;
	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, g_regkey, 0, KEY_WRITE, &key)) {
		std::wstring str(applicationFilePath().toStdWString());
		RegSetValueEx(key, g_subkey, 0, REG_SZ, (const BYTE*)str.c_str(), str.size() * sizeof(wchar_t));
		RegCloseKey(key);
	}
#endif

	connect(new IPC(this), SIGNAL(commandReceived(QStringList)), SLOT(processCommand(QStringList)));
}

void neteK::Application::processCommand(QStringList cmd)
{
	m_commands.append(cmd);
	emit processCommandsSignal();
}

void neteK::Application::processCommandsSlot()
{
#ifdef Q_WS_X11
	// This will hopefully disable X11 WMs' focus stealing prevention.
	// We are opening dialogs over IPC, so WM can't relate user actions.
	QX11Info::setAppTime(0);
	QX11Info::setAppUserTime(0);
#endif

	while(m_commands.size()) {
		QStringList args = m_commands.front();
		m_commands.pop_front();

		qDebug() << "Processing command:" << args;
		
		if(args.size() == 0)
			continue;
			
		QString cmd = args.front();
		
		if(cmd == "createShare" && args.size() >= 2)
			emit command_createShare(args.at(1));
	}
}

void neteK::Application::userQuit()
{
#ifdef Q_WS_X11
	QFile(QDir::home().filePath(g_kde_autostart)).remove();
	QFile(QDir::home().filePath(g_gnome_autostart)).remove();
#endif

#ifdef Q_OS_WIN
	HKEY key;
	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, g_regkey, 0, KEY_WRITE, &key)) {
		RegDeleteValue(key, g_subkey);
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

void neteK::Log::clearLog()
{
	QFile(m_file).remove();
}

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
		if(f.open(QIODevice::Append | QIODevice::Text)) {
			f.write(txt.toUtf8());
			f.close();
		}

		if(f.size() > settings.logKBytes() * 3 / 2) {
			QString log = readLog();
			if(f.open(QIODevice::WriteOnly | QIODevice::Text)) {
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
	if(f.open(QIODevice::ReadOnly | QIODevice::Text)) {
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

#include "netek_application.moc"

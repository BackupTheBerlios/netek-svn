#include "netek_share.h"
#include "netek_settings.h"
#include "netek_sharesettings.h"
#include "netek_ftphandler.h"
#include "netek_netutils.h"

neteK::Share::Share()
: m_port(0), m_run(false), m_readonly(true), m_access(AccessAnonymous), m_client_count(0)
{ }

QString neteK::Share::URLProtocol() const
{
	return "ftp";
}

bool neteK::Share::readOnly() const
{
	return m_readonly;
}

void neteK::Share::setReadOnly(bool yes)
{
	m_readonly = yes;
}

neteK::Share::Access neteK::Share::access() const
{
	return m_access;
}

void neteK::Share::setAccess(Access a)
{
	m_access = a;
}

void neteK::Share::setUsernamePassword(QString u, QString p)
{
	m_username = u;
	m_password = p;
}

void neteK::Share::usernamePassword(QString &u, QString &p) const
{
	u = m_username;
	p = m_password;
}

bool neteK::Share::authenticate(QString username, QString password) const
{
	if(m_access == AccessAnonymous && username == "anonymous")
		return true;

	if(m_access == AccessUsernamePassword)
		return m_username.size() && m_password.size() && m_username == username && m_password == password;

	return false;
}

void neteK::Share::setFolder(QString folder)
{ m_folder = folder; }

void neteK::Share::setPort(quint16 port)
{ m_port = port; }

QString neteK::Share::niceId() const
{
	return QString("%1:%2").arg(m_folder).arg(m_port);
}

QString neteK::Share::folder() const
{ return m_folder; }

quint16 neteK::Share::port() const
{ return m_port; }

int neteK::Share::clients() const
{
	return m_client_count;
}

void neteK::Share::handleNewClient()
{
	while(m_server) {
		QPointer<QTcpSocket> client = m_server->nextPendingConnection();
		if(!client)
			break;

		++m_client_count;
		connect(client, SIGNAL(destroyed()), SLOT(clientGone()));
		
		QPointer<FtpHandler> handler = new FtpHandler(this, client);
		handler->setParent(m_server);

		{
			QHostAddress local = client->localAddress();
			
			if(!isPublicNetwork(local) && isPublicNetwork(client->peerAddress()))
				resolvePublicAddress(local, handler, SLOT(start(QHostAddress)));
			else
				handler->start(local);
		}
			
		emit statusChanged();
	}
}

void neteK::Share::clientGone()
{
	--m_client_count;
	emit statusChanged();
}

void neteK::Share::checkServer()
{
	if(m_run && !m_server && status() != StatusUnconfigured) {
		m_server = new QTcpServer(this);
		connect(m_server, SIGNAL(newConnection()), SLOT(handleNewClient()));

		if(!m_server->listen(QHostAddress::Any, m_port)) {
			m_server->deleteLater();
			m_server = 0;
			QTimer::singleShot(1000, this, SLOT(checkServer()));
		}
	} else if(!m_run && m_server) {
		m_server->deleteLater();
		m_server = 0;
	}

	emit statusChanged();
}

void neteK::Share::restartIfStarted()
{
	if(m_run && m_server) {
		m_server->deleteLater();
		m_server = 0;
	}

	checkServer();
}

void neteK::Share::startIfStopped()
{
	if(!m_run) {
		m_run = true;
		emit settingsChanged();
	}

	checkServer();
}

void neteK::Share::stop()
{
	if(m_run) {
		m_run = false;
		emit settingsChanged();
	}

	checkServer();
}

bool neteK::Share::runStatus() const
{ return m_run; }

void neteK::Share::setRunStatus(bool run)
{ m_run = run; }

neteK::Share::Status neteK::Share::status() const
{
	if(m_port == 0 || m_folder.size() == 0)
		return StatusUnconfigured;

	if(m_run && m_server)
		return StatusStarted;

	if(!m_run && !m_server)
		return StatusStopped;

	return StatusChanging;
}

bool neteK::Share::showSettings()
{
	if(ShareSettings(this).exec() == ShareSettings::Accepted) {
		restartIfStarted();
		emit settingsChanged();
		return true;
	}

	return false;
}

QString neteK::Share::initialFolder() const
{ return "/"; }

bool neteK::Share::changeCurrentFolder(QString cwd, QString path, QString &newcwd) const
{
	QString fspath;
	if(filesystemPath(cwd, path, fspath) && resolvePath(cwd, path, path)) {
		QFileInfo info(fspath);
		if(info.isDir() && info.isReadable()) {
			newcwd = path;
			return true;
		}
	}
	
	return false;
}

bool neteK::Share::rename(QString cwd, QString path1, QString path2) const
{
	if(m_readonly)
		return 0;
		
	return filesystemPathNotRoot(cwd, path1, path1)
		&& filesystemPathNotRoot(cwd, path2, path2)
		&& !(path2+'/').startsWith(path1+'/') // this prevents Qt from going into infinite loop
		&& QFile(path1).rename(path2);
}

bool neteK::Share::fileInformation(QString cwd, QString path, QFileInfo &info) const
{
	if(filesystemPath(cwd, path, path)) {
		info = QFileInfo(path);
		return info.exists();
	}

	return false;
}

bool neteK::Share::listFolder(QString cwd, QString path, QFileInfoList &list) const
{
	if(filesystemPath(cwd, path, path)) {
		QFileInfo info = QFileInfo(path);
		if(info.isDir() && info.isReadable()) {
			list = QDir(path).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);		
			return true;
		}
	}
	
	return false;
}

bool neteK::Share::createFolder(QString cwd, QString path) const
{
	if(m_readonly)
		return 0;

	return filesystemPathNotRoot(cwd, path, path) && QDir().mkdir(path);
}

bool neteK::Share::deleteFolder(QString cwd, QString path) const
{
	if(m_readonly)
		return 0;

	return filesystemPathNotRoot(cwd, path, path) && QDir().rmdir(path);
}

QFile *neteK::Share::readFile(QString cwd, QString path, qint64 pos) const
{
	if(filesystemPathNotRoot(cwd, path, path)) {
		QPointer<QFile> file(new QFile(path));
		if(file->open(QIODevice::ReadOnly)
			&& pos >= 0 && pos <= file->size()
			&& (pos == 0 || file->seek(pos)))
			return file;
			
		delete file;
	}
	
	return 0;
}

QFile *neteK::Share::writeFile(QString cwd, QString path, bool append) const
{
	if(m_readonly)
		return 0;
		
	if(filesystemPathNotRoot(cwd, path, path)) {
		QPointer<QFile> file(new QFile(path));
		if(file->open(append ? QIODevice::Append : QIODevice::WriteOnly))
			return file;
			
		delete file;
	}
	
	return 0;
}

QFile *neteK::Share::writeFileUnique(QString cwd, QString &fname) const
{
	if(m_readonly)
		return 0;

	QString path;
	if(filesystemPath(cwd, "", path)) {
		QString prefix(QString("ftp_%1_%2").arg(time(0)).arg(rand() & 0xffff));
		for(int i=0; i<100; ++i) {
			fname = QString("%1_%2").arg(prefix).arg(i);
			QPointer<QFile> file(new QFile(QDir(path).filePath(fname)));
	
			if(!file->exists() && file->open(QIODevice::WriteOnly))
				return file;
	
			delete file;
		}
	}

	return 0;
}

bool neteK::Share::deleteFile(QString cwd, QString path) const
{
	if(m_readonly)
		return 0;

	return filesystemPathNotRoot(cwd, path, path) && QFile(path).remove();
}

bool neteK::Share::resolvePath(QString cwd, QString path, QString &resolved) const
{
	cwd.replace('\\', '/');
	path.replace('\\', '/');
	
	if(!path.startsWith("/"))
		path = cwd + "/" + path;
	
	QStringList current;
	QStringList change = path.split('/');
	foreach(QString c, change)
		if(c == "." || c == "")
			;
		else if(c == "..") {
			if(current.size() == 0)
				return false;
			current.removeLast();
		} else
			current.append(c);
	
	resolved = "/";
	resolved += current.join("/");
	
	qDebug() << "=== Resolved path:" << resolved;
	
	return true;
}

bool neteK::Share::filesystemPath(QString cwd, QString path, QString &fspath, bool notroot) const
{
	// TODO: this needs to be checked for security a bit more
	
	QString root = QDir(m_folder).canonicalPath();
	if(root.size() == 0)
		return false;
	root.replace('\\', '/');
		
	QString resolved;
	if(!resolvePath(cwd, path, resolved))
		return false;
		
	if(notroot && resolved == "/")
		return false;
		
	fspath = QDir(root).filePath(resolved.mid(1));
	fspath.replace('\\', '/');
	
	qDebug() << "--- Filesystem path:" << fspath;
	
	return true;
}

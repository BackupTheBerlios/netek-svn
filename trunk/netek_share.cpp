#include "netek_share.h"
#include "netek_settings.h"
#include "netek_sharesettings.h"
#include "netek_ftphandler.h"

neteK::Share::Share()
: m_port(0), m_run(false), m_readonly(true), m_access(AccessAnonymous), m_client_count(0)
{
	{
		QTimer *timer = new QTimer(this);
		connect(timer, SIGNAL(timeout()), SLOT(checkServer()));
		timer->start(10000);
	}

	connect(this, SIGNAL(recheckServer()), SLOT(checkServer()), Qt::QueuedConnection);

	emit recheckServer();
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
	if(m_server) {
		QPointer<FtpHandler> handler =
			new FtpHandler(this, m_server->nextPendingConnection());
		handler->setParent(this);
		connect(handler, SIGNAL(destroyed()), SLOT(clientGone()));
		connect(m_server, SIGNAL(destroyed()), handler, SLOT(deleteLater()));

		++m_client_count;
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

	emit recheckServer();
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

bool neteK::Share::forbiddenName(QString name)
{ return name.size() == 0 || name == "." || name == ".." || name == "/" || name == "\\"; }

bool neteK::Share::resolveDir(QStringList path, QDir &dir) const
{
	dir.setPath(m_folder);
	foreach(QString p, path) {
		if(forbiddenName(p))
			return false;

		if(!dir.cd(p))
			return false;
	}

	qDebug() << "=== Resolving directory:" << dir.path();

	return dir.isReadable();
}

bool neteK::Share::resolveDirName(QStringList path, QDir &dir, QString &name) const
{
	if(path.size()) {
		name = path.last();
		if(forbiddenName(name))
			return false;

		qDebug() << "=== Resolving directory entry:" << name;

		path.removeLast();
		return resolveDir(path, dir);
	}

	return false;
}

bool neteK::Share::resolveFile(QStringList path, QFile &file) const
{
	if(path.size()) {
		QString name = path.last();
		if(forbiddenName(name))
			return false;

		qDebug() << "=== Resolving file:" << name;

		path.removeLast();

		QDir dir;
		if(!resolveDir(path, dir))
			return false;

		file.setFileName(dir.filePath(name));

		return true;
	}

	return false;
}

bool neteK::Share::isFolderReadable(QStringList path) const
{
	QDir dir;
	return resolveDir(path, dir);
}

bool neteK::Share::fileExists(QStringList path) const
{
	QFile file;
	return resolveFile(path, file) && file.exists();
}

bool neteK::Share::rename(QStringList path1, QStringList path2) const
{
    if(m_readonly)
		return 0;

	// TODO: what if directory is moved into its child directory
	QDir dir1, dir2;
	QString name1, name2;
	return path1.size()
		&& resolveDirName(path1, dir1, name1)
		&& resolveDirName(path2, dir2, name2)
		&& dir1.rename(name1, dir2.filePath(name2));
}

bool neteK::Share::pathInformation(QStringList path, QFileInfo &info) const
{
	QDir dir;
	QFile file;
	if(resolveDir(path, dir)) {
		info = QFileInfo(dir.absolutePath());
		return true;
	} else if(resolveFile(path, file) && file.exists()) {
		info = QFileInfo(file);
		return true;
	}

	return false;
}

bool neteK::Share::pathInformationList(QStringList path, QFileInfoList &list) const
{
	QDir dir;
	QFile file;
	if(resolveDir(path, dir)) {
		QFileInfoList tmp = dir.entryInfoList();
		foreach(QFileInfo i, tmp)
			if(!forbiddenName(i.fileName()))
				list.append(i);

		return true;
	} else if(resolveFile(path, file) && file.exists()) {
		list.append(QFileInfo(file));
		return true;
	}

	return false;
}

bool neteK::Share::createFolder(QStringList path) const
{
    if(m_readonly)
		return 0;

	QDir dir;
	QString name;
	return resolveDirName(path, dir, name) && dir.mkdir(name);
}

bool neteK::Share::deleteFolder(QStringList path) const
{
    if(m_readonly)
		return 0;

	QDir dir;
	QString name;
	return resolveDirName(path, dir, name) && dir.rmdir(name);
}

QFile *neteK::Share::readFile(QStringList path, qint64 pos) const
{
	QPointer<QFile> file(new QFile);
	if(!resolveFile(path, *file)
			|| !file->open(QIODevice::ReadOnly)
			|| pos < 0
			|| pos > file->size()
			|| pos != 0 && !file->seek(pos))
	{
		delete file;
		return 0;
	}

	return file;
}

QFile *neteK::Share::writeFile(QStringList path, bool append) const
{
	if(m_readonly)
		return 0;

	QPointer<QFile> file(new QFile);
	if(!resolveFile(path, *file)
			|| !file->open(append ? QIODevice::Append : QIODevice::WriteOnly))
	{
		delete file;
		return 0;
	}

	return file;
}

QFile *neteK::Share::writeFileUnique(QStringList path, QString &fname) const
{
	if(m_readonly)
		return 0;

	QDir dir;
	if(!resolveDir(path, dir))
		return 0;

	QString prefix(QString("ftp_%1_%2").arg(time(0)).arg(rand() & 0xffff));
	for(int i=0; i<100; ++i) {
		fname = QString("%1_%2").arg(prefix).arg(i);
		QPointer<QFile> file(new QFile);
		file->setFileName(dir.filePath(fname));

		if(!file->exists() && file->open(QIODevice::WriteOnly))
			return file;

		delete file;
	}

	return 0;
}

bool neteK::Share::deleteFile(QStringList path) const
{
    if(m_readonly)
		return 0;

	QFile file;
	return resolveFile(path, file) && file.remove();
}

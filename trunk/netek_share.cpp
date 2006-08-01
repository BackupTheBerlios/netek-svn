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

#include "netek_share.h"
#include "netek_settings.h"
#include "netek_sharesettings.h"
#include "netek_ftphandler.h"
#include "netek_httphandler.h"
#include "netek_netutils.h"
#include "netek_application.h"

neteK::ProtocolHandler::ProtocolHandler(Share *share, QString type, QHostAddress addr)
: m_address(addr.toString()), m_type(type), m_share(share)
{
	new ObjectLog(this,
		tr("New %1 client: %2").arg(m_type).arg(m_address),
		tr("%1 client is gone: %2").arg(m_type).arg(m_address));
}

QString neteK::ProtocolHandler::me() const
{
	return tr("%1 client %2").arg(m_type).arg(m_address);
}

void neteK::ProtocolHandler::logAction(QString what) const
{
	Application::log()->logLine(QString("(%1) %2").arg(me()).arg(what));
}


neteK::Share::Share()
: m_port(0), m_run(false), m_permission(PermissionRO), m_access(AccessAnonymous), m_type(TypeHTTP), m_client_count(0)
{ }

QString neteK::Share::URLScheme() const
{
	return m_type == TypeHTTP ? "http" : "ftp";
}

neteK::Share::Permission neteK::Share::permission() const
{
	return m_permission;
}

void neteK::Share::setPermission(Permission p)
{
	m_permission = p;
}

QString neteK::Share::nicePermission(Permission p)
{
	switch(p) {
		case PermissionRO: return tr("Download only");
		case PermissionRW: return tr("Full access");
		case PermissionDropbox: return tr("Dropbox");
	}
	return QString();
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

bool neteK::Share::authenticate(QString who, QString username, QString password) const
{
	if(m_access == AccessAnonymous) {
		bool ok = m_type == TypeFTP && username == "anonymous";
		if(!ok)
			ok = m_type == TypeHTTP;

		if(ok) {
			logAction(who, tr("authenticated as a guest"));
			return true;
		}
	}

	if(m_access == AccessUsernamePassword && m_username.size() && m_password.size() && m_username == username && m_password == password) {
		logAction(who, tr("authenticated as user %1").arg(username));
		return true;
	}

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

neteK::Share::Type neteK::Share::type() const
{ return m_type; }

void neteK::Share::setType(Type t)
{ m_type = t; }

QString neteK::Share::niceType(Type t)
{
	switch(t) {
		case TypeHTTP:
			return tr("Web folder");
		case TypeFTP:
			return tr("FTP");
	}

	return QString();
}

QString neteK::Share::niceTypeLong(Type t)
{
	switch(t) {
		case TypeHTTP:
			return tr("Web folder (HTTP/WebDAV)");
		case TypeFTP:
			return tr("FTP");
	}

	return QString();
}

int neteK::Share::clients() const
{
	return m_client_count;
}

void neteK::Share::initHandler(QObject *handler)
{
	handler->setParent(m_server);

	connect(handler, SIGNAL(destroyed()), SLOT(clientGone()));
	connect(handler, SIGNAL(transfer()), SIGNAL(transfer()));
}

void neteK::Share::handleNewClient()
{
	while(m_server) {
		QPointer<QTcpSocket> client = m_server->nextPendingConnection();
		if(!client)
			break;

		++m_client_count;
		
		if(m_type == TypeHTTP) {
			initHandler(new HttpHandler(this, client));
		} else {
			QPointer<FtpHandler> handler = new FtpHandler(this, client);
			initHandler(handler);

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
		connect(m_server, SIGNAL(newConnection()), SIGNAL(transfer()));

		if(m_server->listen(QHostAddress::Any, m_port)) {
			new ObjectLog(m_server,
				tr("Share started: %1").arg(niceId()),
				tr("Share stopped: %1").arg(niceId()));
		} else {
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

bool neteK::Share::rename(QString who, QString cwd, QString path1, QString path2) const
{
	bool ret =
		m_permission == PermissionRW
		&& filesystemPathNotRoot(cwd, path1, path1)
		&& filesystemPathNotRoot(cwd, path2, path2)
		&& !(path2+'/').startsWith(path1+'/') // this prevents Qt from going into infinite loop (TODO: report this to trolltech)
		&& QFile(path1).rename(path2);
		
	if(ret)
		logAction(who, tr("renamed %1 into %2").arg(path1).arg(path2));
	
	return ret;
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

bool neteK::Share::createFolder(QString who, QString cwd, QString path) const
{
	bool ret =
		m_permission != PermissionRO
		&& filesystemPathNotRoot(cwd, path, path)
		&& QDir().mkdir(path);
		
	if(ret)
		logAction(who, tr("folder created: %1").arg(path));
		
	return ret;
}

bool neteK::Share::deleteFolder(QString who, QString cwd, QString path) const
{
	bool ret =
		m_permission == PermissionRW
		&& filesystemPathNotRoot(cwd, path, path)
		&& QDir().rmdir(path);
		
	if(ret)
		logAction(who, tr("folder deleted: %1").arg(path));
		
	return ret;
}

QFile *neteK::Share::readFile(QString who, QString cwd, QString path, qint64 pos) const
{
	if(filesystemPathNotRoot(cwd, path, path)) {
		QPointer<QFile> file(new QFile(path));
		if(file->open(QIODevice::ReadOnly)
			&& pos >= 0 && pos <= file->size()
			&& (pos == 0 || file->seek(pos)))
		{
			logAction(who, tr("downloading file %1").arg(path));
			return file;
		}
			
		delete file;
	}
	
	return 0;
}

QFile *neteK::Share::writeFile(QString who, QString cwd, QString path, bool append) const
{
	if(m_permission == PermissionRO)
		return 0;
		
	if(filesystemPathNotRoot(cwd, path, path)) {
		QPointer<QFile> file(new QFile(path));
		if((m_permission == PermissionRW || !file->exists())
			&& file->open(append ? QIODevice::Append : QIODevice::WriteOnly))
		{
			logAction(who, tr("uploading into file %1").arg(path));
			return file;
		}
			
		delete file;
	}
	
	return 0;
}

QFile *neteK::Share::writeFileUnique(QString who, QString cwd, QString &fname) const
{
	if(m_permission == PermissionRO)
		return 0;

	QString path;
	if(filesystemPath(cwd, "", path)) {
		QString prefix(QString("ftp_%1_%2").arg(QDateTime::currentDateTime().toTime_t()).arg(rand() & 0xffff));
		for(int i=0; i<100; ++i) {
			fname = QString("%1_%2").arg(prefix).arg(i);
			QString fpath = QDir(path).filePath(fname);			
			QPointer<QFile> file(new QFile(fpath));
	
			if(!file->exists() && file->open(QIODevice::WriteOnly)) {
				logAction(who, tr("uploading into unique file %1").arg(fpath));
				return file;
			}
	
			delete file;
		}
	}

	return 0;
}

bool neteK::Share::deleteFile(QString who, QString cwd, QString path) const
{
	bool ret =
		m_permission == PermissionRW
		&& filesystemPathNotRoot(cwd, path, path)
		&& QFile(path).remove();
		
	if(ret)
		logAction(who, tr("file deleted: %1").arg(path));
		
	return ret;
}

QFile *neteK::Share::changeAttributes(QString who, QString cwd, QString path) const
{
	QPointer<QFile> ret;
	if(m_permission == PermissionRW && filesystemPathNotRoot(cwd, path, path)) {
		ret = new QFile(path);
		if(!ret->exists())
			delete ret;
	}
		
	if(ret)
		logAction(who, tr("changing attributes of: %1").arg(path));
		
	return ret;
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

void neteK::Share::logAction(QString who, QString what) const
{
	Application::log()->logLine(QString("(%1) %2").arg(who).arg(what));
}

namespace neteK {

class RecursiveDeleteFrame: QObject {
	Q_OBJECT;
	
public:
	RecursiveDeleteFrame(QObject *parent)
	: QObject(parent)
	{ }
	
	QFileInfoList files;
	QFileInfoList::const_iterator file;
};

}

neteK::RecursiveDelete::RecursiveDelete(QObject *p, QString who, Share *share, QString cwd, QString file)
: QObject(p), m_who(who), m_share(share), m_cwd(cwd), m_ok(true)
{
	RecursiveDeleteFrame *f = new RecursiveDeleteFrame(this);
	m_stack.append(f);
	QFileInfo info;
	if(m_share->fileInformation(m_cwd, file, info)) {
		f->files.append(info);
		f->file = f->files.begin();
	} else {
		f->file = f->files.end();
		m_ok = false;
	}
	
	connect(this, SIGNAL(processSignal()), SLOT(process()), Qt::QueuedConnection);
	emit processSignal();
}

void neteK::RecursiveDelete::process()
{
	RecursiveDeleteFrame *f = m_stack.last();
	if(f->file == f->files.end()) {
		delete f;
		m_stack.pop_back();
		
		if(m_stack.isEmpty()) {
			emit done(m_ok);
			deleteLater();
			return;
		}
		
		if(!m_share->deleteFolder(m_who, m_cwd, current_path()))
			m_ok = false;
		
		++m_stack.last()->file;
	} else {
		QString path = current_path();
		
		if(m_share->deleteFile(m_who, m_cwd, path) || m_share->deleteFolder(m_who, m_cwd, path)) {
			++f->file;
		} else {
			RecursiveDeleteFrame *fr = new RecursiveDeleteFrame(this);
			m_stack.append(fr);
			if(m_share->listFolder(m_cwd, path, fr->files)) {
				fr->file = fr->files.begin();
			} else {
				fr->files.clear();
				fr->file = fr->files.end();
			}
		}
	}
	
	emit processSignal();
}

QString neteK::RecursiveDelete::current_path()
{
	QString path;
	foreach(RecursiveDeleteFrame *fr, m_stack) {
		path += '/';
		path += fr->file->fileName();
	}
	return path;
}

#include "netek_share.moc"

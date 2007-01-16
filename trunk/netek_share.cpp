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
	m_network_timeout = new QTimer(this);
	m_network_timeout->setInterval(neteK::networkTimeout * 1000);
	
	connect(m_network_timeout, SIGNAL(timeout()), SLOT(networkTimeout()));
	connect(this, SIGNAL(transfer()), SLOT(resetTimeout()));
	
	new ObjectLog(this,
		tr("New %1 client: %2").arg(m_type).arg(m_address),
		tr("%1 client is gone: %2").arg(m_type).arg(m_address));
		
	resetTimeout();
}

QString neteK::ProtocolHandler::me() const
{
	return tr("%1 client %2").arg(m_type).arg(m_address);
}

void neteK::ProtocolHandler::logAction(QString what) const
{
	Application::log()->logLine(QString("(%1) %2").arg(me()).arg(what));
}

void neteK::ProtocolHandler::resetTimeout()
{
	m_network_timeout->start();
}

void neteK::ProtocolHandler::networkTimeout()
{
	logAction(tr("network timeout"));
	deleteLater();
}


neteK::Share::Share()
: m_port(0), m_run(false), m_permission(PermissionRO), m_access(AccessAnonymous), m_type(TypeHTTP), m_client_count(0)
{ }

QString neteK::Share::root()
{ return "/"; }

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

bool neteK::Share::copyFile(QString who, QString cwd, QString path, QString path2, QFile *&rf_, QFile *&wf_) const
{
	if(m_permission == PermissionRO)
		return false;
		
	if(filesystemPathNotRoot(cwd, path, path) && filesystemPathNotRoot(cwd, path2, path2) && path != path2) {
		QFile *rf = new QFile(path);
		QFile *wf = new QFile(path2);
		if((m_permission == PermissionRW || !wf->exists())
			&& rf->open(QIODevice::ReadOnly)
			&& wf->open(QIODevice::WriteOnly))
		{
			logAction(who, tr("copying file %1 into %2").arg(path).arg(path2));
			rf_ = rf;
			wf_ = wf;
			return true;
		}
			
		delete rf;
		delete wf;
	}
	
	return false;
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

bool neteK::Share::parentPath(QString cwd, QString path, QString &resolved)
{
	if(resolvePath(cwd, path, resolved)) {
		int pos = resolved.lastIndexOf('/');
		if(pos != -1) {
			resolved.truncate(pos);
			if(resolved.isEmpty())
				resolved = '/';
			qDebug() << "=== Parent path:" << resolved;
			return true;
		}
	}
	return false;
}

bool neteK::Share::movePath(QString cwd, QString oldroot, QString newroot, QString file, QString &newfile)
{
	if(resolvePath(cwd, oldroot, oldroot)
		&& resolvePath(cwd, newroot, newroot)
		&& resolvePath(cwd, file, file)
		&& oldroot != newroot
		&& file.startsWith(oldroot))
	{
		QString f = newroot;
		if(file.size() > oldroot.size())
			f += '/' + file.mid(oldroot.size());
		if(resolvePath(cwd, f, newfile)) {
			qDebug() << "=== Move target:" << newfile;
			return true;
		}
	}
	return false;
}

bool neteK::Share::resolvePath(QString cwd, QString path, QString &resolved)
{
	cwd.replace('\\', '/');
	path.replace('\\', '/');
	
	if(!path.startsWith("/"))
		path = cwd + "/" + path;
	
	QStringList current;
	QStringList change = path.split('/');
	foreach(QString c, change)
		if(c == "." || c.isEmpty())
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
	
class WalkShareFrame {
public:
	WalkShareFrame(): skipFolder(false) { }
	QStringList files;
	QStringList::const_iterator file;
	bool skipFolder;
};

}

neteK::WalkShare::WalkShare(QObject *parent, Share *share, QString cwd, QString file, int depth)
: QObject(parent), m_share(share), m_cwd(cwd), m_depth(depth), m_ok(true), m_done(false)
{
	connect(this, SIGNAL(processSignal()), SLOT(process()), Qt::QueuedConnection);
	connect(this, SIGNAL(errorSignal(QString)), SIGNAL(error(QString)), Qt::QueuedConnection);

	QFileInfo tmp;
	if(m_share->fileInformation(m_cwd, file, tmp)) {
		WalkShareFrame *f = new WalkShareFrame;
		f->files.append(file);
		f->file = f->files.begin();
		m_stack.append(f);
		getNext();
	} else {
		m_ok = false;
		emit errorSignal(file);
	}
}

neteK::WalkShare::~WalkShare()
{
	foreach(WalkShareFrame *f, m_stack)
		delete f;
	m_stack.clear();

	if(m_done)
		emit done(m_ok);
}

void neteK::WalkShare::getNext()
{
	emit processSignal();
}

QString neteK::WalkShare::move(QStringList::const_iterator &f)
{
	QString name = *f;
	++f;
	return name;
}

void neteK::WalkShare::process()
{
	if(m_stack.empty()) {
		if(!m_done) {
			m_done = true;
			deleteLater();
		}
		return;
	}
		
	WalkShareFrame *f = m_stack.last();
	if(f->file == f->files.end()) {
		delete f;
		m_stack.pop_back();
		if(m_stack.size()) {
			WalkShareFrame *fr = m_stack.last();
			QString name = move(fr->file);
			if(!skipFolder) {
				emit leaveFolder(name);
				return;
			}
		}
		getNext();
	} else {
		QFileInfo fi;
		bool err = !m_share->fileInformation(m_cwd, *f->file, fi);

		if(!err && !fi.isSymLink() && fi.isDir() && (m_depth < 0 || m_stack.size() <= m_depth)) {
			QFileInfoList info;
			if(m_share->listFolder(m_cwd, *f->file, info)) {
				WalkShareFrame *fr = new WalkShareFrame;
				foreach(QFileInfo i, info)
					fr->files.append(QString("%1/%2").arg(*f->file).arg(i.fileName()));
				fr->file = fr->files.begin();
				m_stack.append(fr);
				emit enterFolder(*f->file);
				return;
			} else {
				err = true;
			}
		}

		QString name = move(f->file);
		if(err)
			emit error(name);
		else
			emit file(name);
	}
}

void neteK::WalkShare::skipFolder()
{
	if(m_stack.size()) {
		WalkShareFrame *f = m_stack.last();
		f->file = m_stack.last()->files.end();
		f->skipFolder = true;
	}
	getNext();
}

neteK::RecursiveDelete::RecursiveDelete(QObject *p, QString who, Share *share, QString cwd, QString file)
: QObject(p), m_who(who), m_share(share), m_cwd(cwd), m_ok(true), m_done(false)
{
	WalkShare *walk = new WalkShare(this, m_share, m_cwd, file);
	connect(walk, SIGNAL(file(QString)), SLOT(deleteFile(QString)));
	connect(walk, SIGNAL(enterFolder(QString)), SIGNAL(getNext()));
	connect(walk, SIGNAL(leaveFolder(QString)), SLOT(deleteFolder(QString)));
	connect(walk, SIGNAL(error(QString)), SIGNAL(error(QString)));
	connect(walk, SIGNAL(error(QString)), SIGNAL(getNext()));
	connect(walk, SIGNAL(done(bool)), SLOT(walkDone(bool)));
	walk->connect(this, SIGNAL(getNext()), SLOT(getNext()));
}

void neteK::RecursiveDelete::emit_error(QString f)
{
	m_ok = false;
	emit error(f);
}

neteK::RecursiveDelete::~RecursiveDelete()
{
	if(m_done)
		emit done(m_ok);
}

void neteK::RecursiveDelete::deleteFile(QString f)
{
	emit getNext();
	if(!m_share->deleteFile(m_who, m_cwd, f))
		emit_error(f);
}

void neteK::RecursiveDelete::deleteFolder(QString f)
{
	emit getNext();
	if(!m_share->deleteFolder(m_who, m_cwd, f))
		emit_error(f);
}

void neteK::RecursiveDelete::walkDone(bool ok)
{
	if(!ok)
		m_ok = false;

	if(!m_done) {
		m_done = true;
		deleteLater();
	}
}

neteK::RecursiveCopy::RecursiveCopy(QObject *parent, QString who, Share *share, QString cwd, QString file, QString newfile, int depth, bool delete_originals)
: QObject(parent), m_who(who), m_share(share), m_cwd(cwd), m_oldfile(file), m_newfile(newfile), m_delete_originals(delete_originals), m_ok(true), m_done(false)
{
	WalkShare *walk = new WalkShare(this, m_share, m_cwd, file, depth);
	connect(walk, SIGNAL(file(QString)), SLOT(copyFile(QString)));
	connect(walk, SIGNAL(enterFolder(QString)), SLOT(copyFolder(QString)));
	connect(walk, SIGNAL(leaveFolder(QString)), m_delete_originals ? SLOT(deleteFolder(QString) : SIGNAL(getNext())));
	connect(walk, SIGNAL(error(QString)), SIGNAL(error(QString)));
	connect(walk, SIGNAL(error(QString)), SIGNAL(getNext()));
	connect(walk, SIGNAL(done(bool)), SLOT(walkDone(bool)));
	walk->connect(this, SIGNAL(getNext()), SLOT(getNext()));
	walk->connect(this, SIGNAL(skipFolder()), SLOT(skipFolder()));

	connect(this, SIGNAL(copySignal()), SLOT(copy()), Qt::QueuedConnection);
}

void neteK::RecursiveCopy::emit_error(QString f)
{
	m_ok = false;
	emit error(f);
}

void neteK::RecursiveCopy::copyFile(QString file)
{
	QString newfile;
	if(Share::movePath(m_cwd, m_oldfile, m_newfile, file, newfile)) {
		QFileInfo info;
		if(m_share->fileInformation(m_cwd, file, info)) {
			if(info.isSymLink()) {
				emit getNext();
				return;
			} else {
				QFile *rf, *wf;
				if(m_share->copyFile(m_who, m_cwd, file, newfile, rf, wf)) {
					rf->setParent(this);
					wf->setParent(this);
					if(m_in)
						m_in->deleteLater();
					if(m_out)
						m_out->deleteLater();
					m_in = rf;
					m_out = wf;
					m_in_file = file;
					m_out_file = newfile;
					emit copySignal();
					return;
				}
			}
		}
	}

	emit getNext();
	emit_error(file);
}

void neteK::RecursiveCopy::copyFolder(QString file)
{
	QString newfile;
	if(!Share::movePath(m_cwd, m_oldfile, m_newfile, file, newfile)) {
		emit getNext();
		emit_error(file);
		return;
	}

	if(m_share->createFolder(m_who, m_cwd, newfile)) {
		emit getNext();
	} else {
		emit skipFolder();
		emit_error(newfile);
	}
}

void neteK::RecursiveCopy::deleteFolder(QString file)
{
	emit getNext();
	if(!m_share->deleteFolder(m_who, m_cwd, file))
		emit_error(file);
}

void neteK::RecursiveCopy::copy()
{
	bool next = false;
	QString *err = 0;
	if(m_in && m_out) {
		if(m_in->atEnd()) {
			next = true;
		} else {
			char buf[networkBufferSize];
			qint64 size = m_in->read(buf, sizeof(buf));
			if(size < 0) {
				err = &m_in_file;
			} else {
				if(size == m_out->write(buf, size))
					emit copySignal();
				else
					err = &m_out_file;
			}
		}
	}

	if(next || err) {
		if(m_in) {
			if(!err && m_delete_originals) {
				m_in->close();
				m_in->remove();
			}
			m_in->deleteLater();
			m_in = 0;
		}

		if(m_out) {
			if(err) {
				m_out->close();
				m_out->remove();
			}
			m_out->deleteLater();
			m_out = 0;
		}

		emit getNext();

		if(err)
			emit_error(*err);
	}
}

void neteK::RecursiveCopy::walkDone(bool ok)
{
	if(!ok)
		m_ok = false;

	if(!m_done) {
		m_done = true;
		deleteLater();
	}
}

neteK::RecursiveCopy::~RecursiveCopy()
{
	if(m_done)
		emit done(m_ok);
}

neteK::RecursiveMove::RecursiveMove(QObject *parent, QString who, Share *share, QString cwd, QString file, QString newfile)
: QObject(parent), m_ok(true), m_done(false)
{
	QString target;
	if(!Share::movePath(cwd, file, newfile, file, target)) {
		setDone(false);
		return;
	}

	if(share->rename(who, cwd, file, target)) {
		setDone(true);
		return;
	}

	RecursiveCopy *copy = new RecursiveCopy(this, who, share, cwd, file, newfile, -1, true);
	connect(copy, SIGNAL(done(bool)), SLOT(setDone(bool)));
	connect(copy, SIGNAL(
}

neteK::RecursiveMove::~RecursiveMove()
{
	if(m_done)
		emit done(m_ok);
}

void neteK::RecursiveMove::setDone(bool ok)
{
	m_ok = ok;
	m_done = true;
	deleteLater();
}
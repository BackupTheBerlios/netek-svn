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

#ifndef __NETEK_SHARE_H__
#define __NETEK_SHARE_H__

#include <QtGui>
#include <QtNetwork>

namespace neteK {

class Share;

class ProtocolHandler: public QObject {
	Q_OBJECT;
	
private:
	QString m_address, m_type;
	QPointer<QTimer> m_network_timeout;
	
protected:
	QPointer<Share> m_share;
	
	ProtocolHandler(Share *share, QString type, QHostAddress addr);
	
	void logAction(QString line) const;
	QString me() const;
	
signals:
	void transfer();
	
private slots:
	void resetTimeout();
	void networkTimeout();
};

class Share: public QObject {
		Q_OBJECT;

	public slots:
		void checkServer();
		bool showSettings();
		void restartIfStarted();
		void startIfStopped();
		void stop();

	private slots:
		void handleNewClient();
		void clientGone();

	signals:
		void settingsChanged();
		void statusChanged();
		void transfer();

	public:
		Share();

		void setFolder(QString top);
		QString folder() const;

		void setPort(quint16 port);
		quint16 port() const;

		int clients() const;
		
		enum Status { StatusUnconfigured, StatusChanging, StatusStopped, StatusStarted };
		
		Status status() const;

		bool runStatus() const;
		void setRunStatus(bool);

		QString niceId() const;

		enum Permission { PermissionRO, PermissionRW, PermissionDropbox };
		Permission permission() const;
		void setPermission(Permission p);
		static QString nicePermission(Permission p);
		
		QString URLScheme() const;

		enum Access { AccessAnonymous, AccessUsernamePassword };
		Access access() const;
		void setAccess(Access a);
		void setUsernamePassword(QString u, QString p);
		void usernamePassword(QString &u, QString &p) const;

		enum Type { TypeFTP, TypeHTTP };
		Type type() const;
		void setType(Type t);
		static QString niceType(Type t);
		static QString niceTypeLong(Type t);
		
		QString initialFolder() const;
		bool changeCurrentFolder(QString cwd, QString change, QString &newcwd) const;
		bool rename(QString who, QString cwd, QString path, QString path2) const;
		bool fileInformation(QString cwd, QString path, QFileInfo &info) const;
		bool listFolder(QString cwd, QString path, QFileInfoList &list) const;
		bool authenticate(QString who, QString username, QString password) const;
		QFile *readFile(QString who, QString cwd, QString path, qint64 pos = 0) const;
		QFile *writeFile(QString who, QString cwd, QString path, bool append = false) const;
		QFile *writeFileUnique(QString who, QString cwd, QString &name) const;
		bool deleteFile(QString who, QString cwd, QString path) const;
		bool createFolder(QString who, QString cwd, QString path) const;
		bool deleteFolder(QString who, QString cwd, QString path) const;
		QFile *changeAttributes(QString who, QString cwd, QString path) const;
		
		bool resolvePath(QString cwd, QString path, QString &resolved) const;

	private:
		QString m_folder;
		quint16 m_port;
		bool m_run;
		QPointer<QTcpServer> m_server;
		Permission m_permission;
		Access m_access;
		Type m_type;
		QString m_username, m_password;
		int m_client_count;

		void initHandler(QObject *);
		
		bool filesystemPath(QString cwd, QString path, QString &fspath, bool nonroot) const;
		
		void logAction(QString who, QString what) const;
		
		inline bool filesystemPath(QString cwd, QString path, QString &fspath) const
		{ return filesystemPath(cwd, path, fspath, false); }
		
		inline bool filesystemPathNotRoot(QString cwd, QString path, QString &fspath) const
		{ return filesystemPath(cwd, path, fspath, true); }
};

class RecursiveDeleteFrame;
class RecursiveDelete: public QObject {
	Q_OBJECT;
	
	QString m_who;
	QPointer<Share> m_share;
	QList<RecursiveDeleteFrame*> m_stack;
	QString m_cwd;
	bool m_ok;
	
	QString current_path();
	
public:
	RecursiveDelete(QObject *parent, QString who, Share *share, QString cwd, QString file);
	
signals:
	void done(bool ok);
	void processSignal();
	
private slots:
	void process();
};

}

#endif

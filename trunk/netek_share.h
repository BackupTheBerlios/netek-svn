#ifndef __NETEK_SHARE_H__
#define __NETEK_SHARE_H__

#include <QtGui>
#include <QtNetwork>

namespace neteK {

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

		bool readOnly() const;
		void setReadOnly(bool yes);
		
		QString URLProtocol() const;

		enum Access { AccessAnonymous, AccessUsernamePassword };
		Access access() const;
		void setAccess(Access a);
		void setUsernamePassword(QString u, QString p);
		void usernamePassword(QString &u, QString &p) const;
		
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
		
		bool resolvePath(QString cwd, QString path, QString &resolved) const;

	private:
		QString m_folder;
		quint16 m_port;
		bool m_run;
		QPointer<QTcpServer> m_server;
		bool m_readonly;
		Access m_access;
		QString m_username, m_password;
		int m_client_count;
		
		bool filesystemPath(QString cwd, QString path, QString &fspath, bool nonroot) const;
		
		void logAction(QString who, QString what) const;
		
		inline bool filesystemPath(QString cwd, QString path, QString &fspath) const
		{ return filesystemPath(cwd, path, fspath, false); }
		
		inline bool filesystemPathNotRoot(QString cwd, QString path, QString &fspath) const
		{ return filesystemPath(cwd, path, fspath, true); }
};

}

#endif

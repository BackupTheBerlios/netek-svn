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
		void recheckServer();

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

		enum Access { AccessAnonymous, AccessUsernamePassword };
		Access access() const;
		void setAccess(Access a);
		void setUsernamePassword(QString u, QString p);
		void usernamePassword(QString &u, QString &p) const;

		bool isFolderReadable(QStringList path) const;
		bool fileExists(QStringList path) const;
		bool rename(QStringList path1, QStringList path2) const;
		bool pathInformation(QStringList path, QFileInfo &info) const;
		bool pathInformationList(QStringList path, QFileInfoList &list) const;
		bool authenticate(QString username, QString password) const;
		QFile *readFile(QStringList path, qint64 pos = 0) const;
		QFile *writeFile(QStringList path, bool append = false) const;
		QFile *writeFileUnique(QStringList path, QString &name) const;
		bool deleteFile(QStringList path) const;
		bool createFolder(QStringList path) const;
		bool deleteFolder(QStringList path) const;

	private:
		QString m_folder;
		quint16 m_port;
		bool m_run;
		QPointer<QTcpServer> m_server;
		bool m_readonly;
		Access m_access;
		QString m_username, m_password;
		int m_client_count;

		bool resolveDir(QStringList path, QDir &dir) const;
		bool resolveDirName(QStringList path, QDir &dir, QString &name) const;
		bool resolveFile(QStringList path, QFile &file) const;
		static bool forbiddenName(QString name);
};

}

#endif

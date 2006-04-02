#ifndef __NETEK_APPLICATION_H__
#define __NETEK_APPLICATION_H__

#include <QtGui>

namespace neteK {

class ObjectLog: public QObject {
	Q_OBJECT;
	
	QString m_stop;
	
public:
	ObjectLog(QObject *attachto, QString start, QString stop);
	~ObjectLog();
};

class Log: public QObject {
	Q_OBJECT;
	
	QString m_file;

public:
	Log();
	
	void logLine(QString line);
	QString readLog();
	void clearLog();

signals:
	void appendToLog(QString data);
};

class Application: public QApplication {
	Q_OBJECT;
	
	static Log *g_log;
	QList<QStringList> m_commands;
	
public:
	Application(int &argc, char **argv);
	
	static Log *log();
	
	static QDir applicationData();
	static QString applicationVersion();
	
signals:
	void command_createShare(QString file);
	
	void processCommandsSignal();
	
private slots:
	void processCommandsSlot();
	
public slots:
	void userQuit();
	void processCommand(QStringList args);
};

}

#endif

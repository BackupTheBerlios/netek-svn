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

signals:
	void appendToLog(QString data);	
};

class Application: public QApplication {
	Q_OBJECT;
	
	static Log *g_log;
	
public:
	Application(int &argc, char **argv);
	
	static Log *log();
	
	static QDir applicationData();
	static QString applicationVersion();
	
public slots:
	void userQuit();
};

}

#endif

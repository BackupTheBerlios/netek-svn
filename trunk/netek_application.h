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
	void command_quit();
	
	void processCommandsSignal();
	
private slots:
	void processCommandsSlot();
	
public slots:
	void userQuit();
	void processCommand(QStringList args);
};

}

#endif

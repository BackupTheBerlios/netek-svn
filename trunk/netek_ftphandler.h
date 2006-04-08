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

#ifndef __NETEK_FTPHANDLER_H__
#define __NETEK_FTPHANDLER_H__

#include <QtNetwork>

namespace neteK {
	
class Share;

class FtpHandlerData: public QObject {
	Q_OBJECT;
	
	bool m_start, m_transfer;
	bool m_transfer_error;
	bool m_stop_transfer;
	
private slots:
	void handleError(QAbstractSocket::SocketError err);

protected:
	QPointer<QAbstractSocket> m_socket;
	QPointer<QIODevice> m_dev;
	bool m_send;
	
	bool transferError();
	bool sourceSink(QPointer<QIODevice> &source, QPointer<QIODevice> &sink);
	bool connectSourceSink();
	bool connectSocket();
	
	void emitStartStatus(bool ok);
	void emitTransferStatus(bool ok);

protected slots:
	void transferEvent();
	virtual void status() = 0;

signals:
	void startStatus(bool ok);
	void transferStatus(bool ok);
	
	void statusSignal();
	void transferSignal();

public:
	FtpHandlerData();
	virtual void start(QIODevice *dev, bool send) = 0;
};

class FtpHandlerPORT: public FtpHandlerData {
	Q_OBJECT;

	QHostAddress m_address;
	quint16 m_port;
	bool m_connected;

private slots:
	void status();

public:
	FtpHandlerPORT(QHostAddress address, quint16 port);

	virtual void start(QIODevice *dev, bool send);
};

class FtpHandlerPASV: public FtpHandlerData {
	Q_OBJECT;

	QPointer<QTcpServer> m_server;
	QHostAddress m_address;
	
private slots:
	void status();
	void handleNewClient();

public:
	FtpHandlerPASV(QHostAddress address, quint16 &port);

	virtual void start(QIODevice *dev, bool send);
};

class FtpHandler: public QObject {
	Q_OBJECT;

	static const int CommandFlagLoggedIn = 1;

	struct command {
		void (FtpHandler::*method)(QString);
		int flags;
	};
	QMap<QString, command> m_commands;
	void addCommand(void (FtpHandler::*)(QString), QString cmd, int flags);

	void command_USER(QString);
	void command_PASS(QString);
	void command_QUIT(QString);
	void command_REIN(QString);
	void command_NOOP(QString);
	void command_SYST(QString);
	void command_MODE(QString);
	void command_RNFR(QString);
	void command_RNTO(QString);
	void command_RETR(QString);
	void command_REST(QString);
	void command_STOU(QString);
	void command_STOR(QString);
	void command_APPE(QString);
	void command_DELE(QString);
	void command_RMD(QString);
	void command_MKD(QString);
	void command_PORT(QString);
	void command_PASV(QString);
	void command_TYPE(QString);
	void command_STRU(QString);
	void command_NLST(QString);
	void command_LIST(QString);
	void command_CDUP(QString);
	void command_CWD(QString);
	void command_PWD(QString);
	void command_HELP(QString);
	void command_FEAT(QString);
	void command_OPTS(QString);
	void command_SIZE(QString);
	void command_MDTM(QString);
	void command_EPSV(QString);
	void command_SITE(QString);


	QPointer<Share> m_share;
	QPointer<QAbstractSocket> m_control;
	QByteArray m_control_buffer;
	QQueue<QString> m_requests;
	QPointer<FtpHandlerData> m_data__;
	bool m_loggedin;
	QString m_username;
	bool m_control_channel_blocked;
	QString m_cwd;
	qint64 m_rest;
	bool m_utf8;
	QString m_store_unique;
	QString m_rename_from;
	QHostAddress m_public;

	void init();
	void closeDataChannel();
	void setDataChannel(FtpHandlerData *data);
	void startDataChannel(QIODevice *dev, bool send);
	QByteArray makeLine(QString line);
	void sendLine(QString line);
	void sendLine(int code, QString args);
	void sendLines(int code, QStringList args);
	bool list(QString path, QFileInfoList &lst, bool *dir = 0);
	static QString quotedPath(QString path);
	static QString month(int m);
	QString me();
	void logAction(QString what);
	static void parseLine(QString line, QString &cmd, QString &args);
	
signals:
	void processSignal();
	
private slots:
	void process();

	void dataStartStatus(bool ok);
	void dataTransferStatus(bool ok);

public slots:
	void start(QHostAddress publ);
	
public:
	FtpHandler(Share *s, QAbstractSocket *control);
};

}

#endif

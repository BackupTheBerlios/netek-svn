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

// TODO: network timeout

#include "netek_settings.h"
#include "netek_ftphandler.h"
#include "netek_netutils.h"
#include "netek_application.h"

neteK::FtpHandlerData::FtpHandlerData()
: m_start(false), m_transfer(false), m_transfer_error(false), m_stop_transfer(false), m_send(false)
{
	//qDebug() << __FUNCTION__;

	connect(this, SIGNAL(statusSignal()), SLOT(status()), Qt::QueuedConnection);
	connect(this, SIGNAL(transferSignal()), SLOT(transferEvent()), Qt::QueuedConnection);
}

bool neteK::FtpHandlerData::transferError()
{ return m_transfer_error; }

void neteK::FtpHandlerData::handleError(QAbstractSocket::SocketError err)
{
	if(err != QAbstractSocket::RemoteHostClosedError)
		m_transfer_error = true;
		
	emit statusSignal();
}

void neteK::FtpHandlerData::emitStartStatus(bool ok)
{
	if(!m_start) {
		m_start = true;
		emit startStatus(ok);
	}
}

void neteK::FtpHandlerData::emitTransferStatus(bool ok)
{
	Q_ASSERT(m_start);

	if(!m_transfer) {
		m_transfer = true;
		emit transferStatus(ok);
	}
}

bool neteK::FtpHandlerData::sourceSink(QPointer<QIODevice> &source, QPointer<QIODevice> &sink)
{
	//qDebug() << __FUNCTION__;

	if(!m_socket || !m_dev)
		return false;

	if(m_send) {
		source = m_dev;
		sink = m_socket;
	} else {
		source = m_socket;
		sink = m_dev;
	}

	return true;
}

bool neteK::FtpHandlerData::connectSourceSink()
{
	//qDebug() << __FUNCTION__;

	QPointer<QIODevice> source, sink;
	return sourceSink(source, sink)
		&& connect(source, SIGNAL(readyRead()), SIGNAL(transferSignal()))
		&& connect(sink, SIGNAL(bytesWritten(qint64)), SIGNAL(transferSignal()));
}

bool neteK::FtpHandlerData::connectSocket()
{
	return m_socket
		&& connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(handleError(QAbstractSocket::SocketError)))
		&& connect(m_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SIGNAL(statusSignal()));
}

void neteK::FtpHandlerData::transferEvent()
{
	//qDebug() << __FUNCTION__;
	
	if(m_stop_transfer)
		return;

	QPointer<QIODevice> source, sink;
	if(sourceSink(source, sink)) {
		//qDebug() << "Bytes available" << source->bytesAvailable();
		//qDebug() << "Bytes to write" << sink->bytesToWrite();
		if(sink->bytesToWrite())
			return;
			
		for(;;) {
			char buf[networkBufferSize];
			qint64 read = source->read(buf, sizeof(buf));
			//qDebug() << "Read" << read;
			if(read <= 0) {
				if(read < 0) {
					qWarning() << "Error reading";
					m_transfer_error = true;
					m_stop_transfer = true;
				} else if(!source->isSequential())
					m_stop_transfer = true;
			} else if(read != sink->write(buf, read)) {
				qWarning() << "Error writing";
				m_transfer_error = true;
				m_stop_transfer = true;
			} else if(source->isSequential()) {
				//qDebug() << "Continue read";
				continue;
			}	
			
			break;
		}

		if(m_stop_transfer) {
			source->close();
			sink->close();
		}
	}
}


neteK::FtpHandlerPORT::FtpHandlerPORT(QHostAddress address, quint16 port)
: m_address(address), m_port(port), m_connected(false)
{
	//qDebug() << __FUNCTION__;
}

void neteK::FtpHandlerPORT::start(QIODevice *dev, bool send)
{
	//qDebug() << __FUNCTION__;

	m_send = send;

	Q_ASSERT(!m_dev);
	m_dev = dev;
	m_dev->setParent(this);

	Q_ASSERT(!m_socket);

	m_socket = new QTcpSocket(this);
	connectSocket();
	m_socket->connectToHost(m_address, m_port);
}

void neteK::FtpHandlerPORT::status()
{
	//qDebug() << __FUNCTION__;

	if(!m_socket)
		return;

	//qDebug() << __FUNCTION__ << m_socket->state();

	if(!m_connected) {
		if(m_socket->state() == QAbstractSocket::ConnectedState) {
			m_connected = true;
			emitStartStatus(true);
			connectSourceSink();
			transferEvent();
		} else if(m_socket->state() == QAbstractSocket::UnconnectedState) {
			emitStartStatus(false);
		}
	} else {
		if(m_socket->state() == QAbstractSocket::UnconnectedState) {
			emitTransferStatus(!transferError());
		}
	}
}

neteK::FtpHandlerPASV::FtpHandlerPASV(QHostAddress address, quint16 &port)
: m_address(address)
{
	for(int i=0; i<100; ++i) {
		port = randomPort();

		if(m_server)
			delete m_server;
		m_server = new QTcpServer(this);

		if(m_server->listen(QHostAddress::Any, port))
			return;
	}

	port = 0;
}

void neteK::FtpHandlerPASV::start(QIODevice *dev, bool send)
{
	m_send = send;

	Q_ASSERT(!m_dev);
	m_dev = dev;
	m_dev->setParent(this);

	Q_ASSERT(m_server);
	connect(m_server, SIGNAL(newConnection()), SLOT(handleNewClient()), Qt::QueuedConnection);
	handleNewClient();
}

void neteK::FtpHandlerPASV::handleNewClient()
{
	if(m_server) {
		if(!m_socket) {
			m_socket = m_server->nextPendingConnection();
			if(!m_socket)
				return;

			if(!(m_address == m_socket->peerAddress())) {
				Application::log()->logLine(
					tr("Expected IP=%1, got=%2, crack attempt? Dropping connection!")
						.arg(m_address.toString())
						.arg(m_socket->peerAddress().toString()));
						
				delete m_socket;
				return;
			}

			m_socket->setParent(this);
			connectSocket();
			connectSourceSink();
			emitStartStatus(true);
		}

		m_server->deleteLater();
		m_server = 0;

		transferEvent();
	}
}

void neteK::FtpHandlerPASV::status()
{
	//qDebug() << __FUNCTION__;

	if(!m_socket)
		return;

	//qDebug() << __FUNCTION__ << m_socket->state();
	
	if(m_socket->state() == QAbstractSocket::UnconnectedState)
		emitTransferStatus(!transferError());
}


neteK::FtpHandler::FtpHandler(Share *s, QAbstractSocket *control)
: ProtocolHandler(s, "FTP", control->peerAddress()), m_control(control), m_control_channel_blocked(false)
{
	Settings settings;

	m_control->setParent(this);

#define ADD_COMMAND(_cmd, _flags) addCommand(&FtpHandler::command_ ## _cmd, #_cmd, (_flags))
	ADD_COMMAND(USER, 0);
	ADD_COMMAND(PASS, 0);
	//ADD_COMMAND(ACCT, 0);
	ADD_COMMAND(CWD, CommandFlagLoggedIn);
	ADD_COMMAND(CDUP, CommandFlagLoggedIn);
	//ADD_COMMAND(SMNT, 0);
	ADD_COMMAND(QUIT, 0);
	ADD_COMMAND(REIN, 0);
	ADD_COMMAND(PORT, CommandFlagLoggedIn);
	if(settings.ftpAllowPassive())
		ADD_COMMAND(PASV, CommandFlagLoggedIn);
	ADD_COMMAND(TYPE, CommandFlagLoggedIn);
	ADD_COMMAND(STRU, CommandFlagLoggedIn);
	ADD_COMMAND(MODE, CommandFlagLoggedIn);
	ADD_COMMAND(RETR, CommandFlagLoggedIn);
	ADD_COMMAND(STOR, CommandFlagLoggedIn);
	ADD_COMMAND(STOU, CommandFlagLoggedIn);
	ADD_COMMAND(APPE, CommandFlagLoggedIn);
	//ADD_COMMAND(ALLO, 0);
	ADD_COMMAND(REST, CommandFlagLoggedIn);
	ADD_COMMAND(RNFR, CommandFlagLoggedIn);
	ADD_COMMAND(RNTO, CommandFlagLoggedIn);
	//ADD_COMMAND(ABOR, 0);
	ADD_COMMAND(DELE, CommandFlagLoggedIn);
	ADD_COMMAND(RMD, CommandFlagLoggedIn);
	ADD_COMMAND(MKD, CommandFlagLoggedIn);
	ADD_COMMAND(PWD, CommandFlagLoggedIn);
	ADD_COMMAND(LIST, CommandFlagLoggedIn);
	ADD_COMMAND(NLST, CommandFlagLoggedIn);
	ADD_COMMAND(SITE, CommandFlagLoggedIn);
	ADD_COMMAND(SYST, 0);
	//ADD_COMMAND(STAT, 0);
	ADD_COMMAND(HELP, 0);
	ADD_COMMAND(NOOP, 0);

	ADD_COMMAND(FEAT, 0);
	ADD_COMMAND(OPTS, 0);

	ADD_COMMAND(SIZE, CommandFlagLoggedIn);
	ADD_COMMAND(MDTM, CommandFlagLoggedIn);
	//ADD_COMMAND(MLST, CommandFlagLoggedIn);
	//ADD_COMMAND(MLSD, CommandFlagLoggedIn);

	//ADD_COMMAND(EPRT, CommandFlagLoggedIn);
	if(settings.ftpAllowPassive())
		ADD_COMMAND(EPSV, CommandFlagLoggedIn);
#undef ADD_COMMAND
}

void neteK::FtpHandler::init()
{
	Settings settings;

	m_loggedin = false;
	m_username.clear();
	m_cwd = m_share->initialFolder();
	m_rest = -1;
	m_utf8 = settings.ftpUseUnicodeByDefault();
	m_store_unique.clear();
	m_rename_from.clear();
	closeDataChannel();

	sendLine(220, QCoreApplication::applicationName());

	emit processSignal();
}

void neteK::FtpHandler::start(QHostAddress publ)
{
	m_public = publ;
	logAction(tr("using %1 as a public address").arg(m_public.toString()));
	
	if(m_control) {
		connect(m_control, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SIGNAL(processSignal()));
		connect(m_control, SIGNAL(error(QAbstractSocket::SocketError)), SIGNAL(processSignal()));
		connect(m_control, SIGNAL(readyRead()), SIGNAL(processSignal()));
	}
	
	connect(this, SIGNAL(processSignal()), SLOT(process()), Qt::QueuedConnection);
	connect(this, SIGNAL(processSignal()), SIGNAL(transfer()));

	init();
}

bool neteK::FtpHandler::list(QString path, QFileInfoList &lst, bool *dir)
{
	QFileInfo info;
	if(m_share->fileInformation(m_cwd, path, info)) {
		if(dir)
			*dir = info.isDir();

		if(info.isDir())
			return m_share->listFolder(m_cwd, path, lst);

		lst.append(info);
		return true;
	}

	return false;
}

void neteK::FtpHandler::closeDataChannel()
{
	if(m_data__) {
		m_data__->deleteLater();
		m_data__ = 0;
	}
}

void neteK::FtpHandler::setDataChannel(FtpHandlerData *data)
{
	closeDataChannel();

	if(data) {
		m_data__ = data;
		m_data__->setParent(this);
		connect(m_data__, SIGNAL(startStatus(bool)), SLOT(dataStartStatus(bool)), Qt::QueuedConnection);
		connect(m_data__, SIGNAL(transferStatus(bool)), SLOT(dataTransferStatus(bool)), Qt::QueuedConnection);
		connect(m_data__, SIGNAL(transferSignal()), SIGNAL(transfer()));
	}
}

void neteK::FtpHandler::startDataChannel(QIODevice *dev, bool send)
{
	m_control_channel_blocked = true;

	if(!m_data__)
		setDataChannel(new FtpHandlerPORT(
			m_control ? m_control->peerAddress() : QHostAddress(), 20));

	m_data__->start(dev, send);
}

void neteK::FtpHandler::addCommand(void (FtpHandler::*method)(QString), QString cmd, int flags)
{
	cmd = cmd.toUpper();
	m_commands[cmd].method = method;
	m_commands[cmd].flags = flags;
}

void neteK::FtpHandler::command_USER(QString args)
{
	m_loggedin = false;
	m_username = args;

	sendLine(331, "User name okay, need password.");
}

void neteK::FtpHandler::command_PASS(QString args)
{
	if(m_share->authenticate(me(), m_username, args)) {
		m_loggedin = true;
		sendLine(230, "User logged in, proceed.");
	} else {
		m_loggedin = false;
		sendLine(530, "Not logged in.");
	}
}

void neteK::FtpHandler::command_REIN(QString)
{
	init();
}

void neteK::FtpHandler::command_RNFR(QString args)
{
	QFileInfo info;
	if(m_share->fileInformation(m_cwd, args, info)) {
		m_rename_from = args;
		sendLine(350, "Requested file action pending further information.");
	} else
		sendLine(550, "Requested action not taken.");
}

void neteK::FtpHandler::command_RNTO(QString args)
{
	if(m_share->rename(me(), m_cwd, m_rename_from, args))
		sendLine(250, "Requested file action okay, completed.");
	else
		sendLine(553, "Requested action not taken.");
}

void neteK::FtpHandler::command_QUIT(QString)
{
	if(m_control) {
		sendLine(221, "Service closing control connection.");
		m_control->close();
	}
}

void neteK::FtpHandler::command_FEAT(QString)
{

	QStringList feats;
	feats.append("Features:");
	feats.append("MDTM");
	feats.append("REST STREAM");
	feats.append("SIZE");
	feats.append("UTF8");
	feats.append("End");

	sendLines(211, feats);
}

void neteK::FtpHandler::command_OPTS(QString args)
{
	if(args.toUpper() == "UTF8 ON") {
		m_utf8 = true;
		sendLine(200, "Command okay.");
	} else
		sendLine(501, "Syntax error in parameters or arguments.");
}

void neteK::FtpHandler::command_SITE(QString line)
{
	QString cmd, args;
	parseLine(line, cmd, args);
	
	if(cmd == "CHMOD") {
		QRegExp rx("([0-9]+) (.+)");
		uint mode = 0;
		bool mode_ok = false;
		if(!rx.exactMatch(args)
			|| (mode = rx.cap(1).toUInt(&mode_ok, 8), !mode_ok)
			|| mode > 0777)
		{
			sendLine(501, "Syntax error in parameters or arguments.");
			return;
		}
		
		QFile::Permissions p = 0;
#define X(_n, _m) if(mode & _m) p = p | QFile::_n;
		X(ReadOwner, 0400);
		X(WriteOwner, 0200);
		X(ExeOwner, 0100);
		X(ReadGroup, 040);
		X(WriteGroup, 020);
		X(ExeGroup, 010);
		X(ReadOther, 04);
		X(WriteOther, 02);
		X(ExeOther, 01);
#undef X
		
		QPointer<QFile> file = m_share->changeAttributes(me(), m_cwd, rx.cap(2));
		if(file && file->setPermissions(p))
			sendLine(200, "Command okay.");
		else
			sendLine(550, "Requested action not taken.");
		
		delete file;
	} else
		sendLine(500, "Syntax error, command unrecognized.");
}

void neteK::FtpHandler::command_HELP(QString args)
{
	QStringList cmds;
	
	if(args.toUpper() == "SITE") {
		cmds.append("CHMOD");
	} else {
		cmds.append("The following commands are recognized.");
	
		int cnt = 0;
		QString buf;
		foreach(QString cmd, m_commands.keys()) {
			if(buf.size()) {
				buf += ' ';
				while(buf.size() % 5 != 0)
					buf += ' ';
			}
	
			++cnt;
			buf += cmd;
			if(cnt >= 12) {
				cnt = 0;
				cmds.append(buf);
				buf.clear();
			}
		}
	
		if(buf.size())
			cmds.append(buf);
	
		cmds.append("Help OK.");
	}
	
	sendLines(214, cmds);
}

void neteK::FtpHandler::command_STRU(QString args)
{
	if(args == "F")
		sendLine(200, "Command okay.");
	else
		sendLine(504, "Command not implemented for that parameter.");
}

void neteK::FtpHandler::command_TYPE(QString args)
{
	if(args == "I") {
		//m_convert_crlf = false;
	} else if(args == "A" || args.startsWith("A ")) {
		//m_convert_crlf = true;
	} else {
		sendLine(504, "Command not implemented for that parameter.");
		return;
	}

	sendLine(200, "Command okay.");
}

void neteK::FtpHandler::command_CDUP(QString)
{
	if(m_share->changeCurrentFolder(m_cwd, "..", m_cwd))
		sendLine(200, "Command okay.");
	else
		sendLine(550, "Requested action not taken.");
}

void neteK::FtpHandler::command_CWD(QString args)
{
	if(m_share->changeCurrentFolder(m_cwd, args, m_cwd))
		sendLine(250, "Requested file action okay, completed.");
	else
		sendLine(550, "Requested action not taken.");
}

void neteK::FtpHandler::command_PWD(QString)
{
	sendLine(257, quotedPath(m_cwd));
}

QString neteK::FtpHandler::quotedPath(QString path)
{
	QString qf('"');
	foreach(QChar c, path) {
		if(c == '"')
			qf += "\"\"";
		else
			qf += c;
	}

	qf += '"';

	return qf;
}

void neteK::FtpHandler::command_MDTM(QString args)
{
	QFileInfo info;
	if(m_share->fileInformation(m_cwd, args, info) && info.isFile()) {
		sendLine(213, info.lastModified().toString("yyyyMMddHHmmss"));
	} else {
		sendLine(550, "Requested action not taken.");
	}
}

void neteK::FtpHandler::command_SIZE(QString args)
{
	QFileInfo info;
	if(m_share->fileInformation(m_cwd, args, info) && info.isFile()) {
		sendLine(213, QString::number(info.size()));
	} else {
		sendLine(550, "Requested action not taken.");
	}
}

void neteK::FtpHandler::command_NLST(QString args)
{
	bool isdir;
	QFileInfoList info;
	if(!list(args, info, &isdir)) {
		sendLine(450, "Requested file action not taken.");
		return;
	}

	QBuffer *buf = new QBuffer;
	buf->open(QIODevice::ReadWrite);
	foreach(QFileInfo i, info)
		buf->write(makeLine(
			isdir && args.size()
				? args + "/" + i.fileName() // emulating vsftpd here
				: i.fileName()));

	qDebug() << ">>> [" << buf->data().data() << "]";

	buf->reset();
	startDataChannel(buf, true);
}

QString neteK::FtpHandler::month(int m)
{
	switch(m) {
		case 1: return "Jan";
		case 2: return "Feb";
		case 3: return "Mar";
		case 4: return "Apr";
		case 5: return "May";
		case 6: return "Jun";
		case 7: return "Jul";
		case 8: return "Aug";
		case 9: return "Sep";
		case 10: return "Oct";
		case 11: return "Nov";
		case 12: return "Dec";
	}

	return QString();
}

void neteK::FtpHandler::command_LIST(QString args)
{
	{
		QRegExp rx("(-[a-zA-Z]+)");
		QRegExp rx2("(-[a-zA-Z]+ +).*");
		for(;;) {
			QRegExp *ok = 0;

			if(rx.exactMatch(args))
				ok = &rx;
			else if(rx2.exactMatch(args))
				ok = &rx2;

			if(ok)
				args.remove(0, ok->cap(1).size());
			else
				break;
		}
	}

	QFileInfoList info;
	if(!list(args, info)) {
		sendLine(450, "Requested file action not taken.");
		return;
	}

	QDateTime now = QDateTime::currentDateTime();

	QBuffer *buf = new QBuffer;
	buf->open(QIODevice::ReadWrite);
	buf->write(makeLine(QString("total %1").arg(info.size())));

	foreach(QFileInfo i, info) {
		QFile::Permissions p(i.permissions());
		QString owner = i.owner();
		QString group = i.group();
		QDateTime modified = i.lastModified();

		buf->write(makeLine(QString("%1%2%3%4%5%6%7%8%9%10 %11 %12 %13 %14 %15 %16 %17 %18")
			.arg(i.isDir() ? 'd' : '-')
			.arg(p & QFile::ReadOwner ? 'r' : '-')
			.arg(p & QFile::WriteOwner ? 'w' : '-')
			.arg(p & QFile::ExeOwner ? 'x' : '-')
			.arg(p & QFile::ReadGroup ? 'r' : '-')
			.arg(p & QFile::WriteGroup ? 'w' : '-')
			.arg(p & QFile::ExeGroup ? 'x' : '-')
			.arg(p & QFile::ReadOther ? 'r' : '-')
			.arg(p & QFile::WriteOther ? 'w' : '-')
			.arg(p & QFile::ExeOther ? 'x' : '-')
			.arg(1, 4)
			.arg(owner.size() ? owner : "unknown", -8)
			.arg(group.size() ? group : "unknown", -8)
			.arg(i.size(), 8)
			.arg(month(modified.date().month()))
			.arg(modified.toString("dd"))
			.arg(
				abs(modified.daysTo(now)) < 300
					? modified.toString("HH:mm")
					: modified.toString("yyyy"),
				5)
			.arg(i.fileName())));
	}
	qDebug() << ">>> [" << buf->data().data() << "]";
	buf->reset();
	startDataChannel(buf, true);
}

void neteK::FtpHandler::command_PORT(QString args)
{
	quint16 port = 0;
	{
		QRegExp rx("^\\d+,\\d+,\\d+,\\d+,(\\d+),(\\d+)$");
		if(!rx.exactMatch(args) || 0 == (port = (rx.cap(1).toUInt() * 256) + rx.cap(2).toUInt())) {
			sendLine(501, "Syntax error in parameters or arguments.");
			return;
		}
	}

	if(m_control) {
		setDataChannel(new FtpHandlerPORT(m_control->peerAddress(), port));
		sendLine(200, "Command okay.");
	}
}

void neteK::FtpHandler::command_EPSV(QString args)
{
	if(!m_control)
		return;

	if(args.toUpper() == "ALL") {
		sendLine(200, "Command okay.");
		return;
	} else if(args.size() == 0 || args == "1" /*|| args == "2"*/) {
	} else {
		sendLine(522, "Network protocol not supported, use (1)");
		return;
	}

	quint16 port;
	QPointer<FtpHandlerPASV> handler(new FtpHandlerPASV(m_control->peerAddress(), port));
	if(port == 0) {
		delete handler;

		// this response is not valid according to RFC...
		sendLine(425, "Can't open data connection.");
	} else {
		setDataChannel(handler);
		sendLine(229, QString("Entering Extended Passive Mode (|||%1|)").arg(port));
	}
}

void neteK::FtpHandler::command_PASV(QString)
{
	if(!m_control)
		return;

	quint16 port;
	QPointer<FtpHandlerPASV> handler(new FtpHandlerPASV(m_control->peerAddress(), port));
	if(port == 0) {
		delete handler;

		// this response is not valid according to RFC...
		sendLine(425, "Can't open data connection.");
	} else {
		setDataChannel(handler);
		quint32 addr = m_public.toIPv4Address();
		sendLine(227, QString("Entering Passive Mode (%1,%2,%3,%4,%5,%6).")
				.arg((addr>>24)&0xff)
				.arg((addr>>16)&0xff)
				.arg((addr>>8)&0xff)
				.arg(addr&0xff)
				.arg((port>>8)&0xff)
				.arg(port&0xff));
	}
}


void neteK::FtpHandler::command_MODE(QString args)
{
	if(args == "S")
		sendLine(200, "Command okay.");
	else
		sendLine(504, "Command not implemented for that parameter.");
}

void neteK::FtpHandler::command_NOOP(QString)
{
	sendLine(200, "Command okay.");
}

void neteK::FtpHandler::command_SYST(QString)
{
	sendLine(215, "UNIX Type: L8");
}

void neteK::FtpHandler::command_REST(QString args)
{
	bool ok;
	m_rest = args.toLongLong(&ok);

	if(ok && m_rest >= 0)
		sendLine(350, "Requested file action pending further information.");
	else {
		m_rest = -1;
		sendLine(501, "Syntax error in parameters or arguments.");
	}
}

void neteK::FtpHandler::command_RETR(QString args)
{
	QPointer<QFile> file = m_share->readFile(me(), m_cwd, args, m_rest >= 0 ? m_rest : 0);
	if(file)
		startDataChannel(file, true);
	else
		sendLine(550, "Requested action not taken.");
}

void neteK::FtpHandler::command_STOU(QString)
{
	if(m_rest > 0) {
		// REST unimplemented for STOU, refuse to prevent data corruption
		sendLine(450, "Requested file action not taken.");
		return;
	}

	QString tmp;
	QPointer<QFile> file = m_share->writeFileUnique(me(), m_cwd, tmp);
	if(file) {
		m_store_unique = tmp;
		startDataChannel(file, false);
	} else
		sendLine(450, "Requested file action not taken.");
}


void neteK::FtpHandler::command_STOR(QString args)
{
	if(m_rest > 0) {
		// REST unimplemented for STOR, refuse to prevent data corruption
		sendLine(450, "Requested file action not taken.");
		return;
	}

	QPointer<QFile> file = m_share->writeFile(me(), m_cwd, args);
	if(file)
		startDataChannel(file, false);
	else
		sendLine(450, "Requested file action not taken.");
}

void neteK::FtpHandler::command_APPE(QString args)
{
	if(m_rest >= 0) {
		// REST unimplemented for APPE, refuse to prevent data corruption
		sendLine(450, "Requested file action not taken.");
		return;
	}

	QPointer<QFile> file = m_share->writeFile(me(), m_cwd, args, true);
	if(file)
		startDataChannel(file, false);
	else
		sendLine(450, "Requested file action not taken.");
}

void neteK::FtpHandler::command_DELE(QString args)
{
	if(m_share->deleteFile(me(), m_cwd, args))
		sendLine(250, "Requested file action okay, completed.");
	else
		sendLine(550, "Requested action not taken.");
}

void neteK::FtpHandler::command_RMD(QString args)
{
	if(m_share->deleteFolder(me(), m_cwd, args))
		sendLine(250, "Requested file action okay, completed.");
	else
		sendLine(550, "Requested action not taken.");
}

void neteK::FtpHandler::command_MKD(QString args)
{
	QString resolved;
	if(m_share->resolvePath(m_cwd, args, resolved) && m_share->createFolder(me(), m_cwd, args))
		sendLine(257, QString("%1 created.").arg(quotedPath(resolved)));
	else
		sendLine(550, "Requested action not taken.");
}

QByteArray neteK::FtpHandler::makeLine(QString line)
{
	QByteArray out(m_utf8 ? line.toUtf8() : line.toLatin1());
	{
		QByteArray crnul;
		crnul.append('\r');
		crnul.append('\0');
		out.replace('\r', crnul);
	}
	out.append("\r\n");

	return out;
}

void neteK::FtpHandler::sendLine(QString line)
{
	if(m_control) {
		qDebug() << "-->" << line;
		m_control->write(makeLine(line));
	}
}

void neteK::FtpHandler::sendLine(int code, QString args)
{
	sendLine(QString("%1 %2").arg(code).arg(args));
}

void neteK::FtpHandler::sendLines(int code, QStringList args)
{
	if(args.size()) {
		if(args.size() == 1) {
			sendLine(code, args.at(0));
			return;
		}

		sendLine(QString("%1-%2").arg(code).arg(args.at(0)));

		for(int i=1; i<args.size()-1; ++i)
			sendLine(QString(" %1").arg(args.at(i)));

		sendLine(QString("%1 %2").arg(code).arg(args.last()));
	}
}

void neteK::FtpHandler::dataStartStatus(bool ok)
{
	if(ok) {
		if(m_store_unique.size())
			// RFC says 250, but we are simulating proftpd here
			sendLine(150, QString("FILE: %1").arg(m_store_unique));
		else
			sendLine(150, "File status okay; about to open data connection.");
	} else {
		m_control_channel_blocked = false;
		sendLine(425, "Can't open data connection.");
		closeDataChannel();
	}

	m_store_unique.clear();

	process();
}

void neteK::FtpHandler::dataTransferStatus(bool ok)
{
	if(ok)
		sendLine(226, "Closing data connection.");
	else
		sendLine(426, "Connection closed; transfer aborted.");

	m_control_channel_blocked = false;

	closeDataChannel();

	process();
}

void neteK::FtpHandler::parseLine(QString line, QString &cmd, QString &args)
{
	int space = line.indexOf(' ');
	if(space < 0) {
		cmd = line.toUpper();
	} else {
		cmd = line.left(space).toUpper();
		args = line.mid(space+1);
	}
}


void neteK::FtpHandler::process()
{
	if(m_control && m_control->state() == QAbstractSocket::UnconnectedState) {
		m_control->deleteLater();
		m_control = 0;
	}

	if(!m_control) {
		deleteLater();
		return;
	}

	if(m_control)
		for(;;) {
			char buf[networkBufferSize];
			qint64 read = m_control->read(buf, sizeof(buf));
			if(read < 0) {
				m_control->close();
				return;
			} else if(read == 0)
				break;

			m_control_buffer.append(QByteArray(buf, read));
			if(m_control_buffer.size() > maxFtpLineSize) {
				m_control->close();
				return;
			}
			
			for(;;) {
				int nl = m_control_buffer.indexOf("\r\n");
				if(nl < 0)
					break;
					
				QByteArray line(m_control_buffer.left(nl));
				m_control_buffer = m_control_buffer.mid(nl+2);
				{
					QByteArray crnul;
					crnul.append('\r');
					crnul.append('\0');
					line.replace(crnul, "\r");
				}
				
				QString ln(m_utf8 ? QString::fromUtf8(line) : QString::fromLatin1(line));
				qDebug() << "<--" << ln;
				m_requests.enqueue(ln);
			}
		}

	while(!m_control_channel_blocked && m_requests.size()) {
		QString cmd, args;
		parseLine(m_requests.dequeue(), cmd, args);

		if(m_commands.contains(cmd)) {
			const command &c = m_commands.value(cmd);
			if(!m_loggedin && (c.flags & CommandFlagLoggedIn))
				sendLine(530, "Not logged in.");
			else
				(this->*c.method)(args);

			if(c.method != &FtpHandler::command_REST)
				m_rest = -1;

			if(c.method != &FtpHandler::command_RNFR)
				m_rename_from.clear();
		} else
			sendLine(502, "Command not implemented.");
	}
}

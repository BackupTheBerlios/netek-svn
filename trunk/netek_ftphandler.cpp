#include "netek_share.h"
#include "netek_settings.h"
#include "netek_ftphandler.h"

// TODO 1.0: no space left on disk test
// TODO 1.1: fix socket speed - unbuffered sockets?
// TODO 1.0: ipv6 support
// TODO 1.0: public address support

neteK::FtpHandlerData::FtpHandlerData()
: m_start(false), m_transfer(false), m_to_be_written(0), m_write_size(0), m_send(false)
{
	//qDebug() << __FUNCTION__;
	
	m_buffer.resize(100000);
	
	connect(this, SIGNAL(statusSignal()), SLOT(status()), Qt::QueuedConnection);
	connect(this, SIGNAL(transferSignal()), SLOT(transferEvent()), Qt::QueuedConnection);
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

void neteK::FtpHandlerData::transferEvent()
{
	//qDebug() << __FUNCTION__;
	
	QPointer<QIODevice> source, sink;
	if(sourceSink(source, sink)) {
		if(sink->bytesToWrite())
			return;

		bool close = false;
		for(;;) {
			Q_ASSERT(m_to_be_written >= 0);

			while(m_to_be_written > 0) {
				qint64 written = sink->write(m_buffer.data() + (m_write_size - m_to_be_written), m_to_be_written);
				if(written <= 0) {
					if(written < 0)
						close = true;
					break;
				}

				//qDebug() << "Written" << written;
				Q_ASSERT(written <= m_to_be_written);
				m_to_be_written -= written;
			}

			if(m_to_be_written > 0)
				break;

			qint64 read = source->read(m_buffer.data(), m_buffer.size());
			if(read <= 0) {
				if(read < 0 || !source->isSequential() && source->atEnd())
					close = true;
				break;
			} else {
				//qDebug() << "Read" << read;
				m_write_size = m_to_be_written = read;
				Q_ASSERT(m_write_size <= m_buffer.size());
			}
		}

		if(close) {
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
	connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), SIGNAL(statusSignal()));
	connect(m_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SIGNAL(statusSignal()));
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
			emitTransferStatus(true);
		}
	}
}

neteK::FtpHandlerPASV::FtpHandlerPASV(QHostAddress address, quint16 &port)
: m_address(address)
{
	Settings settings;
	quint16 min(settings.randomTcpPortMin()), max(settings.randomTcpPortMax());

	for(int i=0; i<100; ++i) {
		port = min + rand() % (max-min+1);

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
				// TODO 1.1: log warning, very likely crack attempt
				delete m_socket;
				return;
			}

			m_socket->setParent(this);
			connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), SIGNAL(statusSignal()));
			connect(m_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SIGNAL(statusSignal()));

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
	if(!m_socket)
		return;

	if(m_socket->state() == QAbstractSocket::UnconnectedState)
		emitTransferStatus(true);
}


neteK::FtpHandler::FtpHandler(Share *s, QAbstractSocket *control)
: m_share(s), m_control(control), m_control_channel_blocked(false)
{
	Settings settings;
	
	m_control->setParent(this);

	connect(m_control, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SIGNAL(processSignal()));
	connect(m_control, SIGNAL(error(QAbstractSocket::SocketError)), SIGNAL(processSignal()));
	connect(m_control, SIGNAL(readyRead()), SIGNAL(processSignal()));
	connect(this, SIGNAL(processSignal()), SLOT(process()), Qt::QueuedConnection);

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
	//ADD_COMMAND(SITE, 0);
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

	init();
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
	if(m_share->authenticate(m_username, args)) {
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
	if(m_share->rename(m_cwd, m_rename_from, args))
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
	if(m_control) {
		QStringList feats;
		feats.append("Features:");
		feats.append("MDTM");
		feats.append("REST STREAM");
		feats.append("SIZE");
		feats.append("UTF8");
		feats.append("End");

		sendLines(211, feats);
	}
}

void neteK::FtpHandler::command_OPTS(QString args)
{
	if(args.toUpper() == "UTF8 ON") {
		m_utf8 = true;
		sendLine(200, "Command okay.");
	} else
		sendLine(501, "Syntax error in parameters or arguments.");
}

void neteK::FtpHandler::command_HELP(QString)
{
	if(m_control) {
		QStringList cmds;
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

		sendLines(214, cmds);
	}
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
	} else if(args.size() == 0 || args == "1" || args == "2") {
		// TODO 1.0: is 2 (IPv6) really ok? do we need to bind ipv6 addr. as well?
	} else {
		sendLine(522, "Network protocol not supported, use (1,2)");
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
		// TODO 1.0: better detection of local address? PASV was obsoleted by EPSV anyway...
		quint32 addr = m_control->localAddress().toIPv4Address();
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
	QPointer<QFile> file = m_share->readFile(m_cwd, args, m_rest >= 0 ? m_rest : 0);
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
	QPointer<QFile> file = m_share->writeFileUnique(m_cwd, tmp);
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
	
	QPointer<QFile> file = m_share->writeFile(m_cwd, args);
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
	
	QPointer<QFile> file = m_share->writeFile(m_cwd, args, true);
	if(file)
		startDataChannel(file, false);
	else
		sendLine(450, "Requested file action not taken.");
}

void neteK::FtpHandler::command_DELE(QString args)
{
	if(m_share->deleteFile(m_cwd, args))
		sendLine(250, "Requested file action okay, completed.");
	else
		sendLine(550, "Requested action not taken.");
}

void neteK::FtpHandler::command_RMD(QString args)
{
	if(m_share->deleteFolder(m_cwd, args))
		sendLine(250, "Requested file action okay, completed.");
	else
		sendLine(550, "Requested action not taken.");
}

void neteK::FtpHandler::command_MKD(QString args)
{
	QString resolved;
	if(m_share->resolvePath(m_cwd, args, resolved) && m_share->createFolder(m_cwd, args))
		sendLine(257, QString("%1 created.").arg(quotedPath(resolved)));
	else
		sendLine(550, "Requested action not taken.");
}

QByteArray neteK::FtpHandler::makeLine(QString line)
{
	QByteArray out;
	{
		QByteArray tmp(m_utf8 ? line.toUtf8() : line.toLatin1());
		foreach(char c, tmp)
			if(c == '\r') {
				out.append('\r');
				out.append('\0');
			} else
				out.append(c);
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
		while(m_control->canReadLine()) {
			QString line = QString::fromUtf8(m_control->readLine(500000).constData());
			while(line.endsWith('\r') || line.endsWith('\n'))
				line.chop(1);

			qDebug() << "<--" << line;
			m_requests.enqueue(line);
		}

	while(!m_control_channel_blocked && m_requests.size()) {
		QString cmd, args;
		{
			QString line = m_requests.dequeue();
			int space = line.indexOf(' ');
			if(space == -1) {
				cmd = line.toUpper();
			} else {
				cmd = line.left(space).toUpper();
				args = line.mid(space+1);
			}
		}

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

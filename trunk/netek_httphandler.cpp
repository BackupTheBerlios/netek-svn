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

#include "netek_httphandler.h"
#include "netek_netutils.h"
#include "netek_application.h"
#include "netek_mimetype.h"

static const char g_css[] =
"<style>"
	"body{"
		"background:#fff;"
		"padding:12pt;"
		"margin:0;"
	"}"
	"table{"
		"font-size:10pt;"
		"background:#fff;"
		"color:#000;"
		"border-collapse: collapse;"
		"border-style:solid;"
		"border-width:2px;"
		"border-color:#666;"
	"}"
	"#htr{"
		"background:#666;"
		"color:#fff;"
	"}"
	"#htd{"
		"text-align:center;"
		"padding:6pt 9pt 6pt 9pt;"
	"}"
	"#a{"
		"background:#eee;"
	"}"
	"a{"
		"color:#06c;"
		"padding-left:3pt;"
		"padding-right:3pt;"
		"text-decoration:none;"
	"}"
	"a:hover{"
		"background:#06c;"
		"color:#fff;"
	"}"
	"#file{"
		"text-align:left;"
		"padding:3pt 9pt 3pt 9pt;"
	"}"
	"#size{"
		"text-align:right;"
		"padding:3pt 9pt 3pt 9pt;"
	"}"
	"#type{"
		"text-align:center;"
		"padding:3pt 9pt 3pt 9pt;"
	"}"
	"#mtime{"
		"text-align:center;"
		"padding:3pt 9pt 3pt 9pt;"
	"}"
"</style>";

static const char g_js_filetable[] =
"<script>"
"function removeAllChildren(node){"
	"while(node.hasChildNodes())"
		"node.removeChild(node.firstChild);"
"}"
"function filetable(node){"
	"try{"
		"var id=node.getAttribute('id');"
		"if(id=='mtime'){"
			"var d=new Date(parseInt(node.innerHTML)*1000);"
			"removeAllChildren(node);"
			"node.appendChild(document.createTextNode(d.toLocaleDateString()+' '+d.toLocaleTimeString()));"
		"}else if(id=='size'){"
			"var n=parseInt(node.innerHTML);"
			"if(n>=1024){"
				"var u;"
				"var units=['KB','MB','GB','TB'];"
				"for(var unit in units){"
					"n/=1024;"
					"u=units[unit];"
					"if(n<1024)"
						"break;"
				"}"
				"removeAllChildren(node);"
				"node.appendChild(document.createTextNode(n.toPrecision(3)+' '+u));"
			"}"
		"}"
	"}catch(e){}"
	"for(var i=0; i<node.childNodes.length; ++i)"
		"filetable(node.childNodes.item(i));"
"}"
"filetable(document.body);"
"</script>";

neteK::HttpHandler::HttpHandler(Share *s, QAbstractSocket *sock)
: ProtocolHandler(s, "HTTP", sock->peerAddress()), m_cwd("/"), m_sock(sock), m_state(StateHeader)
{
	m_sock->setParent(this);

	connect(this, SIGNAL(processSignal()), SLOT(process()), Qt::QueuedConnection);

	connect(m_sock, SIGNAL(readyRead()), SIGNAL(processSignal()));
	connect(m_sock, SIGNAL(bytesWritten(qint64)), SIGNAL(processSignal()));
	connect(m_sock, SIGNAL(error(QAbstractSocket::SocketError)), SIGNAL(processSignal()));
	connect(m_sock, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SIGNAL(processSignal()));

	emit processSignal();
}

void neteK::HttpHandler::initResponse_(QHttpResponseHeader &response, int code)
{
#define HTTP_LESS(x1, x2, y1, y2) x1 < y1 || x1 == y1 && x2 < y2

	int major = 1, minor = 1;
	m_connection_close = false;

	if(m_request.isValid()) {
		if(HTTP_LESS(m_request.majorVersion(), m_request.minorVersion(), major, minor)) {
			major = m_request.majorVersion();
			minor = m_request.minorVersion();
		}

		if(m_request.value("Connection").toLower() == "close")
			m_connection_close = true;
	}

	if(HTTP_LESS(major, minor, 1, 1))
		m_connection_close = true;

	response.setStatusLine(code, QString(), major, minor);
	response.addValue("Server", qApp->applicationName());
	response.addValue("Connection", m_connection_close ? "Close" : "Keep-Alive");

#undef HTTP_LESS
}

bool neteK::HttpHandler::sendResponse_(const QHttpResponseHeader &response)
{
	if(m_sock) {
		QByteArray str(response.toString().toUtf8());
		if(str.size() == m_sock->write(str)) {
			qDebug() << "HTTP response"  << str;
			changeState(StateDownload);
			return true;
		} else
			terminate();
	}

	return false;
}

bool neteK::HttpHandler::sendResponse(int code, QString ct, qint64 cl)
{
	QHttpResponseHeader response;
	initResponse_(response, code);
	if(ct.size())
		response.addValue("Content-Type", ct);
	response.addValue("Content-Length", QString::number(cl));
	return sendResponse_(response);
}

bool neteK::HttpHandler::redirectTo(QString loc)
{
	QHttpResponseHeader response;
	initResponse_(response, 303); // TODO: recheck redirect code
	response.addValue("Location", loc);
	response.addValue("Content-Length", "0");
	return sendResponse_(response);
}

bool neteK::HttpHandler::send(const char *buf, qint64 size)
{
	if(m_sock) {
		if(size == m_sock->write(buf, size))
			return true;
		else
			terminate();
	}

	return false;
}

bool neteK::HttpHandler::read()
{
	if(m_sock) {
		char buf[networkBufferSize];
		qint64 ret = m_sock->read(buf, sizeof(buf));
		if(ret > 0) {
			m_buffer.append(QByteArray(buf, ret));
			return true;
		}

		if(ret < 0)
			terminate();
	}

	return false;
}

void neteK::HttpHandler::changeState(State s)
{
	m_state = s;
	emit processSignal();
}

void neteK::HttpHandler::terminate()
{
	changeState(StateNone);

	if(m_sock)
		m_sock->close();
}

void neteK::HttpHandler::process()
{
	if(!m_sock)
		return;

	if(m_sock->state() == QAbstractSocket::UnconnectedState) {
		deleteLater();
		return;
	}
	
	qDebug() << "state:" << m_state;

	switch(m_state) {
		case StateNone:
			break;

		case StateHeader: {
			if(m_download) {
				m_download->deleteLater();
				m_download = 0;
			}

			int sep;
			bool header = false;
			do {
				sep = m_buffer.indexOf("\r\n\r\n");
				if(sep < 0 && m_buffer.size() > 100000)
					terminate();

				if(sep >= 0)
					header = true;
			} while(read());
			
			if(header) {
				m_request = QHttpRequestHeader(QString::fromUtf8(m_buffer.left(sep)));
				m_buffer.remove(0, sep+4);

				if(m_request.isValid()) {
					m_request_url = QUrl::fromEncoded(m_request.path().toUtf8(), QUrl::StrictMode);
					m_content_left = m_request.value("Content-Length").toLongLong();
				}

				if(!m_request.isValid() || !m_request_url.isValid() || m_content_left < 0)
					sendResponse(400);
				else {
					qDebug() << "HTTP request" << m_request.toString().toUtf8();

					QString method = m_request.method().toUpper();
					if(method == "GET") {
						QFileInfo info;
						QString ctype, redirect;
						if(m_share->fileInformation(m_cwd, m_request_url.path(), info)) {
							if(info.isDir()) {
								if(!m_request_url.path().endsWith('/')) {
									redirect = m_request_url.path() + '/';
								} else {
									QFileInfoList infos;
									if(m_share->listFolder(m_cwd, m_request_url.path(), infos)) {
										m_download = new QBuffer(this);
										m_download->open(QIODevice::ReadWrite);

										{
											HtmlPage page(m_download, QString("Location: %1").arg(m_request_url.path()));

											m_download->write(g_css);

											m_download->write(
												"<table><tr id=\"htr\">"
												"<td id=\"htd\">File name</td>"
												"<td id=\"htd\">Size</td>"
												"<td id=\"htd\">Type</td>"
												"<td id=\"htd\">Last modified</td>"
												"</tr>"
												);

											bool alternate = false;

											foreach(QFileInfo i, infos) {
												if(alternate)
													m_download->write("<tr id=\"a\">");
												else
													m_download->write("<tr>");

												m_download->write("<td id=\"file\">");
												m_download->write("<a");
												if(i.isDir())
													writeHtmlAttribute(m_download, "href", i.fileName() + '/');
												else
													writeHtmlAttribute(m_download, "href", i.fileName());
												m_download->write(">");
												writeHtmlEscaped(m_download, i.fileName());
												m_download->write("</a>");
												m_download->write("</td>");

												m_download->write("<td id=\"size\">");
												if(i.isFile())
													writeHtmlEscaped(m_download, QString::number(i.size()));
												m_download->write("</td>");

												m_download->write("<td id=\"type\">");
												if(i.isDir())
													m_download->write("folder");
												else
													writeHtmlEscaped(m_download, getMimeType(i.fileName()));
												m_download->write("</td>");

												m_download->write("<td id=\"mtime\">");
												writeHtmlEscaped(m_download, QString::number(
													i.lastModified().toTime_t()));
												m_download->write("</td>");

												m_download->write("</tr>");

												alternate = !alternate;
											}

											m_download->write("</table>");
										}

										m_download->write(g_js_filetable);

										m_download->reset();
										ctype = "text/html; charset=utf-8";
									}
								}
							} else if(info.isFile()) {
								m_download = m_share->readFile(me(), m_cwd, m_request_url.path());
								m_download->setParent(this);
								ctype = getMimeType(info.fileName());
							}
						}

						if(m_download)
							sendResponse(200, ctype, m_download->size());
						else if(redirect.size())
							redirectTo(redirect);
						else
							sendResponse(404);
					} else
						sendResponse(501); // TODO: recheck no method error code
				}
			}

			break; }

		case StateDownload:
			if(m_sock && m_sock->bytesToWrite())
				;
			else if(!m_download || m_download->atEnd()) {
				if(m_connection_close)
					terminate();
				else
					changeState(StateHeader);
			} else {
				char buf[networkBufferSize];
				qint64 ret = m_download->read(buf, sizeof(buf));
				if(ret > 0)
					send(buf, ret);
				else if(ret < 0)
					terminate();


			}
			break;
	}
}

void neteK::HttpHandler::writeHtmlEscaped(QIODevice *d, QString s)
{
	foreach(char c, s.toUtf8())
		switch(c) {
			case '<': d->write("&lt;"); break;
			case '>': d->write("&gt;"); break;
			case '&': d->write("&amp;"); break;
			default: d->write(&c, 1);
		}
}

void neteK::HttpHandler::writeHtmlAttribute(QIODevice *d, QString k, QString v)
{
	d->write(" ");
	d->write(k.toUtf8());
	d->write("=\"");

	foreach(char c, v.toUtf8())
		switch(c) {
			case '<': d->write("&lt;"); break;
			case '>': d->write("&gt;"); break;
			case '&': d->write("&amp;"); break;
			case '"': d->write("&quot;"); break;
			case '\'': d->write("&#39;"); break;
			default: d->write(&c, 1);
		}

	d->write("\"");
}

neteK::HttpHandler::HtmlPage::HtmlPage(QIODevice *d, QString title)
: m_out(d)
{
	m_out->write("<html>");
	if(title.size()) {
		m_out->write("<head><title>");
		writeHtmlEscaped(m_out, title);
		m_out->write("</title></head>");
	}
	m_out->write("<body>");
}

neteK::HttpHandler::HtmlPage::~HtmlPage()
{
	m_out->write("</body></html>");
}

QString neteK::HttpHandler::getMimeType(QString name)
{
	QString rev;
	foreach(QChar c, name)
		rev.prepend(c);
	const char *type = mimeTypeSearch(rev.toUtf8().data());
	if(type)
		return type;
	return QString();
}

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

// TODO: check html & css with validator
// TODO: implement caching
// TODO: verify HTTP/1.1 method specs
// TODO: content-* headers

static const char g_content_type_html[] = "text/html; charset=utf-8";
static const char g_content_type_xml[] = "text/xml; charset=utf-8";

static const char g_css[] =
"<style type=\"text/css\">"
	"body{"
		"background:#fff;"
		"padding:0;"
		"margin:0;"
		//"font-size:12pt;"
		"text-align:center;"
		"font-family:sans-serif;"
		"color:#000;"
	"}"
	".errorb{"
		"margin-top:72pt;"
		"border-width:4px 0 4px 0;"
		"border-style:solid;"
		"border-color:#c00;"
		"padding:24pt;"
	"}"
	".errort{"
		"color:#c00;"
		"font-size:16pt;"
	"}"
	".errord{"
		"margin-top:12pt;"
	"}"
	".title{"
		"width:100%;"
		"padding:6pt 0 6pt 0;"
		"background:#eee;"
		//"font-size:14pt;"
		"margin-bottom:18pt;"
	"}"
	".filetable{"
		//"font-size:12pt;"
		"background:#fff;"
		"color:#000;"
		"border-collapse: collapse;"
		"border-style:solid;"
		"border-width:2px;"
		"border-color:#666;"
		"margin-top:18pt;"
		"margin-left:auto;"
		"margin-right:auto;"
	"}"
	".htr{"
		"background:#666;"
		"color:#fff;"
		"font-weight:bold;"
	"}"
	".htd{"
		"text-align:center;"
		"padding:6pt 9pt 6pt 9pt;"
	"}"
	".a{"
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
	".select{"
		"text-align:center;"
		"padding:3pt 9pt 3pt 9pt;"
	"}"
	".file{"
		"text-align:left;"
		"padding:3pt 9pt 3pt 9pt;"
	"}"
	".size{"
		"text-align:right;"
		"padding:3pt 9pt 3pt 9pt;"
	"}"
	".type{"
		"text-align:center;"
		"padding:3pt 9pt 3pt 9pt;"
	"}"
	".mtime{"
		"text-align:center;"
		"padding:3pt 9pt 3pt 9pt;"
	"}"
	"#upload{"
		"margin-left:auto;"
		"margin-right:auto;"
	"}"
"</style>";

static const char g_file_upload[] =
"<form action=\"upload\" method=\"post\" enctype=\"multipart/form-data\">"
	"<table id=\"upload\">"
		"<tr>"
			"<td>Upload file:</td>"
			"<td><input type=\"file\" name=\"file\" size=\"45\"/></td>"
			"<td id=\"uploadAction\"><input type=\"submit\" value=\"Upload\"/></td>"
		"</tr>"
	"</table>"
"</form>"
"<script type=\"text/javascript\">"
	"var upload=document.getElementById('upload');"
	"var uploadAction=document.getElementById('uploadAction');"
	"var uploadMore=document.createElement('input');"
	"uploadMore.type='button';"
	"uploadMore.value='More uploads...';"
	"uploadMore.onclick=function(){"
		"for(var i=0;i<5;++i){"
			"var tr=document.createElement('tr');"
			"upload.appendChild(tr);"
			"tr.appendChild(document.createElement('td'));"
			"var td=document.createElement('td');"
			"tr.appendChild(td);"
			"var nfile=document.createElement('input');"
			"nfile.type='file';"
			"nfile.name='file';"
			"nfile.size=45;"
			"td.appendChild(nfile);"
		"}"
	"};"
	"uploadAction.appendChild(document.createTextNode(' '));"
	"uploadAction.appendChild(uploadMore);"
"</script>";

static const char g_js_filetable[] =
"<script type=\"text/javascript\">"
"function removeAllChildren(node){"
	"while(node.hasChildNodes())"
		"node.removeChild(node.firstChild);"
"}"
"function filetable(node){"
	"try{"
		"var id=node.getAttribute('class');"
		"if(id=='mtime'){"
			"var d=new Date(parseInt(node.innerHTML)*1000);"
			"removeAllChildren(node);"
			"node.appendChild(document.createTextNode(d.toLocaleDateString()+' '+d.toLocaleTimeString()));"
		"}else if(id=='size'){"
			"var n=parseInt(node.innerHTML);"
			"if(n>=1024){"
				"var u;"
				"var units=['KiB','MiB','GiB','TiB'];"
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

static const char *g_options_general = "GET,HEAD,POST,OPTIONS";
static const char *g_options_nonexistant = "OPTIONS,MKCOL,PUT";
static const char *g_options_directory = "OPTIONS,GET,HEAD,DELETE,PROPFIND,PROPPATCH,COPY,MOVE";
static const char *g_options_file = "OPTIONS,GET,HEAD,DELETE,PROPFIND,PROPPATCH,COPY,MOVE,PUT";

neteK::HttpHandler::HttpHandler(Share *s, QAbstractSocket *sock)
: ProtocolHandler(s, "HTTP", sock->peerAddress()), m_sock(sock), m_state(StateHeader), m_post_action_delete(0)
{
	m_sock->setParent(this);

	connect(this, SIGNAL(processSignal()), SLOT(process()), Qt::QueuedConnection);

	connect(m_sock, SIGNAL(readyRead()), SIGNAL(processSignal()));
	connect(m_sock, SIGNAL(bytesWritten(qint64)), SIGNAL(processSignal()));
	connect(m_sock, SIGNAL(error(QAbstractSocket::SocketError)), SIGNAL(processSignal()));
	connect(m_sock, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SIGNAL(processSignal()));
	
	connect(this, SIGNAL(processSignal()), SIGNAL(transfer()));

	emit processSignal();
}

bool neteK::HttpHandler::httpVersionLess(int x1, int x2, int y1, int y2)
{
	return x1 < y1 || x1 == y1 && x2 < y2;
}

void neteK::HttpHandler::responseHttpVersion(int &ma, int &mi)
{
	ma = 1;
	mi = 1;
	if(m_request.isValid()) {
		if(httpVersionLess(m_request.majorVersion(), m_request.minorVersion(), ma, mi)) {
			ma = m_request.majorVersion();
			mi = m_request.minorVersion();
		}
	}
}

void neteK::HttpHandler::initResponse_(QHttpResponseHeader &response, int code, qint64 content_length)
{
	int major, minor;
	responseHttpVersion(major, minor);

	m_connection_close = false;
	bool request_get = false;
	
	if(m_request.isValid()) {
		if(m_request.value("connection").toLower() == "close")
			m_connection_close = true;
		
		GetLike g;
		if(isGetLike(m_request.method(), &g) && (g == GET || g == HEAD))
			request_get = true;
	}

	if(httpVersionLess(major, minor, 1, 1))
		m_connection_close = true;

	response.setStatusLine(code, QString(), 1, 1);
	response.addValue("Server", qApp->applicationName());
	response.addValue("Connection", m_connection_close ? "Close" : "Keep-Alive");
	response.addValue("Content-Length", QString::number(content_length));
	
	if(code == 206 || code == 416 || code / 100 == 2 && request_get)
		response.addValue("Accept-Ranges", "bytes");

	if(code == 401)
		response.addValue("WWW-Authenticate", QString("Basic realm=\"%1\"").arg(qApp->applicationName())); // TODO: realm should be user's description
}

bool neteK::HttpHandler::sendResponse_(State nstate, const QHttpResponseHeader &response)
{
	if(m_sock) {
		QByteArray str(response.toString().toUtf8());
		if(str.size() == m_sock->write(str)) {
			qDebug() << "HTTP response"  << str;
			changeState(nstate);
			return true;
		} else
			terminate();
	}

	return false;
}

bool neteK::HttpHandler::sendResponse(State nstate, int code, QString ct, qint64 cl)
{
	QHttpResponseHeader response;
	initResponse_(response, code, cl);
	if(ct.size())
		response.addValue("Content-Type", ct);
	return sendResponse_(nstate, response);
}

bool neteK::HttpHandler::sendCreatedResponse(State nstate, QString loc)
{
	QHttpResponseHeader response;
	initResponse_(response, 201);
	response.addValue("Location", loc);
	return sendResponse_(nstate, response);
}

bool neteK::HttpHandler::sendNoContentResponse(State nstate)
{
	QHttpResponseHeader response;
	initResponse_(response, 204);
	return sendResponse_(nstate, response);
}

bool neteK::HttpHandler::sendPartialResponse(State nstate, QString ct, qint64 full, qint64 from, qint64 to)
{
	QHttpResponseHeader response;
	initResponse_(response, 206, to-from+1);
	if(ct.size())
		response.addValue("Content-Type", ct);
	response.addValue("Content-Range", QString("bytes %1-%2/%3").arg(from).arg(to).arg(full));
	return sendResponse_(nstate, response);
}

bool neteK::HttpHandler::sendOptionsResponse(State nstate, int code, QString allow)
{
	QHttpResponseHeader response;
	initResponse_(response, code);
	response.addValue("Allow", allow);
	return sendResponse_(nstate, response);
}

bool neteK::HttpHandler::sendErrorResponse(State nstate, int code, QString description)
{
	QString error = QString("%1 %2").arg(code).arg(errorDescription(code));
	if(m_download)
		m_download->deleteLater();
	m_download = new QBuffer(this);
	m_download->open(QIODevice::ReadWrite);
	{
		HtmlPage page(m_download, error);
		m_download->write("<div class=\"errorb\"><div class=\"errort\">");
		writeHtmlEscaped(m_download, error);
		m_download->write("</div>");
		if(description.size()) {
			m_download->write("<div class=\"errord\">");
			writeHtmlEscaped(m_download, description);
			m_download->write("</div>");
		}
		m_download->write("</div>");
	}
	m_download->reset();
	
	return sendResponse(nstate, code, g_content_type_html, m_download->size());
}

bool neteK::HttpHandler::redirectTo(State nstate, QString loc)
{
	QHttpResponseHeader response;
	initResponse_(response, 303);
	response.addValue("Location", loc);
	return sendResponse_(nstate, response);
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

void neteK::HttpHandler::handleUrlencodedPost(State next)
{
	if(m_content_left > maxHttpPostSize) {
		terminate();
		return;
	}
	
	m_state_after_urlencodedpost = next;
	changeState(StateUrlencodedPost);
}

void neteK::HttpHandler::terminate()
{
	changeState(StateIgnore);

	if(m_sock)
		m_sock->close();
}

bool neteK::HttpHandler::handlePOST()
{
	QRegExp uploadrx("(.*/)upload");
	if(uploadrx.exactMatch(m_request_url.path())) {
		m_post_upload_dir = uploadrx.cap(1);
		QString ct;
		QMap<QString, QString> values;
		parseContentType(m_request.value("content-type"), ct, values);
		if(ct == "multipart/form-data" && values.value("boundary").size()) {
			m_post_upload_next_boundary = "--" + values.value("boundary").toUtf8() + "\r\n";
			m_post_upload_last_boundary = "\r\n--" + values.value("boundary").toUtf8() + "--\r\n";
			changeState(StateMultipartUpload);
			return true;
		} else {
			return sendErrorResponse(StateSkipContent, 400, "multipart/form-data with boundary expected.");
		}
	}
	
	QRegExp actionrx("(.*/)action");
	if(actionrx.exactMatch(m_request_url.path())) {
		m_post_action_dir = actionrx.cap(1);
		handleUrlencodedPost(StateHandleActionPost);
		return true;
	}
	
	return sendErrorResponse(StateSkipContent, 404);
}

bool neteK::HttpHandler::handleGetLike(GetLike method)
{
	if(method == OPTIONS && m_request_url.path() == "*")
		return sendOptionsResponse(StateSkipContent, 200, g_options_general);
	
	QFileInfo info;
	QString ctype;
	if(m_share->fileInformation(Share::root(), m_request_url.path(), info)) {
		if(info.isDir()) {
			if(!m_request_url.path().endsWith('/')) {
				return redirectTo(StateSkipContent, getLocation(m_request_url.path() + '/'));
			} else if(method == OPTIONS) {
				return sendOptionsResponse(StateSkipContent, 200, g_options_directory);
			} else {
				QFileInfoList infos;
				if(m_share->listFolder(Share::root(), m_request_url.path(), infos)) {
					if(m_download)
						m_download->deleteLater();
					m_download = new QBuffer(this);
					m_download->open(QIODevice::ReadWrite);

					{
						QStringList dirs = m_request_url.path().split('/', QString::SkipEmptyParts);
						
						HtmlPage page(m_download, QString("Location: %1").arg('/' + dirs.join("/")));
						
						if(dirs.size()) {
							m_download->write("<div class=\"title\"><a href=\"/\">Top</a>");
							
							for(int i=0; i<dirs.size(); ++i) {
								m_download->write(" / ");
								
								if(i != dirs.size()-1) {
									m_download->write("<a");
									
									QString rel;
									for(int j=0; j<=i; ++j) {
										rel += '/';
										rel += dirs.at(j);
									}
									
									writeHtmlAttribute(m_download, "href", rel);
									m_download->write(">");
								}
								
								writeHtmlEscaped(m_download, dirs.at(i));
								
								if(i != dirs.size()-1)
									m_download->write("</a>");
							}
							m_download->write("</div>");
						} else {
							m_download->write("<div class=\"title\">Top</div>");
						}
						
						if(m_share->permission() != Share::PermissionRO)
							m_download->write(g_file_upload);

						m_download->write("<form method=\"post\" action=\"action\"><table class=\"filetable\"><tr class=\"htr\">");
						
						if(m_share->permission() == Share::PermissionRW)
							m_download->write("<td class=\"htd\">Select</td>");
						
						m_download->write(
							"<td class=\"htd\">File name</td>"
							"<td class=\"htd\">Size</td>"
							"<td class=\"htd\">Type</td>"
							"<td class=\"htd\">Last modified</td>"
							"</tr>"
							);

						bool alternate = false;

						foreach(QFileInfo i, infos) {
							if(alternate)
								m_download->write("<tr class=\"a\">");
							else
								m_download->write("<tr>");
								
							if(m_share->permission() == Share::PermissionRW) {
								m_download->write("<td class=\"select\"><input type=\"checkbox\"");
								writeHtmlAttribute(m_download, "name", QString("select_%1").arg(i.fileName()));
								m_download->write("/></td>");
							}

							m_download->write("<td class=\"file\">");
							m_download->write("<a");
							if(i.isDir())
								writeHtmlAttribute(m_download, "href", i.fileName() + '/');
							else
								writeHtmlAttribute(m_download, "href", i.fileName());
							m_download->write(">");
							writeHtmlEscaped(m_download, i.fileName());
							m_download->write("</a>");
							m_download->write("</td>");

							m_download->write("<td class=\"size\">");
							if(i.isFile())
								writeHtmlEscaped(m_download, QString::number(i.size()));
							m_download->write("</td>");

							m_download->write("<td class=\"type\">");
							if(i.isDir())
								m_download->write("folder");
							else
								writeHtmlEscaped(m_download, getMimeType(i.fileName()));
							m_download->write("</td>");

							m_download->write("<td class=\"mtime\">");
							writeHtmlEscaped(m_download, QString::number(
								i.lastModified().toTime_t()));
							m_download->write("</td>");

							m_download->write("</tr>");

							alternate = !alternate;
						}

						m_download->write("</table>");
						
						if(m_share->permission() == Share::PermissionRW)
							m_download->write("<p><input type=\"submit\" name=\"delete\" value=\"Delete selected\"/></p>");
							
						m_download->write("</form>");
						
						m_download->write(g_js_filetable);
					}

					m_download->reset();
					ctype = g_content_type_html;
				}
			}
		} else if(info.isFile()) {
			if(method == OPTIONS)
				return sendOptionsResponse(StateSkipContent, 200, g_options_file);
			
			if(m_download)
				m_download->deleteLater();
			m_download = m_share->readFile(me(), Share::root(), m_request_url.path());
			m_download->setParent(this);
			ctype = getMimeType(info.fileName());
		} else if(method == OPTIONS) {
			return sendErrorResponse(StateSkipContent, 503, "Unknown type of file.");
		}
	} else if(method == OPTIONS) {
		return sendOptionsResponse(StateSkipContent, 200, g_options_nonexistant);
	}

	if(m_download) {
		QString range = m_request.value("range");
		if(method == GET && range.size()) {
			qint64 from, to;
			bool ok = true;
			QRegExp rx1("bytes=(\\d+)-(\\d+)"), rx2("bytes=(\\d+)-"), rx3("bytes=-(\\d+)");
			if(rx1.exactMatch(range)) {
				from = rx1.cap(1).toLongLong();
				to = rx1.cap(2).toLongLong();
			} else if(rx2.exactMatch(range)) {
				from = rx2.cap(1).toLongLong();
				to = m_download->size()-1;
			} else if(rx3.exactMatch(range)) {
				from = m_download->size()-rx3.cap(1).toLongLong();
				to = m_download->size()-1;
			} else
				ok = false;
				
			if(ok) {
				if(from < 0 || to < from || to >= m_download->size()) {
					m_download->deleteLater();
					m_download = 0;
					return sendResponse(StateSkipContent, 416);
				}

				if(!m_download->seek(from))
					return sendErrorResponse(StateSkipContent, 503, QString("Could not skip first %1 bytes in response.").arg(from));
				
				m_limit_response_content = to-from+1;
				return sendPartialResponse(StateSkipContent, ctype, m_download->size(), from, to);
			}	
		}
		
		if(method == GET)
			return sendResponse(StateSkipContent, 200, ctype, m_download->size());
		
		m_download->deleteLater();
		m_download = 0;
		return sendResponse(StateSkipContent, 200, ctype);
	}
	
	return sendErrorResponse(StateSkipContent, 404);
}

bool neteK::HttpHandler::handlePUT()
{
	if(m_upload)
		m_upload->deleteLater();
	m_upload = m_share->writeFile(me(), Share::root(), m_request_url.path());
	if(m_upload) {
		changeState(StatePutFile);
		return true;
	}
	
	QFileInfo info;
	if(m_share->fileInformation(Share::root(), m_request_url.path(), info)) {
	       	if(info.isDir())
			return sendOptionsResponse(StateSkipContent, 405, g_options_directory);
	}
	
	QString parent;
	if(m_share->parentPath(Share::root(), m_request_url.path(), parent)
		&& m_share->fileInformation(Share::root(), parent, info))
		return sendErrorResponse(StateSkipContent, 403);
	
	return sendErrorResponse(StateSkipContent, 409);
}

bool neteK::HttpHandler::handleMKCOL()
{
	if(m_share->createFolder(me(), Share::root(), m_request_url.path()))
		return sendCreatedResponse(StateSkipContent, getLocation(m_request_url.path()));
		
	QFileInfo info;
	if(m_share->fileInformation(Share::root(), m_request_url.path(), info)) {
		if(info.isDir())
			return sendOptionsResponse(StateSkipContent, 405, g_options_directory);
		else if(info.isFile())
			return sendOptionsResponse(StateSkipContent, 405, g_options_file);
	}
	
	QString parent;
	if(m_share->parentPath(Share::root(), m_request_url.path(), parent) && m_share->fileInformation(Share::root(), parent, info))
		return sendErrorResponse(StateSkipContent, 403);
	
	return sendErrorResponse(StateSkipContent, 409);
}

int neteK::HttpHandler::depth()
{
	if(m_request.isValid()) {
		QString d = m_request.value("depth").toLower();
		if(d.isEmpty() || d == "infinity")
			return -1;
		bool ok;
		int num = d.toInt(&ok);
		if(ok)
			return num;
	}
	return -2;
}

bool neteK::HttpHandler::overwrite()
{
	if(m_request.isValid()) {
		QString o = m_request.value("overwrite").toLower();
		if(o.isEmpty() || o == "t")
			return true;
	}
	return false;
}

bool neteK::HttpHandler::sendInvalidDepth(State nstate)
{
	return sendErrorResponse(nstate, 400, QString("Invalid depth %1.").arg(m_request.value("depth")));
}

bool neteK::HttpHandler::handleDELETE()
{
	if(depth() != -1)
		return sendInvalidDepth(StateSkipContent);

	QFileInfo info;
	if(!m_share->fileInformation(Share::root(), m_request_url.path(), info))
		return sendErrorResponse(StateSkipContent, 404);

	RecursiveDelete *d = new RecursiveDelete(this, me(), m_share, Share::root(), m_request_url.path());
	connect(d, SIGNAL(done(bool)), SLOT(methodDeleteDone()));
	connect(d, SIGNAL(error(QString)), SLOT(addMultiStatusForbiddenLocation(QString)));
	
	changeState(StateIgnore);

	return true;
}

namespace neteK {

	class WebDAVCopy: public QObject {
		Q_OBJECT;

		QString m_who;
		QPointer<Share> m_share;
		QString m_from, m_to;
		int m_depth;
		bool m_done;

	public:
		WebDAVCopy(QObject *parent, QString who, Share *share, QString from, QString to, int depth, bool del_first)
			: QObject(parent), m_who(who), m_share(share), m_from(from), m_to(to), m_depth(depth), m_done(false)
		{
			if(del_first) {
				RecursiveDelete *d = new RecursiveDelete(this, m_who, m_share, Share::root(), m_to);
				connect(d, SIGNAL(done(bool)), SLOT(startCopy(bool)));
				connect(d, SIGNAL(error(QString)), SIGNAL(error(QString)));
			} else
				startCopy(true);
		}

		~WebDAVCopy()
		{
			if(m_done)
				emit done(m_to);
		}

	private slots:
		void copyDone()
		{
			if(!m_done) {
				m_done = true;
				deleteLater();
			}
		}

		void startCopy(bool del_ok)
		{
			if(!del_ok) {
				if(!m_done) {
					m_done = true;
					deleteLater();
				}
				return;
			}

			RecursiveCopy *c = new RecursiveCopy(this, m_who, m_share, Share::root(), m_from, m_to, m_depth);
			connect(c, SIGNAL(done(bool)), SLOT(copyDone()));
			connect(c, SIGNAL(error(QString)), SIGNAL(error(QString)));
		}

	signals:
		void error(QString);
		void done(QString);
	};
}

bool neteK::HttpHandler::handleCOPY()
{
	int d = depth();
	if(d < -1)
		return sendInvalidDepth(StateSkipContent);

	QString from, to;
	if(!getDestination(to)
		|| !Share::resolvePath(Share::root(), m_request_url.path(), from)
		|| !Share::resolvePath(Share::root(), to, to)
		|| from == to)
		return sendErrorResponse(StateSkipContent, 403);

	QFileInfo info;
	if(!m_share->fileInformation(Share::root(), from, info))
		return sendErrorResponse(StateSkipContent, 404);

	bool del_first = false;
	if(m_share->fileInformation(Share::root(), to, info)) {
		if(!overwrite())
			return sendErrorResponse(StateSkipContent, 412);
		del_first = true;
	}

	WebDAVCopy *c = new WebDAVCopy(this, me(), m_share, from, to, d, del_first);
	connect(c, SIGNAL(done(QString)), SLOT(methodCopyDone(QString)));
	connect(c, SIGNAL(error(QString)), SLOT(addMultiStatusForbiddenLocation(QString)));

	changeState(StateIgnore);

	return true;
}

void neteK::HttpHandler::methodCopyDone(QString to)
{
	if(m_multi_status.documentElement().hasChildNodes()) {
		sendMultiStatusResponse(StateSkipContent);
	} else {
		sendCreatedResponse(StateSkipContent, getLocation(to));
	}
}

void neteK::HttpHandler::methodDeleteDone()
{
	if(m_multi_status.documentElement().hasChildNodes()) {
		sendMultiStatusResponse(StateSkipContent);
	} else {
		sendNoContentResponse(StateSkipContent);
	}
}

void neteK::HttpHandler::addMultiStatusForbiddenLocation(QString file)
{
	addMultiStatusResponse(403, getLocation(file));
}

bool neteK::HttpHandler::sendMultiStatusResponse(State nstate)
{
	if(m_download)
		m_download->deleteLater();
	m_download = new QBuffer(this);
	m_download->open(QIODevice::ReadWrite);
	m_download->write(m_multi_status.toByteArray());
	m_download->reset();
	
	return sendResponse(nstate, 207, g_content_type_xml, m_download->size());
}

void neteK::HttpHandler::addMultiStatusResponse(int code, QString loc)
{
	QDomElement resp = m_multi_status.createElement("D:response");
	QDomElement status = m_multi_status.createElement("D:status");
	QDomElement href = m_multi_status.createElement("D:href");
	m_multi_status.documentElement().appendChild(resp);
	resp.appendChild(status);
	resp.appendChild(href);
	int ma, mi;
	responseHttpVersion(ma, mi);
	status.appendChild(m_multi_status.createTextNode(QString("HTTP/%1.%2 %3").arg(ma).arg(mi).arg(code)));
	href.appendChild(m_multi_status.createTextNode(loc));
}

void neteK::HttpHandler::process()
{
	if(!m_sock)
		return;

	if(m_sock->state() == QAbstractSocket::UnconnectedState) {
		deleteLater();
		return;
	}
	
	if(m_upload) {
		switch(m_state) {
			case StateMultipartUploadBody:
			case StatePutFile:
				break;
			default:
				qDebug() << "Closing upload";
				m_upload->deleteLater();
				m_upload = 0;
				emit processSignal();
				return;
		}
	}
	
	switch(m_state) {
		case StateIgnore:
			qDebug() << "StateIgnore";
			break;

		case StateHeader: {
			qDebug() << "StateHeader";
			
			if(!m_multi_status.hasChildNodes() || m_multi_status.documentElement().hasChildNodes()) {
				m_multi_status = QDomDocument();
				m_multi_status.appendChild(
					m_multi_status.createProcessingInstruction("xml", "version=\"1.0\""));
				QDomElement root = m_multi_status.createElement("D:multistatus");
				root.setAttribute("xmlns:D", "DAV:");
				m_multi_status.appendChild(root);
			}

			m_urlencodedpost.clear();
			m_limit_response_content = -1;
			
			if(m_download) {
				qDebug() << "Closing download";
				m_download->deleteLater();
				m_download = 0;
				emit processSignal();
				return;
			}
			
			int sep;
			bool header = false;
			do {
				sep = m_buffer.indexOf("\r\n\r\n");
				if(sep < 0 && m_buffer.size() > maxHttpHeaderSize)
					terminate();

				if(sep >= 0)
					header = true;
			} while(read());
			
			if(header) {
				qDebug() << "HTTP request" << m_buffer.left(sep);
				m_request = QHttpRequestHeader(QString::fromUtf8(m_buffer.left(sep)));
				m_buffer.remove(0, sep+4);

				if(m_request.isValid()) {
					m_request_url = QUrl::fromEncoded(m_request.path().toUtf8(), QUrl::StrictMode);
					m_content_left = m_request.value("content-length").toLongLong();
				}

				if(m_request.isValid() && m_request_url.isValid() && m_content_left >= 0) {
					QString user, pwd;
					
					QString auth = m_request.value("authorization");
					if(auth.size()) {
						QRegExp rx("basic +(.*)", Qt::CaseInsensitive);
						if(rx.exactMatch(auth)) {
							QString data = QString::fromUtf8(QByteArray::fromBase64(rx.cap(1).toUtf8()));
							int split = data.indexOf(':');
							if(split != -1) {
								user = data.left(split);
								pwd = data.mid(split+1);
							}
						}
					}
					
					if(!m_share->authenticate(me(), user, pwd)) {
						sendErrorResponse(StateSkipContent, 401, "Please, enter correct username and password.");
						return;
					}
					
					QString method = m_request.method().toUpper();

					GetLike glike;
					if(isGetLike(method, &glike))
						handleGetLike(glike);
					else if(method == "PUT")
						handlePUT();
					else if(method == "MKCOL")
						handleMKCOL();
					else if(method == "DELETE")
						handleDELETE();
					else if(method == "COPY")
						handleCOPY();
					else if(method == "POST")
						handlePOST();
					else
						sendErrorResponse(StateSkipContent, 501, QString("Unknown method %1.").arg(method));
				} else {
					qDebug() << "Invalid request";
					sendResponse(StateDownload, 400);
				}
			}

			break; }
			
		case StateSkipContent:
			qDebug() << "StateSkipContent" << m_content_left;
			do {
				if(m_content_left <= 0 || m_buffer.size() >= m_content_left) {
					if(m_content_left > 0) {
						qDebug() << "Skipping" << m_content_left << m_buffer.left(m_content_left);
						m_buffer.remove(0, m_content_left);
					}
					m_content_left = 0;
					changeState(StateDownload);
					return;
				}
				
				qDebug() << "Skipping" << m_buffer.size() << m_buffer;
				
				m_content_left -= m_buffer.size();
				m_buffer.clear();
			} while(read());
			
			break;

		case StateDownload:
			qDebug() << "StateDownload";
			if(m_sock && m_sock->bytesToWrite())
				;
			else if(!m_download || m_download->atEnd() || m_limit_response_content == 0) {
				if(m_connection_close)
					terminate();
				else
					changeState(StateHeader);
			} else {
				char buf[networkBufferSize];
				qint64 ret = m_download->read(buf, sizeof(buf));
				if(ret > 0) {
					if(m_limit_response_content >= 0) {
						if(ret > m_limit_response_content)
							ret = m_limit_response_content;
						m_limit_response_content -= ret;
					}
					send(buf, ret);
				} else if(ret < 0)
					terminate();
			}
			break;
			
		case StateMultipartUpload:
			qDebug() << "StateMultipartUpload" << m_content_left;
			do {
				if(m_content_left < m_post_upload_next_boundary.size()
					|| m_buffer.size() >= m_post_upload_next_boundary.size()
						&& !m_buffer.startsWith(m_post_upload_next_boundary))
				{
					sendErrorResponse(StateSkipContent, 400, "Boundary was not found at the start of the request.");
					return;
				}
				
				if(m_buffer.size() < m_post_upload_next_boundary.size())
					continue;
				
				int end_of_header = doesBufferContain("\r\n\r\n", m_post_upload_next_boundary.size());
				if(end_of_header == -1) {
					if(m_buffer.size() >= maxHttpHeaderSize) {
						terminate();
						return;
					}
						
					continue;
				}
				
				QHttpRequestHeader header(
					QString::fromUtf8(
						"GET / HTTP/1.0\r\n"
						+ m_buffer.mid(
							m_post_upload_next_boundary.size(),
							end_of_header - m_post_upload_next_boundary.size())));
					
				if(!header.isValid()) {
					sendErrorResponse(StateSkipContent, 400, "Cannot parse part header.");
					return;
				}
				
				qDebug() << "Header" << header.toString();
				
				{
					QString cd;
					QMap<QString, QString> data;
					parseContentType(header.value("content-disposition"), cd, data);
					m_post_upload_file = data.value("filename");
					qDebug() << cd << data.value("name") << m_post_upload_file;
					if(cd == "form-data" && data.value("name") == "file" && m_post_upload_file.size()) {
						m_upload = m_share->writeFile(me(), Share::root(),
							m_post_upload_dir + m_post_upload_file);
						if(!m_upload) {
							sendErrorResponse(StateSkipContent, 503,
								QString("Cannot write to file %1.")
									.arg(m_post_upload_file));
							return;
						}
					}
				}
				
				if(!m_post_upload_next_boundary.startsWith("\r\n"))
					m_post_upload_next_boundary.prepend("\r\n");
				
				int remove = end_of_header + 4;
				m_buffer.remove(0, remove);
				m_content_left -= remove;
				changeState(StateMultipartUploadBody);
				
				break;
			} while(read());
			
			break;
			
		case StateMultipartUploadBody:
			qDebug() << "StateMultipartUploadBody" << m_content_left;
			do {
				State next = StateMultipartUploadBody;
				
				int upto = doesBufferContain(m_post_upload_next_boundary);
				if(upto != -1) {
					qDebug() << "Intermediate boundary found at" << upto;
					next = StateMultipartUpload;
				} else {
					upto = doesBufferContain(m_post_upload_last_boundary);
					if(upto != -1) {
						qDebug() << "Last boundary found at" << upto;
						next = StateSkipContent;
					} else {
						upto = qMin(m_content_left, (qint64)m_buffer.size() - m_post_upload_last_boundary.size());
						if(upto <= 0) {
							if(m_buffer.size() >= m_content_left) {
								sendErrorResponse(StateSkipContent, 400, "Multipart request does not end with boundary.");
								return;
							}
							
							continue;
						}
					}
				}
					
				if(upto > 0) {
					if(m_upload) {
						if(upto != m_upload->write(m_buffer, upto)) {
							sendErrorResponse(StateSkipContent, 503,
								QString("Error writing to file %1.")
									.arg(m_post_upload_file));
							return;
						}
					}
					
					m_buffer.remove(0, upto);
					m_content_left -= upto;
				}
				
				if(next == StateSkipContent) {
					if(m_content_left != m_post_upload_last_boundary.size()) {
						sendErrorResponse(StateSkipContent, 400, "Last boundary is not at the end of request.");
						return;
					}
					
					redirectTo(StateSkipContent, getLocation(m_post_upload_dir));
					return;
				}
				
				if(next != StateMultipartUploadBody) {
					changeState(next);
					return;
				}
			} while(read());
			
			break;
			
		case StateUrlencodedPost:
			qDebug() << "StateUrlencodedPost" << m_buffer.size() << m_content_left;
			do {
				if(m_buffer.size() >= m_content_left) {
					m_urlencodedpost = QUrl::fromEncoded("/?" + m_buffer.left(m_content_left), QUrl::StrictMode);
					if(!m_urlencodedpost.isValid()) {
						sendErrorResponse(StateSkipContent, 400, "Cannot parse URL-encoded POST content.");
						return;
					}
					
					m_buffer.remove(0, m_content_left);
					m_content_left = 0;
					changeState(m_state_after_urlencodedpost);
					return;
				}
			} while(read());
			
			break;
			
		case StateHandleActionPost: {
			qDebug() << "StateHandleActionPost";
			m_post_action_delete_ok = true;
			
			QPair<QString, QString> arg;
			foreach(arg, m_urlencodedpost.queryItems()) {
				if(arg.first.startsWith("select_") && arg.second.size()) {
					++m_post_action_delete;
					connect(new RecursiveDelete(this, me(), m_share, m_post_action_dir, arg.first.mid(7)),
						SIGNAL(done(bool)), SLOT(actionDeleteDone(bool)));
				}
			}
			
			if(m_post_action_delete == 0)
				redirectTo(StateSkipContent, getLocation(m_post_action_dir));
			else
				changeState(StateIgnore);
			break; }
			
		case StatePutFile:
			qDebug() << "StatePutFile" << m_content_left;
			for(;;) {
				if(m_content_left <= 0) {
					m_content_left = 0;
					sendCreatedResponse(StateSkipContent, getLocation(m_request_url.path()));
					return;
				}
				
				if(m_buffer.size()) {
					qint64 wr = qMin(qint64(m_buffer.size()), m_content_left);
					if(m_upload && wr != m_upload->write(m_buffer.data(), wr)) {
						sendErrorResponse(StateSkipContent, 503, "Error writing to file.");
						return;
					}
					m_buffer.remove(0, wr);
					m_content_left -= wr;
				} else {
					char buf[networkBufferSize];
					qint64 ret = m_sock->read(buf, sizeof(buf));
					if(ret < 0) {
						terminate();
						return;
					} else if(ret > 0) {
						qint64 wr = qMin(ret, m_content_left);
						if(wr == m_content_left)
							m_buffer.append(QByteArray(buf+wr, ret-wr));
						m_content_left -= wr;
						if(m_upload && wr != m_upload->write(buf, wr)) {
							sendErrorResponse(StateSkipContent, 503, "Error writing to file.");
							return;
						}
					} else
						break;
				}
			}
			
			break;
	}
}

void neteK::HttpHandler::actionDeleteDone(bool ok)
{
	if(!ok)
		m_post_action_delete_ok = false;
	
	if(--m_post_action_delete == 0) {
		if(m_post_action_delete_ok)
			redirectTo(StateSkipContent, getLocation(m_post_action_dir));
		else
			sendErrorResponse(StateSkipContent, 503, "Some files could not be deleted.");
	}
}

int neteK::HttpHandler::doesBufferContain(const QByteArray &a, int from)
{
	int pos = m_buffer.indexOf(a, from);
	if(pos != 1 && pos + a.size() > m_content_left)
		pos = -1;
	return pos;
}

QString neteK::HttpHandler::getLocation(QString path)
{
	QUrl url;
	if(m_request.isValid()) {
		url.setScheme(m_share->URLScheme());
		url.setAuthority(m_request.value("host"));
	}
	url.setPath(path);
	return url.toString();
}

bool neteK::HttpHandler::getDestination(QString &path)
{
	if(m_request.isValid()) {
		QUrl url(m_request.value("destination"), QUrl::StrictMode);
		if(url.scheme() == m_share->URLScheme() && url.authority() == m_request.value("host")) {
			path = url.path();
			return true;
		}
	}
	return false;
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
	m_out->write("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">");
	m_out->write("<html>");
	m_out->write("<head>");
	if(title.size()) {
		m_out->write("<title>");
		writeHtmlEscaped(m_out, title);
		m_out->write("</title>");
	}
	m_out->write(g_css);
	m_out->write("</head>");
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

void neteK::HttpHandler::parseContentType(QString str, QString &ct, QMap<QString, QString> &values)
{
	ct = str;
	
	QRegExp rx("([^;]+)(;.*)");
	if(!rx.exactMatch(str))
		return;
	ct = rx.cap(1);
	str = rx.cap(2);
	
	for(;;) {
		rx.setPattern("; *([^ =]+) *= *\"([^\"]*)\"(.*)");
		if(rx.exactMatch(str)) {
			QString &value = values[rx.cap(1)];
			value = rx.cap(2);
			value.replace("\\\"", "\"");
			str = rx.cap(3);
			continue;
		}
		
		rx.setPattern("; *([^ =]+) *=([^;]*)(.*)");
		if(rx.exactMatch(str)) {
			values[rx.cap(1)] = rx.cap(2);
			str = rx.cap(3);
			continue;
		}
		
		break;
	}
}

QString neteK::HttpHandler::errorDescription(int code)
{
	switch(code) {
		case 404: return "File not found";
		case 401: return "Authorization failure";
	}
	return "Error processing request";
}

bool neteK::HttpHandler::isGetLike(QString method, GetLike *g)
{
	method = method.toUpper();
	if(method == "GET") {
		if(g) *g = GET;
		return true;
	} else if(method == "HEAD") {
		if(g) *g = HEAD;
		return true;
	} else if(method == "OPTIONS") {
		if(g) *g = OPTIONS;
		return true;
	}
	return false;
}

#include "netek_httphandler.moc"

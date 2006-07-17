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

#ifndef __NETEK_HTTPHANDLER_H__
#define __NETEK_HTTPHANDLER_H__

#include <QtNetwork>

#include "netek_share.h"

namespace neteK {

class HttpHandler: public ProtocolHandler {
	Q_OBJECT;

	QString m_cwd;
	QPointer<QAbstractSocket> m_sock;
	QByteArray m_buffer;
	QHttpRequestHeader m_request;
	QUrl m_request_url;
	qint64 m_content_left;
	bool m_connection_close;
	
	QString m_post_upload_dir;
	QByteArray m_post_upload_next_boundary, m_post_upload_last_boundary;

	enum State { StateNone, StateHeader, StateSkipContent, StateDownload, StateMultipartUpload, StateMultipartUploadBody };
	State m_state;
	
	QPointer<QIODevice> m_download, m_upload;

	void initResponse_(QHttpResponseHeader &h, int code);
	bool sendResponse_(State nstate, const QHttpResponseHeader &h);
	bool sendResponse(State nstate, int code, QString content_type = QString(), qint64 content_length = 0);
	bool redirectTo(State nstate, QString loc);
	
	void handleGET();
	void handlePOST();

	bool read();
	bool send(const char *buf, qint64 size);
	void changeState(State s);
	void terminate();
	
	bool bufferMustContain(int &pos, const QByteArray &a, int from = 0);
	int doesBufferContain(const QByteArray &a, int from = 0);

	static QString getMimeType(QString name);

	static void writeHtmlEscaped(QIODevice *d, QString data);
	static void writeHtmlAttribute(QIODevice *d, QString key, QString value);
	static void parseContentType(QString str, QString &ct, QMap<QString, QString> &values);

	class HtmlPage {
			QPointer<QIODevice> m_out;

		public:
			HtmlPage(QIODevice *d, QString title);
			~HtmlPage();
	};

signals:
	void processSignal();

private slots:
	void process();
	
public:
	HttpHandler(Share *s, QAbstractSocket *sock);
};

}

#endif

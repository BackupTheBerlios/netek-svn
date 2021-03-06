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
#include <QtXml>

#include "netek_share.h"

namespace neteK {

class HttpHandler: public ProtocolHandler {
	Q_OBJECT;

	QPointer<QAbstractSocket> m_sock;
	QByteArray m_buffer;
	QHttpRequestHeader m_request;
	QUrl m_request_url;
	qint64 m_content_left, m_limit_response_content;
	bool m_connection_close;
	
	QString m_post_upload_dir;
	QString m_post_upload_file;
	QByteArray m_post_upload_next_boundary, m_post_upload_last_boundary;

	enum State { StateIgnore, StateHeader, StateSkipContent, StateDownload, StateUrlencodedPost,
		StateMultipartUpload, StateMultipartUploadBody, StateHandleActionPost, StatePutFile };
	State m_state, m_state_after_urlencodedpost;
	
	QUrl m_urlencodedpost;
	
	QString m_post_action_dir;
	bool m_post_action_delete_ok;
	int m_post_action_delete;

	QPointer<QIODevice> m_download, m_upload;

	QDomDocument m_multi_status;
	void addMultiStatusResponse(int code, QString loc);

	static bool httpVersionLess(int x1, int x2, int y1, int y2);
	void responseHttpVersion(int &ma, int &mi);
	void initResponse_(QHttpResponseHeader &h, int code, qint64 content_length = 0);
	bool sendResponse_(State nstate, const QHttpResponseHeader &h);
	bool sendResponse(State nstate, int code, QString content_type = QString(), qint64 content_length = 0);
	bool sendPartialResponse(State nstate, QString content_type, qint64 full_size, qint64 from, qint64 to);
	bool sendOptionsResponse(State nstate, int code, QString allow);
	bool sendCreatedResponse(State nstate, QString loc);
	bool sendNoContentResponse(State nstate);
	bool sendErrorResponse(State nstate, int code, QString description = "");
	bool sendInvalidDepth(State nstate);
	bool redirectTo(State nstate, QString loc);
	bool sendMultiStatusResponse(State nstate);

	enum GetLike { GET, HEAD, OPTIONS };
	bool isGetLike(QString method, GetLike *g = 0);
	
	bool handleGetLike(GetLike g);
	bool handlePOST();
	bool handleMKCOL();
	bool handlePUT();
	bool handleCOPY();
	bool handleMOVE();
	bool handleDELETE();

	bool read();
	bool send(const char *buf, qint64 size);
	void changeState(State s);
	void handleUrlencodedPost(State next);
	void terminate();
	
	bool bufferMustContain(int &pos, const QByteArray &a, int from = 0);
	int doesBufferContain(const QByteArray &a, int from = 0);
	QString getLocation(QString path = "");
	int depth();
	bool overwrite();
	bool getDestination(QString &path);

	static QString getMimeType(QString name);

	static void writeHtmlEscaped(QIODevice *d, QString data);
	static void writeHtmlAttribute(QIODevice *d, QString key, QString value);
	static void parseContentType(QString str, QString &ct, QMap<QString, QString> &values);
	static QString errorDescription(int code);

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
	void actionDeleteDone(bool ok);

	void addMultiStatusForbiddenLocation(QString);
	void methodDeleteDone();
	void methodCopyDone(QString);
	
public:
	HttpHandler(Share *s, QAbstractSocket *sock);
};

}

#endif

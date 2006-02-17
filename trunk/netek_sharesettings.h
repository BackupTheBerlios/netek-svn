#ifndef __NETEK_SHARESETTINGS_H__
#define __NETEK_SHARESETTINGS_H__

#include "ui_netek_sharesettings.h"

#include <QtNetwork>

namespace neteK {

class Share;

class ShareSettings: public QDialog {
		Q_OBJECT;

		QPointer<Share> m_share;
		QPointer<QTcpServer> m_test_server;

	public slots:
		virtual void accept();
		void pickRandom();
		void securityRadio();
		void folderBrowse();

	public:
		ShareSettings(Share *share);

	private:
		Ui::ShareSettings ui;
};

}

#endif

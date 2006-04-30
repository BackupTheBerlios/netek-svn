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

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

#ifndef __NETEK_SETTINGS_H__
#define __NETEK_SETTINGS_H__

#include <QtGui>

namespace neteK {

class Settings: public QSettings {
public:
	quint16 randomTcpPortMin();
	void setRandomTcpPortMin(quint16 port);

	quint16 randomTcpPortMax();
	void setRandomTcpPortMax(quint16 port);
	
	int logKBytes();
	void setLogKBytes(int kb);
	
	enum PublicAddress { PublicAddressAutodetect, PublicAddressManual };
	
	PublicAddress publicAddress();
	void setPublicAddress(PublicAddress a);
	
	QString customPublicAddress();
	void setCustomPublicAddress(QString addr);
	
	QRect guiGeometry();
	void setGuiGeometry(QRect rect);
	
	QRect logViewerGeometry();
	void setLogViewerGeometry(QRect rect);
	
	QList<int> guiShareListColumns();
	void setGuiShareListColumns(QList<int> slc);
	
	bool ftpUseUnicodeByDefault();
	void setFtpUseUnicodeByDefault(bool yes);
	
	bool ftpAllowPassive();
	void setFtpAllowPassive(bool yes);
};

}

#endif

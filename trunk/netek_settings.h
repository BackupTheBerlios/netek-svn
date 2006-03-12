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
	
	enum PublicAddress { PublicAddressAutodetect, PublicAddressManual };
	
	PublicAddress publicAddress();
	void setPublicAddress(PublicAddress a);
	
	QString customPublicAddress();
	void setCustomPublicAddress(QString addr);
	
	QRect guiGeometry();
	void setGuiGeometry(QRect rect);
	
	QList<int> guiShareListColumns();
	void setGuiShareListColumns(QList<int> slc);
	
	bool ftpUseUnicodeByDefault();
	void setFtpUseUnicodeByDefault(bool yes);
	
	bool ftpAllowPassive();
	void setFtpAllowPassive(bool yes);
};

}

#endif

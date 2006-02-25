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
	
	QString publicAddress();
	void setPublicAddress(QString addr);
	
	QRect guiGeometry();
	void setGuiGeometry(QRect rect);
};

}

#endif

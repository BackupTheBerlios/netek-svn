#ifndef __NETEK_NETUTILS_H__
#define __NETEK_NETUTILS_H__

#include <QtCore>
#include <QtNetwork>

namespace neteK {

	void resolvePublicAddress(QHostAddress def, QObject *, const char *slot);
	bool isPublicNetwork(QHostAddress addr);

}

#endif

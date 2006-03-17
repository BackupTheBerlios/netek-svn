#ifndef __NETEK_NETUTILS_H__
#define __NETEK_NETUTILS_H__

#include <QtCore>
#include <QtNetwork>

namespace neteK {

	void resolvePublicAddress(QHostAddress def, QObject *, const char *slot);

	bool isPrivateNetwork(QHostAddress addr);
	bool isLoopback(QHostAddress addr);
	bool isOtherNonPublic(QHostAddress addr);
	bool isPublicNetwork(QHostAddress addr);

	bool networkInterfaces(QList<QPair<QString, QHostAddress> > &nifs);

	quint16 randomPort();
}

#endif

#include "netek_netutils.h"
#include "netek_settings.h"

// TODO 1.0: provide settings for autodetect timeouts...?
// TODO 1.0: more tests for autodetection

#ifdef Q_OS_UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#endif

#ifdef Q_OS_WIN32
#include <windows.h>
#include <ws2tcpip.h>
#endif

namespace neteK {

struct AutoDetectCache {
	QHostAddress address;
	QDateTime timeout;
};

class PublicAddressDetector: public QObject {
	Q_OBJECT;

	bool m_detected;
	QHostAddress m_default;

	bool m_auto;
	QPointer<QBuffer> m_auto_buf;
	QPointer<QHttp> m_auto_http;
	int m_auto_get;

	static AutoDetectCache *m_auto_cache;
	static QObject *m_auto_detector;

	void emitDetected(QHostAddress addr)
	{
		if(!m_detected) {
			qDebug() << "Public address:" << addr.toString();

			m_detected = true;

			if(m_auto) {
				Q_ASSERT(m_auto_detector == this);
				m_auto_detector = 0;
			}

			emit detected(addr);
			deleteLater();
		}
	}

private slots:
	void emitDefault()
	{
		emitDetected(m_default);
	}

	void hostInfo(QHostInfo info)
	{
		QList<QHostAddress> addrs = info.addresses();
		if(addrs.size())
			emitDetected(addrs.at(0));
		else {
			qWarning() << "Lookup failed";
			emitDefault();
		}
	}

	void autoFinished(int id, bool error)
	{
		if(id == m_auto_get) {
			delete m_auto_cache;
			m_auto_cache = new AutoDetectCache;

			if(!error && m_auto_http->lastResponse().isValid() && m_auto_http->lastResponse().statusCode() / 100 == 2) {
				m_auto_buf->reset();
				if(m_auto_buf->canReadLine()) {
					QHostAddress addr;
					if(addr.setAddress(QString::fromUtf8(m_auto_buf->readLine(1000)))) {
						m_auto_cache->address = addr;
						m_auto_cache->timeout = QDateTime::currentDateTime().addSecs(300);
						emitDetected(addr);
						return;
					}
				}
			}

			qWarning() << "Autodetect failed, using default address for 15 seconds";
			m_auto_cache->timeout = QDateTime::currentDateTime().addSecs(15);
			emitDefault();
		}
	}

	void checkAuto()
	{
		if(m_auto_detector) {
			qWarning() << "Scheduling autodetection into queue";
			connect(m_auto_detector, SIGNAL(destroyed()), SLOT(checkAuto()), Qt::QueuedConnection);
			return;
		}

		m_auto_detector = this;

		if(m_auto_cache && m_auto_cache->timeout < QDateTime::currentDateTime()) {
			delete m_auto_cache;
			m_auto_cache = 0;
		}

		if(m_auto_cache) {
			if(m_auto_cache->address.isNull())
				emitDefault();
			else
				emitDetected(m_auto_cache->address);
		} else {
			qDebug() << "Performing autodetect";
			m_auto_buf = new QBuffer(this);
			m_auto_buf->open(QIODevice::ReadWrite);
			m_auto_http = new QHttp(this);
			connect(m_auto_http, SIGNAL(requestFinished(int, bool)), SLOT(autoFinished(int, bool)));
			m_auto_http->setHost("netek.berlios.de");
			m_auto_get = m_auto_http->get("/cgi-bin/myip", m_auto_buf);
		}
	}

public:
	PublicAddressDetector(QHostAddress addr)
	: m_detected(false), m_default(addr), m_auto(false)
	{ }

	void start()
	{
		QTimer::singleShot(15000, this, SLOT(emitDefault()));

		Settings settings;
		if(settings.publicAddress() == Settings::PublicAddressManual) {
			QString custom = settings.customPublicAddress();
			QHostAddress addr;
			if(addr.setAddress(custom))
				emitDetected(addr);
			else {
				qDebug() << "Performing lookup:" << custom;
				QHostInfo::lookupHost(custom, this, SLOT(hostInfo(QHostInfo)));
			}
		} else {
			m_auto = true;
			checkAuto();
		}
	}

signals:
	void detected(QHostAddress);
};

}

neteK::AutoDetectCache *neteK::PublicAddressDetector::m_auto_cache = 0;
QObject *neteK::PublicAddressDetector::m_auto_detector = 0;

void neteK::resolvePublicAddress(QHostAddress def, QObject *rec, const char *slot)
{
	QPointer<PublicAddressDetector> det = new PublicAddressDetector(def);
	QObject::connect(det, SIGNAL(detected(QHostAddress)), rec, slot);
	det->start();
}

bool neteK::isLoopback(QHostAddress addr)
{
	if(addr.protocol() == QAbstractSocket::IPv4Protocol) {
		return (addr.toIPv4Address() & 0xff000000) == 0x7f000000;
	}

	return false;
}

bool neteK::isOtherNonPublic(QHostAddress addr)
{
	if(addr.protocol() == QAbstractSocket::IPv4Protocol) {
		return (addr.toIPv4Address() & 0xffff0000) == 0xa9fe0000;
	}

	return false;
}

bool neteK::isPrivateNetwork(QHostAddress addr)
{
	if(addr.protocol() == QAbstractSocket::IPv4Protocol) {
		quint32 a = addr.toIPv4Address();
		return
			((a & 0xff000000) == 0x0a000000) ||
			((a & 0xfff00000) == 0xac100000) ||
			((a & 0xffff0000) == 0xc0a80000);
	}

	return false;
}

bool neteK::isPublicNetwork(QHostAddress addr)
{
	return
		!isLoopback(addr)
		&& !isOtherNonPublic(addr)
		&& !isPrivateNetwork(addr);
}

bool neteK::networkInterfaces(QList<QPair<QString, QHostAddress> > &nifs)
{
	bool ret = false;

#ifdef Q_OS_UNIX
	ifaddrs *ifa;
	if(0 == getifaddrs(&ifa)) {
		ret = true;

		for(ifaddrs *nif = ifa; nif != 0; nif = nif->ifa_next) {
			if(nif->ifa_name && nif->ifa_addr) {
				QHostAddress addr;
				addr.setAddress(nif->ifa_addr);

				if(!addr.isNull())
					nifs.append(qMakePair(QString(nif->ifa_name), addr));
			}
		}

		freeifaddrs(ifa);
	}
#endif

#ifdef Q_OS_WIN32
	int afs[] = {AF_INET, AF_INET6};
	for(int af = 0; af < 2; ++af) {
		SOCKET sock = ::socket(afs[af], SOCK_STREAM, 0);
		if(sock != INVALID_SOCKET) {
			INTERFACE_INFO info[512];
			DWORD bufsize;
			if(SOCKET_ERROR != ::WSAIoctl(sock, SIO_GET_INTERFACE_LIST, 0, 0, &info, sizeof(info), &bufsize, 0, 0)) {
				ret = true;
				int num = bufsize/sizeof(INTERFACE_INFO);
				for(int i=0; i<num; ++i) {
					INTERFACE_INFO *ifa = info + i;

					QHostAddress addr;
					addr.setAddress(&(ifa->iiAddress.Address));

					if(!addr.isNull())
						nifs.append(qMakePair(QString("if%1").arg(i), addr));
				}
			}

			::closesocket(sock);
		}
	}
#endif

	return ret;
}

quint16 neteK::randomPort()
{
	// maybe we should use something more serious here?
	quint16 rnd = rand();
	rnd *= rand();
	rnd += rand();

	Settings settings;
	quint16 min(settings.randomTcpPortMin()), max(settings.randomTcpPortMax());
	return min + rnd % (max-min+1);
}

#include "netek_netutils.moc"

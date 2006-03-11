#include "netek_netutils.h"
#include "netek_settings.h"

// TODO 1.0: provide settings for autodetect timeouts...?
// TODO 1.0: more test for autodetection

namespace neteK {

struct AutoDetectCache {
	QHostAddress address;
	QDateTime timeout;
};

class PublicAddressDetector: public QObject {
	Q_OBJECT;
	
	bool m_detected;
	QHostAddress m_default;
	
	QPointer<QBuffer> m_auto_buf;
	QPointer<QHttp> m_auto_http;
	int m_auto_get;
	
	static AutoDetectCache *m_auto_cache;
	
	void emitDetected(QHostAddress addr)
	{
		if(!m_detected) {
			qDebug() << "Public address:" << addr.toString();
			
			m_detected = true;
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
						m_auto_cache->timeout = QDateTime::currentDateTime().addSecs(900);
						emitDetected(addr);
						return;
					}
				}
			}
			
			qWarning() << "Autodetect failed, using default address for 30 seconds";
			m_auto_cache->timeout = QDateTime::currentDateTime().addSecs(30);
			emitDefault();
		}
	}
		
public:
	PublicAddressDetector(QHostAddress addr)
	: m_detected(false), m_default(addr)
	{ }
	
	void start()
	{
		QTimer::singleShot(20000, this, SLOT(emitDefault()));
		
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
	}

signals:
	void detected(QHostAddress);
};

}

neteK::AutoDetectCache *neteK::PublicAddressDetector::m_auto_cache = 0;

void neteK::resolvePublicAddress(QHostAddress def, QObject *rec, const char *slot)
{
	QPointer<PublicAddressDetector> det = new PublicAddressDetector(def);
	QObject::connect(det, SIGNAL(detected(QHostAddress)), rec, slot);
	det->start();
}

bool neteK::isPublicNetwork(QHostAddress addr)
{
	if(addr.protocol() != QAbstractSocket::IPv4Protocol)
		return true;
		
	quint32 a = addr.toIPv4Address();
	return
		(a & 0xff000000) != 0x0a000000 &&
		(a & 0xfff00000) != 0xac100000 &&
		(a & 0xffff0000) != 0xc0a80000 &&
		(a & 0xffff0000) != 0xa9fe0000 &&
		(a & 0xff000000) != 0x7f000000;
}

#include "netek_netutils.moc"

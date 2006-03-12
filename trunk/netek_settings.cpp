#include "netek_settings.h"

quint16 neteK::Settings::randomTcpPortMin()
{ return value("randomTcpPort/min", 30000).toUInt(); }

void neteK::Settings::setRandomTcpPortMin(quint16 port)
{ setValue("randomTcpPort/min", port); }

quint16 neteK::Settings::randomTcpPortMax()
{ return value("randomTcpPort/max", 30999).toUInt(); }

void neteK::Settings::setRandomTcpPortMax(quint16 port)
{ setValue("randomTcpPort/max", port); }

neteK::Settings::PublicAddress neteK::Settings::publicAddress()
{ return (PublicAddress)value("publicAddress", PublicAddressAutodetect).toInt(); }

void neteK::Settings::setPublicAddress(PublicAddress a)
{ setValue("publicAddress", a); }

QString neteK::Settings::customPublicAddress()
{ return value("customPublicAddress").toString(); }

void neteK::Settings::setCustomPublicAddress(QString addr)
{ setValue("customPublicAddress", addr); }

void neteK::Settings::setGuiGeometry(QRect rect)
{ setValue("gui/geometry", rect); }

QRect neteK::Settings::guiGeometry()
{ return value("gui/geometry").toRect(); }

QList<int> neteK::Settings::guiShareListColumns()
{
	QList<int> slc;
	foreach(QVariant c, value("gui/shareListColumns").toList())
		slc.append(c.toInt());
	return slc;
}

void neteK::Settings::setGuiShareListColumns(QList<int> slc_)
{
	QList<QVariant> slc;
	foreach(int c, slc_)
		slc.append(QVariant(c));
	setValue("gui/shareListColumns", slc);
}

bool neteK::Settings::ftpAllowPassive()
{ return value("ftp/allowPassive", true).toBool(); }

void neteK::Settings::setFtpAllowPassive(bool yes)
{ setValue("ftp/allowPassive", yes); }

bool neteK::Settings::ftpUseUnicodeByDefault()
{ return value("ftp/useUnicodeByDefault", true).toBool(); }

void neteK::Settings::setFtpUseUnicodeByDefault(bool yes)
{ setValue("ftp/useUnicodeByDefault", yes); }


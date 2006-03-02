#include "netek_settings.h"

quint16 neteK::Settings::randomTcpPortMin()
{ return value("randomTcpPort/min", 30000).toUInt(); }

void neteK::Settings::setRandomTcpPortMin(quint16 port)
{ setValue("randomTcpPort/min", port); }

quint16 neteK::Settings::randomTcpPortMax()
{ return value("randomTcpPort/max", 30999).toUInt(); }

void neteK::Settings::setRandomTcpPortMax(quint16 port)
{ setValue("randomTcpPort/max", port); }

QString neteK::Settings::publicAddress()
{ return value("publicAddress").toString(); }

void neteK::Settings::setPublicAddress(QString addr)
{ setValue("publicAddress", addr); }

void neteK::Settings::setGuiGeometry(QRect rect)
{ setValue("gui/geometry", rect); }

QRect neteK::Settings::guiGeometry()
{ return value("gui/geometry").toRect(); }

bool neteK::Settings::ftpAllowPassive()
{ return value("ftp/allowPassive", true).toBool(); }

void neteK::Settings::setFtpAllowPassive(bool yes)
{ setValue("ftp/allowPassive", yes); }

bool neteK::Settings::ftpUseUnicodeByDefault()
{ return value("ftp/useUnicodeByDefault", true).toBool(); }

void neteK::Settings::setFtpUseUnicodeByDefault(bool yes)
{ setValue("ftp/useUnicodeByDefault", yes); }


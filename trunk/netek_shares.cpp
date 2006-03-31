#include "netek_share.h"
#include "netek_shares.h"
#include "netek_settings.h"

neteK::Shares::Shares()
{
	Settings settings;
	int shares = settings.beginReadArray("shares");
	for(int i=0; i<shares; ++i) {
		settings.setArrayIndex(i);

		QPointer<Share> s = new Share;
		m_shares.append(s);
		initShare(s);

		s->setFolder(settings.value("folder").toString());
		s->setPort(settings.value("port").toUInt());
		s->setRunStatus(settings.value("run").toBool());
		s->setReadOnly(settings.value("readOnly").toBool());
		s->setAccess((neteK::Share::Access)settings.value("access").toInt());
		s->setUsernamePassword(
				settings.value("username").toString(),
				settings.value("password").toString());
				
		s->checkServer();
	}
	settings.endArray();
}

neteK::Share *neteK::Shares::share(int i) const
{
	return i < 0 || i >= m_shares.size() ? 0 : m_shares.at(i);
}


int neteK::Shares::createShareWithSettings(QString path)
{
	QPointer<Share> s = new Share;
	s->setFolder(path);
	if(s->showSettings()) {
		int ret = m_shares.size();
		m_shares.append(s);
		initShare(s);
		s->startIfStopped();
		settingsChanged();
		return ret;
	}

	s->deleteLater();

	return -1;
}

bool neteK::Shares::deleteShareWithQuestion(int i)
{
	QPointer<Share> sh = share(i);
	if(sh && QMessageBox::Yes == QMessageBox::question(0,
				qApp->applicationName(),
				tr("Do you really want to delete share %1?").arg(sh->niceId()),
				QMessageBox::Yes,
				QMessageBox::No | QMessageBox::Default))
	{
		if(-1 != (i == m_shares.indexOf(sh))) {
			m_shares.removeAt(i);
			sh->deleteLater();
			settingsChanged();
			return true;
		}
	}

	return false;
}

void neteK::Shares::initShare(Share *s)
{
	s->setParent(this);

	connect(s, SIGNAL(settingsChanged()), SLOT(settingsChanged()));
	connect(s, SIGNAL(statusChanged()), SIGNAL(changed()));
}

void neteK::Shares::settingsChanged()
{
	Settings settings;
	settings.remove("shares");
	settings.beginWriteArray("shares");
	for(int i=0; i<m_shares.size(); ++i) {
		settings.setArrayIndex(i);
		QPointer<Share> sh = m_shares.at(i);
		settings.setValue("folder", sh->folder());
		settings.setValue("port", sh->port());
		settings.setValue("run", sh->runStatus());
		settings.setValue("readOnly", sh->readOnly());
		settings.setValue("access", sh->access());

		{
			QString u,p;
			sh->usernamePassword(u,p);
			settings.setValue("username", u);
			settings.setValue("password", p);
		}
	}
	settings.endArray();

	emit changed();
}

int neteK::Shares::shares() const
{
	return m_shares.size();
}

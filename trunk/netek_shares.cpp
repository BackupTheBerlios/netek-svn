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

#include "netek_share.h"
#include "netek_shares.h"
#include "netek_settings.h"

neteK::Shares::Shares()
{
	connect(qApp, SIGNAL(command_createShare(QString)), SLOT(createShareResolvePath(QString)));
	
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
	s->setFolder(QDir::convertSeparators(path));
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
	connect(s, SIGNAL(transfer()), SIGNAL(transfer()));
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

bool neteK::Shares::resolveAnyPath(QString file, QString &path)
{
	return resolveLocalPath(QUrl(file, QUrl::StrictMode).toLocalFile(), path)
		|| resolveLocalPath(file, path);
}

bool neteK::Shares::resolveLocalPath(QString file, QString &path)
{
	if(file.size() == 0)
		return false;
		
	qDebug() << "resolveLocalPath is file?" << file;
	QFileInfo info1(file);
	if(info1.isFile())
		file = info1.path();
	
	qDebug() << "resolveLocalPath is directory?" << file;
	QFileInfo info2(file);
	if(info2.isDir()) {
		file = info2.canonicalFilePath();
		if(file.size()) {
			path = file;
			qDebug() << "Resolved:" << path;
			return true;
		}
	}
	
	return false;
}

int neteK::Shares::createShareResolvePath(QString path)
{
	return
		resolveAnyPath(path, path)
			? createShareWithSettings(path)
			: -1;
}

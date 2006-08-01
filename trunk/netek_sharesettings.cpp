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

#include "netek_settings.h"
#include "netek_share.h"
#include "netek_sharesettings.h"
#include "netek_netutils.h"

neteK::ShareSettings::ShareSettings(Share *share)
: m_share(share)
{
	ui.setupUi(this);

	connect(ui.folderBrowse, SIGNAL(clicked()), SLOT(folderBrowse()));
	connect(ui.portPickRandom, SIGNAL(clicked()), SLOT(pickRandom()));

	ui.folder->setText(m_share->folder());

	if(m_share->port() == 0) {
		Settings settings;
		ui.port->setValue(settings.randomTcpPortMin());
		pickRandom();
	} else
		ui.port->setValue(m_share->port());

	ui.type->addItem(Share::niceTypeLong(Share::TypeHTTP));
	ui.type->addItem(Share::niceTypeLong(Share::TypeFTP));
	ui.type->setCurrentIndex(m_share->type() == Share::TypeFTP ? 1 : 0);

	ui.downloadOnly->setChecked(m_share->permission() == Share::PermissionRO);
	ui.dropbox->setChecked(m_share->permission() == Share::PermissionDropbox);
	ui.fullAccess->setChecked(m_share->permission() == Share::PermissionRW);
	
	ui.anonymous->setChecked(m_share->access() == Share::AccessAnonymous);
	ui.authentication->setChecked(m_share->access() == Share::AccessUsernamePassword);
	connect(ui.anonymous, SIGNAL(clicked()), SLOT(securityRadio()));
	connect(ui.authentication, SIGNAL(clicked()), SLOT(securityRadio()));

	{
		QString u, p;
		m_share->usernamePassword(u,p);
		ui.username->setText(u);
		ui.password->setText(p);
	}

	securityRadio();
}

void neteK::ShareSettings::securityRadio()
{
	bool enabled = ui.authentication->isChecked();

	ui.username->setEnabled(enabled);
	ui.password->setEnabled(enabled);
	ui.usernameLabel->setEnabled(enabled);
	ui.passwordLabel->setEnabled(enabled);
}

void neteK::ShareSettings::folderBrowse()
{
	QString name = QDir::convertSeparators(QFileDialog::getExistingDirectory(this, tr("Select share folder"), ui.folder->text()));
	if(name.size())
		ui.folder->setText(name);
}

void neteK::ShareSettings::pickRandom()
{
	for(int i=0; i<100; ++i) {
		quint16 port = randomPort();

		if(m_test_server)
			delete m_test_server;
		m_test_server = new QTcpServer(this);

		if(m_test_server->listen(QHostAddress::Any, port)) {
			ui.port->setValue(port);
			return;
		}
	}

	QMessageBox::warning(this, qApp->applicationName(), tr("Unable to bind any TCP port. Check random TCP port settings."), QMessageBox::Cancel, 0, 0);
}

void neteK::ShareSettings::accept()
{
	if(ui.folder->text().size() == 0) {
		QMessageBox::information(this, qApp->applicationName(), tr("Folder name must be entered."), QMessageBox::Cancel);
		return;
	}

	if(ui.authentication->isChecked() &&
		(ui.username->text().size() == 0 || ui.password->text().size() == 0))
	{
		QMessageBox::information(this, qApp->applicationName(), tr("Username and password must be entered."), QMessageBox::Cancel);
		return;
	}

	m_share->setFolder(ui.folder->text());
	m_share->setPort(ui.port->value());

	m_share->setType(ui.type->currentIndex() == 1 ? Share::TypeFTP : Share::TypeHTTP);
	
	if(ui.downloadOnly->isChecked())
		m_share->setPermission(Share::PermissionRO);
	if(ui.dropbox->isChecked())
		m_share->setPermission(Share::PermissionDropbox);
	if(ui.fullAccess->isChecked())
		m_share->setPermission(Share::PermissionRW);

	if(ui.authentication->isChecked()) {
		m_share->setAccess(Share::AccessUsernamePassword);
		m_share->setUsernamePassword(ui.username->text(), ui.password->text());
	} else {
		m_share->setUsernamePassword("", "");
		m_share->setAccess(Share::AccessAnonymous);
	}

	QDialog::accept();
}

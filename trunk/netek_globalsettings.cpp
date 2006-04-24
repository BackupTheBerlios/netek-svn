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

#include "netek_globalsettings.h"
#include "netek_settings.h"
#include "netek_adetectsettings.h"

neteK::GlobalSettings::GlobalSettings()
{
	ui.setupUi(this);
	
	Settings settings;
	ui.tcpFrom->setValue(settings.randomTcpPortMin());
	ui.tcpTo->setValue(settings.randomTcpPortMax());
	ui.ftpUseUnicode->setChecked(settings.ftpUseUnicodeByDefault());
	ui.ftpAllowPassive->setChecked(settings.ftpAllowPassive());
	ui.paAutodetect->setChecked(settings.publicAddress() == Settings::PublicAddressAutodetect);
	ui.paManual->setChecked(settings.publicAddress() == Settings::PublicAddressManual);
	ui.publicAddress->setText(settings.customPublicAddress());
	
	connect(ui.paAutodetect, SIGNAL(clicked()), SLOT(publicAddressRadio()));
	connect(ui.paManual, SIGNAL(clicked()), SLOT(publicAddressRadio()));
	connect(ui.autodetectSettings, SIGNAL(clicked()), SLOT(showAutodetectSettings()));
	
	publicAddressRadio();
}

void neteK::GlobalSettings::showAutodetectSettings()
{
	AutodetectSettings().exec();
}

void neteK::GlobalSettings::publicAddressRadio()
{
	ui.publicAddress->setEnabled(ui.paManual->isChecked());
}

void neteK::GlobalSettings::accept()
{
	if(ui.tcpFrom->value() > ui.tcpTo->value()) {
		QMessageBox::information(this, qApp->applicationName(), tr("Invalid TCP port range."), QMessageBox::Cancel);
		return;
	}
	
	Settings settings;
	settings.setRandomTcpPortMin(ui.tcpFrom->value());
	settings.setRandomTcpPortMax(ui.tcpTo->value());
	settings.setFtpUseUnicodeByDefault(ui.ftpUseUnicode->isChecked());
	settings.setFtpAllowPassive(ui.ftpAllowPassive->isChecked());
	settings.setPublicAddress(
		ui.paManual->isChecked()
			? Settings::PublicAddressManual
			: Settings::PublicAddressAutodetect);
	settings.setCustomPublicAddress(ui.publicAddress->text());
	
	QDialog::accept();
}

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

#include "netek_adetectsettings.h"
#include "netek_settings.h"

neteK::AutodetectSettings::AutodetectSettings()
{
	ui.setupUi(this);

	connect(ui.customSiteCheck, SIGNAL(clicked()), SLOT(changeState()));
	connect(ui.cacheTimeCheck, SIGNAL(clicked()), SLOT(changeState()));
	
	Settings settings;
	
	ui.customSite->setText(settings.autodetectAddress());
	ui.customSiteCheck->setChecked(settings.autodetectAddressCustom());
	ui.cacheTime->setValue(settings.autodetectRefresh());
	ui.cacheTimeCheck->setChecked(settings.autodetectRefreshCustom());

	changeState();
}

void neteK::AutodetectSettings::accept()
{
	Settings settings;
	
	settings.setAutodetectAddress(ui.customSite->text());
	settings.setAutodetectAddressCustom(ui.customSiteCheck->isChecked());
	settings.setAutodetectRefresh(ui.cacheTime->value());
	settings.setAutodetectRefreshCustom(ui.cacheTimeCheck->isChecked());
	
	QDialog::accept();
}

void neteK::AutodetectSettings::changeState()
{
	ui.customSite->setEnabled(ui.customSiteCheck->isChecked());
	if(!ui.customSiteCheck->isChecked())
		ui.customSite->setText(Settings::autodetectAddressDefault);
		
	ui.cacheTime->setEnabled(ui.cacheTimeCheck->isChecked());
	ui.cacheTimeLabel->setEnabled(ui.cacheTimeCheck->isChecked());
	if(!ui.cacheTimeCheck->isChecked())
		ui.cacheTime->setValue(Settings::autodetectRefreshDefault);
	
}

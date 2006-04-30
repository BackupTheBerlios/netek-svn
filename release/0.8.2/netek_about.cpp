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

#include "netek_about.h"
#include "netek_application.h"

neteK::About::About()
{
	ui.setupUi(this);
	
	ui.programName->setText(QString("%1 %2").arg(qApp->applicationName()).arg(Application::applicationVersion()));
	ui.programSub->setText(tr("Using Qt %1").arg(qVersion()));
	
	ui.textBox->setHtml(
		"<html><body>"
			
		"<p>neteK project http://netek.berlios.de"
		"<br/>Copyright (C) 2005-2006 Egon Kocjan &lt;egon.kocjan@xlab.si&gt;</p>"
			
		"<p>This program is free software; you can redistribute it and/or modify"
		" it under the terms of the GNU General Public License as published by"
		" the Free Software Foundation; either version 2 of the License, or"
		" (at your option) any later version.</p>"
			
		"<p>This program is distributed in the hope that it will be useful,"
		" but WITHOUT ANY WARRANTY; without even the implied warranty of"
		" MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the"
		" GNU General Public License for more details.</p>"
					
		"<p>You should have received a copy of the GNU General Public License"
		" along with this program; if not, write to the Free Software"
		" Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA</p>"
			
		"</body></html>"
	);
}

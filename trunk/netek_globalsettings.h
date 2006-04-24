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

#ifndef __NETEK_GLOBALSETTINGS_H__
#define __NETEK_GLOBALSETTINGS_H__

#include "ui_netek_globalsettings.h"

#include <QtGui>

namespace neteK {

class GlobalSettings: public QDialog {
		Q_OBJECT;

	public slots:
		virtual void accept();
		void publicAddressRadio();
		void showAutodetectSettings();

	public:
		GlobalSettings();

	private:
		Ui::GlobalSettings ui;
};

}

#endif

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

// TODO: show speed in gui
// TODO: browsable copy link
// TODO: use kde-config for konq services menu
// TODO: remove QPointers on stack
// TODO: ip based filter
// TODO: use inno setup
// TODO: remove all exec()s
// TODO: use builtin tray icon
// TODO: get rid of netek.rc win32 file

#include "netek_application.h"
#include "netek_gui.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

#include <ctime>

int main(int argc, char *argv[])
{
	srand(
#ifdef Q_OS_UNIX
		getpid()
#endif

#ifdef Q_OS_WIN
		GetCurrentProcessId()
#endif

		+ clock()
	);

	neteK::Application app(argc, argv);
	neteK::Gui gui;

	//gui.show();

	return app.exec();
}

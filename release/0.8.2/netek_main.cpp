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
#ifdef Q_WS_X11
	if(argc >= 2 && !strcmp(argv[1], "waitabit"))
		sleep(5); // Qt & GNOME colorscheme bug...
#endif

	srand(
		time(0) +
#if defined(Q_OS_UNIX)
		getpid()
#elif defined(Q_OS_WIN)
		GetCurrentProcessId() + GetCurrentThreadId()
#endif
	);

	neteK::Application app(argc, argv);
	
	{
		QStringList broken;
		// see Qt bug 105055
		broken.append("4.1.1");
		broken.append("4.1.2");
		
		if(broken.contains(qVersion()))
			QMessageBox::warning(0, qApp->applicationName(),
				QObject::tr("Your version of Qt has a bug, which is known to break this application.\nBad Qt versions: %1.")
					.arg(broken.join(", ")),
				QMessageBox::Ok, QMessageBox::NoButton);
				    
	}

	neteK::Gui gui;

	//gui.show();

	return app.exec();
}

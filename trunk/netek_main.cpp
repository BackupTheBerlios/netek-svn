#include "netek_application.h"
#include "netek_gui.h"

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
	srand(
		time(0)
#if defined(Q_OS_UNIX)
		+ getpid()
#elif defined(Q_OS_WIN32)
		+ GetCurrentProcessId() + GetCurrentThreadId()
#endif
	);

	neteK::Application app(argc, argv);
	
	neteK::Gui gui;

	//gui.show();

	return app.exec();
}

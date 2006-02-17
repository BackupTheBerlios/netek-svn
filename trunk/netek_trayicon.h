#ifndef __NETEK_TRAYICON_H__
#define __NETEK_TRAYICON_H__

#include <QtGui>

namespace neteK {

//signals:
//	void activated();
//	void showMenu(const QPoint &pos);

QObject *makeTrayIcon(QMainWindow *owner);

}

#endif

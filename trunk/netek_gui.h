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

#ifndef __NETEK_GUI_H__
#define __NETEK_GUI_H__

#include "ui_netek_gui.h"

#include <QtGui>

namespace neteK {

class Share;
class Shares;
class LogViewer;
class TrayIcon;

class Gui: public QMainWindow {
	Q_OBJECT;

	Ui::Gui ui;

	QPointer<TrayIcon> m_tray_icon;
	bool m_save_geometry_timer;
	QPointer<Shares> m_shares;
	QPointer<LogViewer> m_log_viewer;

	Share *getShare(int id = -1);
	static QPixmap shareIcon(Share *sh);
	static bool validAndConfigured(Share *sh);
	void makeMappedShareAction(Share *sh, QMenu *m, QSignalMapper *sm, int i, QAction *a);
	static bool getDragAndDropPath(const QMimeData *mime, QString &path);

public:
	Gui();

	void closeEvent(QCloseEvent *);
	void resizeEvent(QResizeEvent *);
	void moveEvent(QMoveEvent *);
	void dragEnterEvent(QDragEnterEvent *e);
	void dropEvent(QDropEvent *e);

signals:
	void userQuit();

public slots:
	void toggleRunStatus();
	void shareSettings(int id = -1);
	void deleteShare(int id = -1);
	void startShare(int id = -1);
	void stopShare(int id = -1);
	void stopAllShares();
	void sharesChanged();
	void globalSettings();
	void toggleVisible();
	void trayMenu(const QPoint &pos);
	void saveGeometry();
	void saveGeometryTimer();
	void shareMenu();
	void copyLinkMenu(int id = -1);
	void showLog();
	void showAbout();
	void quitRequest();
};

}

#endif

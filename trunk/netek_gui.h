#ifndef __NETEK_GUI_H__
#define __NETEK_GUI_H__

#include "ui_netek_gui.h"

#include <QtGui>

namespace neteK {

class Share;
class Shares;
class LogViewer;

class Gui: public QMainWindow {
	Q_OBJECT;
	
	Ui::Gui ui;
	
	QPointer<QObject> m_icon;
	bool m_save_geometry_timer;
	QPointer<Shares> m_shares;
	QPointer<LogViewer> m_log_viewer;
	
	Share *currentShare();
	
public:
	Gui();
	
	void closeEvent(QCloseEvent *);
	void resizeEvent(QResizeEvent *);
	void moveEvent(QMoveEvent *);
	
signals:
	void quit();
	
public slots:
	void toggleRunStatus();
	void shareSettings();
	void deleteShare();
	void startShare();
	void stopShare();
	void sharesChanged();
	void globalSettings();
	void toggleVisible();
	void trayMenu(const QPoint &pos);
	void saveGeometry();
	void saveGeometryTimer();
	void shareMenu();
	void copyLinkMenu();
	void showLog();
};

}

#endif

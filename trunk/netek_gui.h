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
	
	Share *getShare(int id = -1);
	static QPixmap shareIcon(Share *sh);
	static bool validAndConfigured(Share *sh);
	void makeMappedShareAction(Share *sh, QMenu *m, QSignalMapper *sm, int i, QAction *a);
	static bool getDragAndDropPath(QString text, QString &path);
	
public:
	Gui();
	
	void closeEvent(QCloseEvent *);
	void resizeEvent(QResizeEvent *);
	void moveEvent(QMoveEvent *);
	void dragEnterEvent(QDragEnterEvent *e);
	void dropEvent(QDropEvent *e);
	
signals:
	void quit();
	
public slots:
	void toggleRunStatus();
	void shareSettings(int id = -1);
	void deleteShare(int id = -1);
	void startShare(int id = -1);
	void stopShare(int id = -1);
	void sharesChanged();
	void globalSettings();
	void toggleVisible();
	void trayMenu(const QPoint &pos);
	void saveGeometry();
	void saveGeometryTimer();
	void shareMenu();
	void copyLinkMenu(int id = -1);
	void showLog();
};

}

#endif

#ifndef __NETEK_GUI_H__
#define __NETEK_GUI_H__

#include "ui_netek_gui.h"

#include <QtGui>

namespace neteK {

class Share;
class Shares;

class Gui: public QMainWindow {
	Q_OBJECT;
	
	QPointer<QObject> m_icon;
	bool m_save_geometry_timer;
	
public:
	Gui();
	
	void closeEvent(QCloseEvent *);
	void resizeEvent(QResizeEvent *);
	void moveEvent(QMoveEvent *);

private:
	Ui::Gui ui;
	
	QPointer<Shares> m_shares;
	Share *currentShare();
	void saveGeometryTimer();
	
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
};

}

#endif

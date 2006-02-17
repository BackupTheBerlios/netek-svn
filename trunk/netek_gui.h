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
	
public:
	Gui();
	
	void closeEvent(QCloseEvent *);

private:
	Ui::Gui ui;
	
	QPointer<Shares> m_shares;
	Share *currentShare();
	
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
};

}

#endif

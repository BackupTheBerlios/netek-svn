#ifndef __NETEK_LOGVIEWER_H__
#define __NETEK_LOGVIEWER_H__

#include "ui_netek_logviewer.h"

namespace neteK {

class LogViewer: public QDialog {
	Q_OBJECT;
	
	Ui::LogViewer ui;
	
public:
	LogViewer();
	
signals:
	void scrollDownSignal();
	
public slots:
	void reject();
	
	void scrollDown();
	void copyToClipboard();
	void saveToFile();
	void log(QString data);
};

}

#endif

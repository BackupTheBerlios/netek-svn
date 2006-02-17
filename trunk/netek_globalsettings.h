#ifndef __NETEK_GLOBALSETTINGS_H__
#define __NETEK_GLOBALSETTINGS_H__

#include "ui_netek_globalsettings.h"

#include <QtGui>

namespace neteK {

class GlobalSettings: public QDialog {
		Q_OBJECT;

	public slots:
		virtual void accept();

	public:
		GlobalSettings();

	private:
		Ui::GlobalSettings ui;
};

}

#endif

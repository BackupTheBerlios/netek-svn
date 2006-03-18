#ifndef __NETEK_APPLICATION_H__
#define __NETEK_APPLICATION_H__

#include <QtGui>

namespace neteK {

class Application: public QApplication {
	Q_OBJECT;

public:
	Application(int &argc, char **argv);

public slots:
	void userQuit();
};

}

#endif

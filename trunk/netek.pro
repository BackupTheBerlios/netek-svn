TEMPLATE = app
TARGET = netek
SOURCES = netek_*.cpp
HEADERS = netek_*.h
FORMS = netek_*.ui
RESOURCES = netek_*.qrc
QT = core gui network xml

win32 {
	system("windres netek.rc rc.o")
	OBJECTS += rc.o
	LIBS += -luser32 -lshell32 -ladvapi32 -lws2_32
	
	debug {
		CONFIG += console
	}
}

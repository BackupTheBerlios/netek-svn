TEMPLATE = app
TARGET = netek
SOURCES = netek_*.cpp
HEADERS = netek_*.h
FORMS = netek_globalsettings.ui netek_sharesettings.ui netek_gui.ui
RESOURCES = icons.qrc
QT = core gui network

win32 {
	system("windres netek.rc rc.o")
	OBJECTS += rc.o
	LIBS += -lws2_32
}

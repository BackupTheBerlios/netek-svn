TEMPLATE = app
VERSION = 0.8.0
TARGET = netek
SOURCES = netek_*.cpp
HEADERS = netek_*.h
FORMS = netek_globalsettings.ui netek_sharesettings.ui netek_gui.ui
RESOURCES = icons.qrc
QT = core gui network
DISTFILES = icons/*.png

target.path = /usr/bin
INSTALLS = target

OVRWINDOW = ../src

include($$OVRWINDOW/ovrwindow.pri)

# Build configuration.
TEMPLATE = app
TARGET = sample
DESTDIR = build
UI_DIR = $$DESTDIR/ui
MOC_DIR = $$DESTDIR/moc
OBJECTS_DIR = $$DESTDIR/obj
QMAKE_CXXFLAGS += -Wall -Wextra

# OVRWindow configuration.
HEADERS += $$OVRWINDOW/OVRWindow.h
SOURCES += $$OVRWINDOW/OVRWindow.cpp main.cpp

INCLUDEPATH += $$OVRWINDOW

# Path to the OVRWindow source code tree.
OVRWINDOW = ../src

# OVRWindow configuration.
include($$OVRWINDOW/ovrwindow.pri)

# OVRWindow source.
INCLUDEPATH += $$OVRWINDOW
HEADERS += $$OVRWINDOW/OVRWindow.h
SOURCES += $$OVRWINDOW/OVRWindow.cpp

# The sample project's build configuration.
TEMPLATE = app
TARGET = sample
DESTDIR = build
UI_DIR = $$DESTDIR/ui
MOC_DIR = $$DESTDIR/moc
OBJECTS_DIR = $$DESTDIR/obj
QMAKE_CXXFLAGS += -Wall -Wextra
SOURCES += main.cpp

lessThan(QT_MAJOR_VERSION, 5):error(The OVRWindow API requires Qt5 or later. Make sure your version of qmake is using Qt5 libraries at least.)

# Make sure the OVRSDK environment variable is defined...
OVRSDK = $$(OVRSDK)
isEmpty(OVRSDK):error(The OVRSDK environment variable is not defined.)
!exists($$OVRSDK):error("The OVRSDK environment variable is defined, but \'$$OVRSDK\' does not exist.")

# ...and contains a path to a valid Oculus SDK installation.
LIBOVR = $$OVRSDK/LibOVR
!exists($$LIBOVR/Include/OVR.h):error("The OVRSDK environment variable is defined, but \'$$OVRSDK\' does not contain a valid Oculus SDK installation.")

# Use gui-private to gain access to the platform's native interface.
QT += gui-private

# Add C++11 support.
CONFIG += c++11

# Common build configuration.
INCLUDEPATH += $$LIBOVR/Include $$LIBOVR/Src
LIBS += -lovr

# GNU/Linux build configuration.
unix:!macx {
   eval(QMAKE_HOST.arch = x86_64): LIBS += -L$$LIBOVR/Lib/Linux/Release/x86_64
   else:                           LIBS += -L$$LIBOVR/Lib/Linux/Release/i386
                                   LIBS += -lX11 -lXinerama -lXrandr -ludev
}

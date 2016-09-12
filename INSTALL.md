## Installation Guide

This document describes how to install the dependencies required by OVRWindow.

OVRWindow requires the [Qt GUI module][qtgui] introduced in [Qt 5][qt5]. It also uses modern C++ features and therefore requires a compiler that implements the C++14 standard, at least.



### Linux (Debian)

#### Qt5 private header files

While Qt5 provides a modern way of using the OpenGL API, it is still does not provide (as of September 2016) an easy way to access to the native display device (required by the Oculus SDK). The current approach is to use a part of the [Qt Platform Abstraction][qpa] (QPA), an abstraction layer still in development, that does not guarantee source or binary compatibility. This is not a portable solution but will very likely become one as the QPA matures.
To use the QPA, you will need to install Qt's private header development files
```bash
sudo apt-get install qtbase5-private-dev
```


#### Oculus SDK

OVRWindow depends on the C API introduced in Oculus SDK 0.3 and later. It is currently developed against [Oculus SDK v0.3.2-preview][ovrsdk] on Linux.

OVRWindow will need to know where to find your Oculus SDK installation. It will look for the __OVRSDK__<sup>1</sup> environment variable which should contain the location of your SDK installation.

If you are building your project from the command line, the variable can be exported by running
```bash
export OVRSDK=<path/to/oculus/sdk>
```
where `<path/to/oculus/sdk>` is the location of your SDK installation.

On the other hand, if you are building a project from Qt Creator, the variable can be added to the current build configuration's *Build Environment* found under [Projects] > [Build & Run]. Remember that each build configuration has its own build environment so if you do change the build configuration, you may have to update its build environment.

<br>
<br>

1.
If the name __OVRSDK__ conflicts with your current environment, line #4 of __ovrwindow.pri__ can be changed from `OVRSDK = $$(OVRSDK)` to `OVRSDK = $$(<CUSTOM_PATH_TO_OVR_SDK>)` where `<CUSTOM_PATH_TO_OVR_SDK>`
is either an environment variable that contains the absolute path to a valid installation of the Oculus SDK, or the path itself.



[qt5]: https://doc.qt.io/qt-5/qt5-intro.html
[qtgui]: https://doc.qt.io/qt-5/qtgui-index.html
[qpa]: https://wiki.qt.io/Qt_Platform_Abstraction
[ovrsdk]: https://developer3.oculus.com/downloads/pc/0.3.2-preview-2/Oculus_SDK_for_Linux/

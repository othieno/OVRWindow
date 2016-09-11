OVRWindow
=========

OVRWindow is an Oculus SDK wrapper for Qt applications that uses the OpenGL API. The full
documentation can be found [__here__](https://othieno.github.io/OVRWindow).


Requirements
------------

The OVRWindow API depends on the C API introduced in Oculus SDK 0.4 and later.

OVRWindow inherits from the __QWindow__ and __QOpenGLFunctions__ classes that belong to the revised
[Qt GUI][qtgui] module introduced in Qt5, meaning that prior versions of the Qt library are not
supported by this API. While Qt5 provides a modern way of using the OpenGL API, it is still
relatively new (as of August, 2014) and a few features that exist in prior versions of Qt are
still not fully implemented. One such feature is an API that provides easy access to the native
display device (required by the Oculus SDK). The current solution is to use a part of the [Qt
Platform Abstraction][qpa] (QPA) that is strongly-coupled to the version of Qt it was built against.
Consequently, this is not a portable solution but will very likely become one as the QPA API matures.

The final requirement is a C++ compiler that implements the C++14 standard, at least.


Installation
------------

To be able to use the provided project include file, the environment variable __OVRSDK__ must be
defined as the path to a valid Oculus SDK installation.

When building a project from QtCreator, this variable and its corresponding path must be added to
the current build configuration's 'Build Environment' found in project mode (CTRL+4). Make sure to
remember that each build configuration has its own build environment!

When building a project from the command line, then __OVRSDK__ needs to be exported as an
environment variable.

If the name OVRSDK conflicts with your system, line #4 of ovrwindow.pri can be changed from
`OVRSDK = $$(OVRSDK)` to `OVRSDK = $$(<CUSTOM_PATH_TO_OVR_SDK>)` where `<CUSTOM_PATH_TO_OVR_SDK>`
is either an environment variable that contains the path to a valid installation of the Oculus SDK,
or an absolute path to the aforementioned SDK.


Using OVRWindow
---------------

Before using the OVRWindow API, make sure all previous requirements are met.

The project include file __src/ovrwindow.pri__ included with the API's source code provides a
configuration that will help build the API. The first step is to simply include this file in
your project file (*.pro). Next, add the locations of __OVRWindow.h__ and __OVRWindow.cpp__ to the
__HEADERS__ and __SOURCES__ variables in the same project file, respectively.

Check out the sample for a working project file example.


Hierarchy
---------

The folders provided with this software are structured in the following manner:
* __doc__ contains the OVRWindow API's documentation.
* __sample__ contains a simple example on how to use OVRWindow.
* __src__ contains the source code tree.
* __tst__ contains unit tests.


[qtgui]: http://qt-project.org/doc/qt-5/qtgui-index.html
[qpa]: http://qt-project.org/wiki/Qt-Platform-Abstraction

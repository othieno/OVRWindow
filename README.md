# OVRWindow

OVRWindow is an Oculus SDK wrapper for Qt applications that uses the OpenGL API. The API's full documentation can be found [__here__](https://othieno.github.io/OVRWindow).


## Using OVRWindow

Before using OVRWindow, make sure you have installed its dependencies and correctly set up your build environment. For help on doing so, please refer to the [installation guide](INSTALL.md).

Included in the source code tree is __ovrwindow.pri__, a project include file that makes it easy to integrate OVRWindow and its dependencies into your own projects. Simply include it in your project file (*.pro).

Next, add the locations of __OVRWindow.h__ and __OVRWindow.cpp__ to the
__HEADERS__ and __SOURCES__ variables in your project file, respectively.

Check out the sample's project's [configuration](sample/sample.pro) for a working project file example.

## Project Hierarchy

The folders provided with this software are structured in the following manner:
* __sample__ contains a simple example on how to use OVRWindow.
* __src__ contains the source code tree.
* __tst__ contains unit tests.

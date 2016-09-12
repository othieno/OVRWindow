/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2016 Jeremy Othieno.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <OVRWindow.h>
#include <QGuiApplication>
#include <cmath>


class SpinningCubeWindow : public OVRWindow {
public:
    void initializeGL() override final;
    void paintGL(const ovrEyeType, const OVRWindow::RenderTransforms&, const float) override final;
private:
    GLfloat _angle;
};


int main(int argc, char **argv) {
    QGuiApplication application(argc, argv);

    SpinningCubeWindow window;
    window.setTitle("OVRWindow : Spinning Cube");
    window.showFullScreen();

    return application.exec();
}


void
SpinningCubeWindow::initializeGL() {
    glClearColor(0.25f, 0.5f, 0.75f, 1.0f);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_BLEND);

    glDisable(GL_TEXTURE_2D);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);

    // Add some stationary lights.
    const GLfloat position[2][4] = {{5.0f, 6.0f, 3.0f, 0.0f}, {-5.0f,-6.0f, 5.0f, 0.0f}};
    const GLfloat diffuse[2][4]  = {{1.0f, 0.8f, 0.6f, 1.0f}, {0.6f, 0.8f, 1.0f, 1.0f}};
    const GLenum light[2] = {GL_LIGHT0, GL_LIGHT1};
    for (unsigned int i = 0; i < 2; ++i) {
        glLightfv(light[i], GL_POSITION, position[i]);
        glLightfv(light[i], GL_DIFFUSE, diffuse[i]);
        glEnable(light[i]);
    }

    // Define the object's material.
    const GLfloat specular[] = {0.3f, 0.3f, 0.3f, 1.0f};
    const GLfloat shininess = 10.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
}


void
SpinningCubeWindow::paintGL(const ovrEyeType, const OVRWindow::RenderTransforms& transforms, const float dt) {
    _angle = static_cast<GLfloat>(std::fmod(_angle + (dt * 5.0), 360.0));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMultMatrixf(transforms.perspective.constData());

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMultMatrixf(transforms.view.constData());

    // Move the triangle away from the screen and rotate it.
    glTranslatef(0.0f, 0.0f, -1.5f);
    glRotatef(_angle, 1.0f, 0.0f, 0.0f);
    glRotatef(_angle, 0.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);
    glNormal3f( 0.0f, 0.0f, 1.0f);
    glVertex3f( 0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f,-0.5f, 0.5f);
    glVertex3f( 0.5f,-0.5f, 0.5f);

    glNormal3f( 0.0f, 0.0f,-1.0f);
    glVertex3f(-0.5f,-0.5f,-0.5f);
    glVertex3f(-0.5f, 0.5f,-0.5f);
    glVertex3f( 0.5f, 0.5f,-0.5f);
    glVertex3f( 0.5f,-0.5f,-0.5f);

    glNormal3f( 0.0f, 1.0f, 0.0f);
    glVertex3f( 0.5f, 0.5f, 0.5f);
    glVertex3f( 0.5f, 0.5f,-0.5f);
    glVertex3f(-0.5f, 0.5f,-0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);

    glNormal3f( 0.0f,-1.0f, 0.0f);
    glVertex3f(-0.5f,-0.5f,-0.5f);
    glVertex3f( 0.5f,-0.5f,-0.5f);
    glVertex3f( 0.5f,-0.5f, 0.5f);
    glVertex3f(-0.5f,-0.5f, 0.5f);

    glNormal3f( 1.0f, 0.0f, 0.0f);
    glVertex3f( 0.5f, 0.5f, 0.5f);
    glVertex3f( 0.5f,-0.5f, 0.5f);
    glVertex3f( 0.5f,-0.5f,-0.5f);
    glVertex3f( 0.5f, 0.5f,-0.5f);

    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-0.5f,-0.5f,-0.5f);
    glVertex3f(-0.5f,-0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f,-0.5f);
    glEnd();
}

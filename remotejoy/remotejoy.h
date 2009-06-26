/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2009, Michael Forney <michael@obberon.com>
 */

#ifndef REMOTEJOY_H
#define REMOTEJOY_H

#include <QGLWidget>

#include <QTime>

class ImageReader;
class AudioCapture;

class QPaintEvent;
class QKeyEvent;

class RemoteJoy : public QGLWidget
{
    Q_OBJECT
    public:
        RemoteJoy();
        virtual ~RemoteJoy();
        GLuint bindTexture(const QSize & size, char * data, GLenum type);
    protected:
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);
        void keyPressEvent(QKeyEvent * event);

    private slots:
        void imageReceived(const QSize & size, char * data, GLenum type);
        //void imageReceived(const QImage & image);
    private:
        ImageReader * reader;
        AudioCapture * audioCapture;
        GLuint texture;
        bool capturing;
        QTime captureStart;
        int imageNumber;
};

#endif // REMOTEJOY_H
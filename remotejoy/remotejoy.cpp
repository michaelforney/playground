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

#include "remotejoy.h"

#include "imagereader.h"
#include "audiocapture.h"

#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QDir>

#define PSP_H 272
#define PSP_W 480

RemoteJoy::RemoteJoy()
 : QGLWidget()
{
    resize(PSP_W, PSP_H);
    reader = new ImageReader();
    audioCapture = new AudioCapture();
    connect(reader, SIGNAL(imageReceived(const QSize &, char *, GLenum)), this, SLOT(imageReceived(const QSize &, char *, GLenum)));
    //connect(reader, SIGNAL(imageReceived(const QImage &)), this, SLOT(imageReceived(const QImage &)));
    reader->start();
    texture = 0;
    capturing = false;
    QTimer * timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer->start(40);
}

RemoteJoy::~RemoteJoy()
{
}

void RemoteJoy::imageReceived(const QSize & size, char * data, GLenum type)
//void RemoteJoy::imageReceived(const QImage & image)
{
    qDebug() << "image received";
    //texture = bindTexture(image);
    if (texture)
    {
        glDeleteTextures(1, &texture);
    }
    texture = bindTexture(size, data, type);
    
    //texture = bindTexture(image);
    //updateGL();
    
    qDebug() << "image received2";
}

GLuint RemoteJoy::bindTexture(const QSize & size, char * data, GLenum type)
{
    qDebug() << "bind texture";
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    //qDebug() << "bind texture 2" << data.size() << "test";
    glTexImage2D(GL_TEXTURE_2D, 0, 3, size.width(), size.height(), 0, GL_RGBA, type, data);
    qDebug() << "bind texture 3";
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
    free(data);
    qDebug() << "free";
    qDebug() << "bind texture end";
    return tex;
}

void RemoteJoy::initializeGL()
{
    qDebug() << "initializeGL";
    glEnable(GL_TEXTURE_2D);
    qglClearColor(Qt::black);
}

void RemoteJoy::paintGL()
{
    qDebug() << "paintgl";
    glLoadIdentity();
    if (texture)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0, 1.0);      // Bottom Left Of The Texture and Quad
            glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0, 1.0);      // Bottom Right Of The Texture and Quad
            glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0, -1.0);      // Top Right Of The Texture and Quad
            glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0, -1.0);
        glEnd();
        if (capturing)
        {
            qDebug() << "Saving image";
            qDebug() << grabFrameBuffer().save(QString("capture/%1/video/%2.png").arg(captureStart.toString()).arg(QString::number(imageNumber).rightJustified(10, '0')));
            imageNumber++;
        }
    }
    else
    {
        qglClearColor(Qt::black);
    }
}

void RemoteJoy::resizeGL(int width, int height)
{
    qDebug() << "resizeGL";
    glViewport(0, 0, width, height);
}

void RemoteJoy::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_C)
    {
        if (capturing)
        {
            qDebug() << "stopping capture";
            audioCapture->stopCapture();
            capturing = false;
        }
        else
        {
            qDebug() << "starting capture";
            captureStart = QTime::currentTime();
            imageNumber = 0;
            QDir::current().mkpath(QString("capture/%1/video").arg(captureStart.toString()));
            QDir::current().mkpath(QString("capture/%1/audio").arg(captureStart.toString()));
            audioCapture->startCapture(QString("capture/%1/audio/audio.pcm").arg(captureStart.toString()));
            capturing = true;
        }
    }
}

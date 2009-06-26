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

#ifndef IMAGEREADER_H
#define IMAGEREADER_H

#include <QThread>

#include <QImage>
#include <GL/gl.h>

class QTcpSocket;
class QProcess;

class ImageReader : public QThread
{
    Q_OBJECT
    public:
        ImageReader();
        virtual ~ImageReader();
    protected:
        void run();
        void enableScreen();
    protected slots:
        void connected();
        void readyRead();
    signals:
        void imageReceived(const QSize & size, char * data, GLenum type);
        //void imageReceived(const QImage & image);
    private:
        QTcpSocket * socket;
        int location;
        bool readHeader;
        GLenum imageType;
        //QImage::Format format;
        int imageSize;
        char * buffer[2];
        int bufferNumber;
        bool capture;
};

#endif // IMAGEREADER_H
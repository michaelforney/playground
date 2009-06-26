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

#include "imagereader.h"

#include <QTcpSocket>
#include <QDebug>
#include <QProcess>

#define PSP_H 272
#define PSP_W 480

const QString host("localhost");
const quint16 port = 10004;
//const quint32 remoteJoyMagic = 0x909ACCEF;
#define remoteJoyMagic 0x909ACCEF

ImageReader::ImageReader()
 : QThread()
{
}

ImageReader::~ImageReader()
{
}

void ImageReader::run()
{
    socket = new QTcpSocket();
    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    socket->connectToHost(host, port);
    exec();
}

void ImageReader::enableScreen()
{
    qDebug() << "enabling screen";
    bufferNumber = 0;
    QDataStream stream(socket);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << remoteJoyMagic;
    stream << (qint32)5;
    stream << (quint32)5;
}

void ImageReader::connected()
{
    qDebug() << "connected";
    readHeader = false;
    imageSize = 0;
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void ImageReader::readyRead()
{
    //qDebug() << "readyRead!";
    if (readHeader)
    {
        //qDebug() << "reading data";
        qint64 bytesRead = socket->read(buffer[bufferNumber] + location, imageSize - location);
        //qDebug() << "read  " << bytesRead << " bytes";
        if (bytesRead < 0)
        {
            qDebug() << "^^^^^^^^^^^^^^^^^^^^^ERROR READING^^^^^^^^^^^^^^^^^^^^^^^^^^^^";
        }
        location += bytesRead;
        Q_ASSERT(location <= imageSize);
        if (location == imageSize)
        {
            qDebug() << "read image";
            emit imageReceived(QSize(PSP_W, PSP_H), buffer[bufferNumber], imageType);
            location = 0;
            imageSize = 0;
            bufferNumber = bufferNumber == 0 ? 1 : 0;
            //buffer[bufferNumber].clear();
            readHeader = false;
        }
        else
        {

        }
    }
    else
    {
        //qDebug() << "reading header";
        if (socket->bytesAvailable() >= 16)
        {
            QDataStream stream(socket);
            stream.setByteOrder(QDataStream::LittleEndian);
            quint32 magic;
            qint32 mode;
            qint32 size;
            qint32 ref;
            stream >> magic;
            stream >> mode;
            stream >> size;
            stream >> ref;
            //qDebug() << QString::number(magic, 16) << ", " << mode << ", " << size << ", " << ref;
            Q_ASSERT(magic == remoteJoyMagic);
            if (mode == -1)
            {
                readHeader = false;
                enableScreen();
            }
            else if (mode < 3 && mode >= 0)
            {
                readHeader = true;
                Q_ASSERT(mode <= 3);
                switch(mode)
                {
                    case 3:
                        imageType = GL_UNSIGNED_INT_8_8_8_8_REV;
                        break;
                    case 2:
                        imageType = GL_UNSIGNED_SHORT_4_4_4_4_REV;
                        break;
                    case 1:
                        imageType = GL_UNSIGNED_SHORT_1_5_5_5_REV;
                        break;
                    case 0:
                        imageType = GL_UNSIGNED_SHORT_5_6_5_REV;
                        break;
                }
                imageSize = size;
                qDebug() << imageType;
                location = 0;
                buffer[bufferNumber] = (char *)malloc(size);
                qDebug() << "malloc" << bufferNumber;
            }
            else
            {
                readHeader = false;
                socket->flush();
            }
        }
    }
}

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

#include "audiocapture.h"

#include <QDebug>
#include <QFile>
#include <alsa/asoundlib.h>

#define BUFFER_SIZE 128

AudioCapture::AudioCapture(QObject * parent)
 : QThread(parent)
{

}

AudioCapture::~AudioCapture()
{
}

void AudioCapture::stopCapture()
{
    capturing = false;
}

void AudioCapture::run()
{
    qDebug() << "AudioCapture::run()";
    snd_pcm_t * captureHandle;
    snd_pcm_hw_params_t * params;
    snd_pcm_hw_params_alloca(&params);
    if (snd_pcm_open(&captureHandle, "default", SND_PCM_STREAM_CAPTURE, 0) < 0)
    {
        qDebug() << "error opening audio device";
        return;
    }
    if (snd_pcm_hw_params_any(captureHandle, params) < 0)
    {
        qDebug() << "cannot initialize hardware paramerter structure";
        return;
    }
    if (snd_pcm_hw_params_set_access(captureHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
    {
        qDebug() << "cannot set access type";
        return;
    }
    if (snd_pcm_hw_params_set_format(captureHandle, params, SND_PCM_FORMAT_S16_LE) < 0)
    {
        qDebug() << "cannot set sample format";
        return;
    }
    unsigned int rate = 48000;
    if (snd_pcm_hw_params_set_rate_near(captureHandle, params, &rate, 0) < 0)
    {
        qDebug() << "cannot set sample rate";
        return;
    }
    qDebug() << rate;
    if (snd_pcm_hw_params_set_channels(captureHandle, params, 2) < 0)
    {
        qDebug() << "cannot set channels to 2";
        return;
    }
    /*int periods = 2;
    snd_pcm_uframes_t periodsize = 8192;
    qDebug() << "settings periods";
    if (snd_pcm_hw_params_set_periods(captureHandle, params, periods, 0) < 0)
    {
        qDebug() << "Error setting periods.";
        return;
    }
    qDebug() << "setting buffer size to " << ((periodsize * periods)>>2);
    snd_pcm_hw_params_set_buffer_size(captureHandle, params, (periodsize * periods)>>2);
    qDebug() << "settings params";*/
    if (snd_pcm_hw_params(captureHandle, params) < 0)
    {
        qDebug() << "cannot set parameters";
        return;
    }
    //snd_pcm_hw_params_free(params);
    //qDebug() << 
    if (snd_pcm_prepare(captureHandle) < 0)
    {
        qDebug() << "cannot prepare audio interface";
    }
    QFile file(captureFile);
    //QFile file("test.pcm");
    qDebug() << file.open(QIODevice::WriteOnly);
    qDebug() << "++starting loop";
    int frames = 1024;
    short buffer[frames * 2];
    while (capturing)
    {
        qDebug() << "about to read samples";
        int samplesRead = snd_pcm_readi(captureHandle, buffer, frames);
        qDebug() << "read " << samplesRead << "samples";
        if (samplesRead != frames)
        {
            qDebug() << "read failed";
            return;
        }
        qDebug() << buffer;
        qDebug() << file.write((const char *)buffer, frames * 2 * sizeof(short));
        //file.write(QByteArray("test"));
        qDebug() << "wrote samples";
    }
    
    snd_pcm_close(captureHandle);
    file.close();
}

void AudioCapture::startCapture(const QString & fileName)
{
    captureFile = fileName;
    start();
    capturing = true;
}

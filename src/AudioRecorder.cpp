#include <QtAudioCapture/AudioRecorder.h>
#include <QMediaDevices>
#include <QDebug>

namespace QtAudioCapture {

AudioRecorder::AudioRecorder(QObject *parent) : QObject(parent) {}

AudioRecorder::~AudioRecorder() {
    stop();
}

void AudioRecorder::setDevice(const QAudioDevice &device) {
    mDevice = device;
}

void AudioRecorder::start() {
    stop();

    const QAudioDevice &dev = mDevice.isNull()
                            ? QMediaDevices::defaultAudioInput()
                            : mDevice;

    if (dev.isNull()) {
        emit errorEncountered("No audio input device available.");
        return;
    }

    // Request 16kHz mono Int16 — the closest native format to what
    // whisper wants. AudioResampler handles any mismatch.
    QAudioFormat format;
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    if (!dev.isFormatSupported(format)) {
        format = dev.preferredFormat();
        qInfo() << "QtAudioCapture: requested format unsupported, falling back to"
                << format.sampleRate() << "Hz"
                << format.channelCount() << "ch"
                << format.sampleFormat();
    }

    mAudioSource = new QAudioSource(dev, format, this);
    mIODevice    = mAudioSource->start();

    if (!mIODevice) {
        emit errorEncountered("Failed to start audio source.");
        return;
    }

    connect(mIODevice, &QIODevice::readyRead, this, &AudioRecorder::onAudioDataReady);

    qInfo() << "QtAudioCapture: recording started —"
            << format.sampleRate() << "Hz"
            << format.channelCount() << "ch";
}

void AudioRecorder::stop() {
    if (mAudioSource) {
        mAudioSource->stop();
        delete mAudioSource;
        mAudioSource = nullptr;
        mIODevice    = nullptr;
    }
}

bool AudioRecorder::isRunning() const {
    return mAudioSource &&
           mAudioSource->state() == QAudio::ActiveState;
}

QAudioFormat AudioRecorder::activeFormat() const {
    return mAudioSource ? mAudioSource->format() : QAudioFormat{};
}

void AudioRecorder::onAudioDataReady() {
    if (!mIODevice) return;
    const QByteArray data = mIODevice->readAll();
    if (!data.isEmpty())
        emit audioDataReady(data, mAudioSource->format());
}

} // namespace QtAudioCapture

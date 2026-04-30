#include <QtAudioCapture/AudioFileDecoder.h>
#include <QDebug>
#include <QUrl>

namespace QtAudioCapture {

AudioFileDecoder::AudioFileDecoder(QObject *parent) : QObject(parent) {
    mDecoder = new QAudioDecoder(this);

    connect(mDecoder, &QAudioDecoder::bufferReady, this, &AudioFileDecoder::onBufferReady);
    connect(mDecoder, &QAudioDecoder::finished,    this, &AudioFileDecoder::onFinished);
    connect(mDecoder, qOverload<QAudioDecoder::Error>(&QAudioDecoder::error),
            this, &AudioFileDecoder::onError);
}

AudioFileDecoder::~AudioFileDecoder() {
    stop();
}

void AudioFileDecoder::setFile(const QString &filePath) {
    mDecoder->setSource(QUrl::fromLocalFile(filePath));
}

void AudioFileDecoder::start() {
    if (mDecoder->source().isEmpty()) {
        qCritical() << "QtAudioCapture:" << errorToString(Error::DecodingFailed);
        emit errorOccurred(Error::DecodingFailed);
        return;
    }
    mDecoder->start();
}

void AudioFileDecoder::stop() {
    mDecoder->stop();
}

bool AudioFileDecoder::isRunning() const {
    return mDecoder->isDecoding();
}

void AudioFileDecoder::onBufferReady() {
    while (mDecoder->bufferAvailable()) {
        const QAudioBuffer buffer = mDecoder->read();
        if (!buffer.isValid()) continue;

        const QByteArray data(
            reinterpret_cast<const char *>(buffer.constData<char>()),
            buffer.byteCount());

        emit audioDataReady(data, buffer.format());
    }
}

void AudioFileDecoder::onFinished() {
    qInfo() << "QtAudioCapture: file decoding finished.";
    emit finished();
}

void AudioFileDecoder::onError(QAudioDecoder::Error /*error*/) {
    qCritical() << "QtAudioCapture:" << errorToString(Error::DecodingFailed)
                << mDecoder->errorString();
    emit errorOccurred(Error::DecodingFailed);
}

} // namespace QtAudioCapture

#include <QtAudioCapture/AudioFileDecoder.h>
#include <QDebug>
#include <QUrl>
#include <QAudioDecoder>

namespace QtAudioCapture {

AudioFileDecoder::AudioFileDecoder(QObject *parent) : QObject(parent) {
    mDecoder = new QAudioDecoder(this);

    connect(mDecoder, &QAudioDecoder::bufferReady,
            this,     &AudioFileDecoder::onBufferReady);
    connect(mDecoder, &QAudioDecoder::finished,
            this,     &AudioFileDecoder::onFinished);
    connect(mDecoder, &QAudioDecoder::errorOccurred,
            this,     &AudioFileDecoder::onError);
}

AudioFileDecoder::~AudioFileDecoder() {
    stop();
}

void AudioFileDecoder::setFile(const QString &filePath) {
    mDecoder->setSource(QUrl::fromLocalFile(filePath));
}

void AudioFileDecoder::start() {
    if (mDecoder->source().isEmpty()) {
        emit errorEncountered("No file path set.");
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

        // QAudioBuffer::constData<T> gives a typed pointer into the raw PCM.
        // We copy into a QByteArray so the downstream interface stays uniform
        // with AudioRecorder.
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

void AudioFileDecoder::onError(QAudioDecoder::Error error) {
    emit errorEncountered(
        QString("Audio decoder error: %1").arg(mDecoder->errorString()));
    Q_UNUSED(error)
}

} // namespace QtAudioCapture

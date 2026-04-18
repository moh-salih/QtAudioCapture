#pragma once
#include <QObject>
#include <QAudioDecoder>
#include <QAudioFormat>
#include <QtAudioCapture/Types.h>

namespace QtAudioCapture {

    class AudioFileDecoder : public QObject {
        Q_OBJECT
    public:
        explicit AudioFileDecoder(QObject *parent = nullptr);
        ~AudioFileDecoder() override;

        void setFile(const QString &filePath);
        void start();
        void stop();

        bool isRunning() const;

    signals:
        void audioDataReady(const QByteArray &pcmData, const QAudioFormat &format);
        void finished();
        void errorEncountered(const QString &message);

    private slots:
        void onBufferReady();
        void onFinished();
        void onError(QAudioDecoder::Error error);

    private:
        QAudioDecoder *mDecoder = nullptr;
    };

} // namespace QtAudioCapture

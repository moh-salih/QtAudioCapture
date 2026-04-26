#pragma once
#include <QObject>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSource>
#include <QIODevice>
#include <QtAudioCapture/Types.h>

namespace QtAudioCapture {

    class AudioRecorder : public QObject {
        Q_OBJECT
    public:
        explicit AudioRecorder(QObject *parent = nullptr);
        ~AudioRecorder() override;

        void setDevice(const QAudioDevice &device);
        void start(int sampleRate = 16000, int channelCount = 1);
        void stop();

        bool          isRunning()   const;
        QAudioFormat  activeFormat() const;

    signals:
        void audioDataReady(const QByteArray &pcmData, const QAudioFormat &format);
        void errorEncountered(const QString &message);

    private slots:
        void onAudioDataReady();

    private:
        QAudioDevice  mDevice;
        QAudioSource *mAudioSource = nullptr;
        QIODevice    *mIODevice    = nullptr;
    };

} // namespace QtAudioCapture

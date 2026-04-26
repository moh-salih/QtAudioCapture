#pragma once
#include <QString>
#include <QAudioDevice>

namespace QtAudioCapture {

    enum class Source {
        Microphone,
        File
    };

    enum class Status {
        Idle,
        Running,
        Error
    };

    struct Config {
        Source       source              = Source::Microphone;
        QAudioDevice device;                                 // default-constructed = system default
        QString      filePath;                               // used when source == File

        int          windowDurationMs    = 5000;
        int          stepDurationMs      = 4500;   
        int          targetSampleRate    = 16000;
        int          channelCount        = 1;
    };

} // namespace QtAudioCapture

Q_DECLARE_METATYPE(QtAudioCapture::Status)

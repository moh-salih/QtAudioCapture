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

    enum class Error {
        DeviceUnavailable,
        FormatUnsupported,
        RecordingStartFailed,
        DecodingFailed,
        ConfigNotSet
    };

    struct Config {
        Source       source              = Source::Microphone;
        QAudioDevice device;
        QString      filePath;

        int          windowDurationMs    = 5000;
        int          stepDurationMs      = 4500;
        int          targetSampleRate    = 16000;
        int          channelCount        = 1;
    };

    inline QString errorToString(Error error) {
        switch (error) {
            case Error::DeviceUnavailable:   return "No audio input device available.";
            case Error::FormatUnsupported:   return "Audio format is not supported.";
            case Error::RecordingStartFailed:return "Failed to start audio source.";
            case Error::DecodingFailed:      return "Audio file decoding failed.";
            case Error::ConfigNotSet:        return "setConfig() must be called before start().";
            default:                         return "Unknown error.";
        }
    }

} // namespace QtAudioCapture

Q_DECLARE_METATYPE(QtAudioCapture::Status)
Q_DECLARE_METATYPE(QtAudioCapture::Error)

#pragma once
#include <QAudioFormat>
#include <QVector>

namespace QtAudioCapture {

    class AudioResampler {
    public:
        // Converts raw PCM bytes in sourceFormat to 16kHz mono float samples.
        // Returns an empty vector if the format is unsupported.
        static QVector<float> resample(const QByteArray &pcmData, const QAudioFormat &sourceFormat, int targetSampleRate);
    private:
        static QVector<float> toMonoFloat(const QByteArray &data, const QAudioFormat &format);

        static QVector<float> resampleLinear(const QVector<float> &input, int sourceSampleRate, int targetSampleRate);
    };

} // namespace QtAudioCapture

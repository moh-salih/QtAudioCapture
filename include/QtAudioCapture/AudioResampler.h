#pragma once
#include <QAudioFormat>
#include <vector>

namespace QtAudioCapture {

    class AudioResampler {
    public:
        // Converts raw PCM bytes in sourceFormat to 16kHz mono float samples.
        // Returns an empty vector if the format is unsupported.
        static std::vector<float> resample(const QByteArray   &pcmData,
                                           const QAudioFormat &sourceFormat,
                                           int                 targetSampleRate);
    private:
        static std::vector<float> toMonoFloat(const QByteArray   &data,
                                              const QAudioFormat &format);
        static std::vector<float> resampleLinear(const std::vector<float> &input,
                                                 int                       sourceSampleRate,
                                                 int                       targetSampleRate);
    };

} // namespace QtAudioCapture

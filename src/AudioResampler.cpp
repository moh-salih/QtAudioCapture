#include <QtAudioCapture/AudioResampler.h>
#include <QDebug>
#include <cstring>

namespace QtAudioCapture {

std::vector<float> AudioResampler::resample(const QByteArray   &pcmData,
                                             const QAudioFormat &sourceFormat,
                                             int                 targetSampleRate) {
    if (pcmData.isEmpty()) return {};

    const std::vector<float> mono = toMonoFloat(pcmData, sourceFormat);
    if (mono.empty()) return {};

    const int sourceSampleRate = sourceFormat.sampleRate();
    if (sourceSampleRate == targetSampleRate) return mono;

    return resampleLinear(mono, sourceSampleRate, targetSampleRate);
}

std::vector<float> AudioResampler::toMonoFloat(const QByteArray   &data,
                                                const QAudioFormat &format) {
    const int channelCount = format.channelCount();
    const int bytesPerSample = format.bytesPerSample();

    if (bytesPerSample == 0 || channelCount == 0) {
        qWarning() << "QtAudioCapture: unsupported audio format —"
                   << format.sampleFormat() << channelCount << "ch";
        return {};
    }

    const int totalSamples   = data.size() / bytesPerSample;
    const int frameCount     = totalSamples / channelCount;
    const auto *raw          = reinterpret_cast<const uint8_t *>(data.constData());

    std::vector<float> mono;
    mono.reserve(frameCount);

    for (int f = 0; f < frameCount; ++f) {
        float sum = 0.0f;
        for (int c = 0; c < channelCount; ++c) {
            const int offset = (f * channelCount + c) * bytesPerSample;
            float sample = 0.0f;

            switch (format.sampleFormat()) {
                case QAudioFormat::Int16: {
                    int16_t s;
                    std::memcpy(&s, raw + offset, 2);
                    sample = s / 32768.0f;
                    break;
                }
                case QAudioFormat::Int32: {
                    int32_t s;
                    std::memcpy(&s, raw + offset, 4);
                    sample = s / 2147483648.0f;
                    break;
                }
                case QAudioFormat::UInt8: {
                    sample = (raw[offset] - 128) / 128.0f;
                    break;
                }
                case QAudioFormat::Float: {
                    std::memcpy(&sample, raw + offset, 4);
                    break;
                }
                default:
                    qWarning() << "QtAudioCapture: unhandled sample format"
                               << format.sampleFormat();
                    return {};
            }
            sum += sample;
        }
        mono.push_back(sum / static_cast<float>(channelCount));
    }

    return mono;
}

std::vector<float> AudioResampler::resampleLinear(const std::vector<float> &input,
                                                   int                       sourceSampleRate,
                                                   int                       targetSampleRate) {
    if (input.empty() || sourceSampleRate <= 0 || targetSampleRate <= 0) return {};

    const double ratio      = static_cast<double>(sourceSampleRate) / targetSampleRate;
    const int    outputSize = static_cast<int>(input.size() / ratio);

    std::vector<float> output;
    output.reserve(outputSize);

    for (int i = 0; i < outputSize; ++i) {
        const double srcPos   = i * ratio;
        const int    srcIndex = static_cast<int>(srcPos);
        const float  frac     = static_cast<float>(srcPos - srcIndex);

        if (srcIndex + 1 < static_cast<int>(input.size())) {
            output.push_back(input[srcIndex] * (1.0f - frac) +
                             input[srcIndex + 1] * frac);
        } else {
            output.push_back(input[srcIndex]);
        }
    }

    return output;
}

} // namespace QtAudioCapture

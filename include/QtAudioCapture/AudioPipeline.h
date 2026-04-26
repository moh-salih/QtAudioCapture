#pragma once
#include <QObject>
#include <vector>
#include <QtAudioCapture/Types.h>

namespace QtAudioCapture {

class AudioRecorder;
class AudioFileDecoder;

class AudioPipeline : public QObject {
    Q_OBJECT
public:
    explicit AudioPipeline(QObject *parent = nullptr);
    ~AudioPipeline() override;

    void setConfig(const Config &config);

    void start();
    void stop();

    Status status()    const;
    bool   isRunning() const;

signals:
    void windowReady(const std::vector<float> &samples);
    void statusChanged(QtAudioCapture::Status status);
    void errorEncountered(const QString &message);

    // Emitted only in File mode when decoding completes.
    // The final partial window (if any) is flushed before this fires.
    void fileDecodingFinished();

private slots:
    void onAudioDataReady(const QByteArray &pcmData, const QAudioFormat &format);
    void onFileFinished();
    void onError(const QString &message);

private:
    void processFloatSamples(const std::vector<float> &samples);
    void flushWindow();
    void setStatus(Status status);

    Config            mConfig;
    AudioRecorder    *mRecorder    = nullptr;
    AudioFileDecoder *mFileDecoder = nullptr;
    Status            mStatus      = Status::Idle;

    // Ring buffer for windowing
    std::vector<float> mSampleBuffer;
    int                mWindowSize  = 0;   // samples
    int                mStepSize    = 0;
};

} // namespace QtAudioCapture

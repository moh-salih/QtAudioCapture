#pragma once
#include <QObject>
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
    void windowReady(const QVector<float> &samples);
    void statusChanged(QtAudioCapture::Status status);
    void errorOccurred(QtAudioCapture::Error error);
    void fileDecodingFinished();

private slots:
    void onAudioDataReady(const QByteArray &pcmData, const QAudioFormat &format);
    void onFileFinished();
    void onError(QtAudioCapture::Error error);

private:
    void processFloatSamples(const QVector<float> &samples);
    void flushWindow();
    void setStatus(Status status);

    Config             mConfig;
    AudioRecorder    * mRecorder    = nullptr;
    AudioFileDecoder * mFileDecoder = nullptr;
    Status             mStatus      = Status::Idle;

    QVector<float>     mSampleBuffer;
    int                mWindowSize  = 0;
    int                mStepSize    = 0;
};

} // namespace QtAudioCapture

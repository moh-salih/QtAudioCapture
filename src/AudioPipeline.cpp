#include <QtAudioCapture/AudioPipeline.h>
#include <QtAudioCapture/AudioRecorder.h>
#include <QtAudioCapture/AudioFileDecoder.h>
#include <QtAudioCapture/AudioResampler.h>
#include <QDebug>

namespace QtAudioCapture {

AudioPipeline::AudioPipeline(QObject *parent) : QObject(parent) {}

AudioPipeline::~AudioPipeline() {
    stop();
}

void AudioPipeline::setConfig(const Config &config) {
    Q_ASSERT_X(config.stepDurationMs > 0,
               "AudioPipeline::setConfig",
               "stepDurationMs must be greater than zero");
    Q_ASSERT_X(config.stepDurationMs < config.windowDurationMs,
               "AudioPipeline::setConfig",
               "stepDurationMs must be less than windowDurationMs");

    mConfig     = config;
    mWindowSize = (mConfig.targetSampleRate * mConfig.windowDurationMs) / 1000;
    mStepSize   = (mConfig.targetSampleRate * mConfig.stepDurationMs)   / 1000;
}
void AudioPipeline::start() {
    if (mStatus == Status::Running) return;

    if (mWindowSize == 0 || mStepSize == 0) {
        qCritical() << "QtAudioCapture:" << errorToString(Error::ConfigNotSet);
        emit errorOccurred(Error::ConfigNotSet);
        return;
    }

    mSampleBuffer.clear();
    mSampleBuffer.reserve(mWindowSize * 2);

    if (mConfig.source == Source::Microphone) {
        // tear down file decoder if switching source
        if (mFileDecoder) {
            mFileDecoder->stop();
            delete mFileDecoder;
            mFileDecoder = nullptr;
        }
        if (!mRecorder) {
            mRecorder = new AudioRecorder(this);
            connect(mRecorder, &AudioRecorder::audioDataReady,
                    this,      &AudioPipeline::onAudioDataReady);
            connect(mRecorder, &AudioRecorder::errorOccurred,
                    this,      &AudioPipeline::onError);
        }
        mRecorder->setDevice(mConfig.device);
        mRecorder->start(mConfig.targetSampleRate, mConfig.channelCount);

    } else {
        // tear down recorder if switching source
        if (mRecorder) {
            mRecorder->stop();
            delete mRecorder;
            mRecorder = nullptr;
        }
        if (!mFileDecoder) {
            mFileDecoder = new AudioFileDecoder(this);
            connect(mFileDecoder, &AudioFileDecoder::audioDataReady,
                    this,         &AudioPipeline::onAudioDataReady);
            connect(mFileDecoder, &AudioFileDecoder::finished,
                    this,         &AudioPipeline::onFileFinished);
            connect(mFileDecoder, &AudioFileDecoder::errorOccurred,
                    this,         &AudioPipeline::onError);
        }
        mFileDecoder->setFile(mConfig.filePath);
        mFileDecoder->start();
    }

    setStatus(Status::Running);
}

void AudioPipeline::processFloatSamples(const QVector<float> &samples) {
    mSampleBuffer.append(samples);

    while (mSampleBuffer.size() >= mWindowSize) {
        emit windowReady(mSampleBuffer.sliced(0, mWindowSize));
        mSampleBuffer.remove(0, mStepSize);
    }
}

void AudioPipeline::flushWindow() {
    if (mSampleBuffer.isEmpty()) return;
    while (mSampleBuffer.size() < mWindowSize)
        mSampleBuffer.append(0.0f);
    emit windowReady(mSampleBuffer);
    mSampleBuffer.clear();
}

void AudioPipeline::stop() {
    if (mRecorder)    mRecorder->stop();
    if (mFileDecoder) mFileDecoder->stop();
    mSampleBuffer.clear();
    if (mStatus != Status::Error)
        setStatus(Status::Idle);
}

Status AudioPipeline::status() const {
    return mStatus;
}

bool AudioPipeline::isRunning() const {
    return mStatus == Status::Running;
}

void AudioPipeline::onAudioDataReady(const QByteArray &pcmData, const QAudioFormat &format) {
    const QVector<float> samples = AudioResampler::resample(pcmData, format, mConfig.targetSampleRate);
    if (samples.isEmpty()) return;
    processFloatSamples(samples);
}


void AudioPipeline::onFileFinished() {
    flushWindow();
    setStatus(Status::Idle);
    emit fileDecodingFinished();
}

void AudioPipeline::onError(QtAudioCapture::Error error) {
    qCritical() << "QtAudioCapture:" << errorToString(error);
    setStatus(Status::Error);
    emit errorOccurred(error);
}

void AudioPipeline::setStatus(Status status) {
    if (mStatus == status) return;
    mStatus = status;
    emit statusChanged(mStatus);
}

} // namespace QtAudioCapture

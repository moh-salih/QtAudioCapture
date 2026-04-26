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
        emit errorEncountered("AudioPipeline: setConfig() must be called before start().");
        return;
    }


    mSampleBuffer.clear();
    mSampleBuffer.reserve(mWindowSize * 2);

    if (mConfig.source == Source::Microphone) {
        if (!mRecorder) {
            mRecorder = new AudioRecorder(this);
            connect(mRecorder, &AudioRecorder::audioDataReady,
                    this,      &AudioPipeline::onAudioDataReady);
            connect(mRecorder, &AudioRecorder::errorEncountered,
                    this,      &AudioPipeline::onError);
        }
        mRecorder->setDevice(mConfig.device);
        mRecorder->start(mConfig.targetSampleRate, mConfig.channelCount);

    } else {
        if (!mFileDecoder) {
            mFileDecoder = new AudioFileDecoder(this);
            connect(mFileDecoder, &AudioFileDecoder::audioDataReady, this, &AudioPipeline::onAudioDataReady);
            connect(mFileDecoder, &AudioFileDecoder::finished, this, &AudioPipeline::onFileFinished);
            connect(mFileDecoder, &AudioFileDecoder::errorEncountered, this, &AudioPipeline::onError);
        }
        mFileDecoder->setFile(mConfig.filePath);
        mFileDecoder->start();
    }

    setStatus(Status::Running);
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

void AudioPipeline::onAudioDataReady(const QByteArray   &pcmData,
                                      const QAudioFormat &format) {
    const std::vector<float> samples = AudioResampler::resample(pcmData, format, mConfig.targetSampleRate);

    if (samples.empty()) return;
    processFloatSamples(samples);
}

void AudioPipeline::processFloatSamples(const std::vector<float> &samples) {
    mSampleBuffer.insert(mSampleBuffer.end(), samples.begin(), samples.end());

    while (static_cast<int>(mSampleBuffer.size()) >= mWindowSize) {
        // Emit exactly one window worth of samples
        const std::vector<float> window(mSampleBuffer.begin(), mSampleBuffer.begin() + mWindowSize);
        emit windowReady(window);

        mSampleBuffer.erase(mSampleBuffer.begin(), mSampleBuffer.begin() + mStepSize);
    }
}

void AudioPipeline::flushWindow() {
    if (mSampleBuffer.empty()) return;

    // Pad the remaining samples to a full window with silence
    mSampleBuffer.resize(mWindowSize, 0.0f);
    emit windowReady(mSampleBuffer);
    mSampleBuffer.clear();
}

void AudioPipeline::onFileFinished() {
    flushWindow();
    setStatus(Status::Idle);
    emit fileDecodingFinished();
}

void AudioPipeline::onError(const QString &message) {
    qCritical() << "QtAudioCapture:" << message;
    setStatus(Status::Error);
    emit errorEncountered(message);
}

void AudioPipeline::setStatus(Status status) {
    if (mStatus == status) return;
    mStatus = status;
    emit statusChanged(mStatus);
}

} // namespace QtAudioCapture

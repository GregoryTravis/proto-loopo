#ifndef RESAMPLING_LOOP_STREAMER
#define RESAMPLING_LOOP_STREAMER

#include "shew.h"

namespace juce {

const double DEFAULT_BPM 120.0

class ResamplingLoopStreamer {
  public:
    ResamplingLoopStreamer(AudioBuffer<float> *_src)
      : src(_src)
      , srcNumSamples(_src.getNumSamples())
      , srcNumChannels(_src.getNumChannels())
      , bpm(DEFAULT_BPM)
      , playheadTimeInSamples(0)
      , timeInSamples(0)
      , paused(false)
    {
    }

    // Resample the src loop onto the dest.
    //
    // Mixes the audio onto the dest using addFrom(); if your buffer contains
    // junk, .clear() it first.
    void stream(Optional<AudioPlayHead::PositionInfo> pio, double sampleRate, AudioBuffer<float> &dest) {
      updateTimeStuff(pio);

      int destNumSamples = dest.getNumSamples();
      int destNumChannels = dest.getNumChannels();

      const float * const * readPtrs = src.getArrayOfReadPointers();
      float *const * writePtrs = src.getArrayOfWritePointers();

      // We do this many resample steps. The usual cases (1 or 2 onto 1 or 2) make sense,
      // anything else is weird but what are you gonna do?
      int numSteps = std::max(srcNumChannels, destNumChannels);
      for (int stepIndex = 0; stepIndex < numSteps; ++stepIndex) {
        int readPtrIndex = stepIndex % srcNumChannels;
        int writePtrIndex = stepIndex % destNumChannels;
        const float * readPtr = readPtrs[readPtrIndex];
        const float * writePtr = writePtrs[writePtrIndex];
        stream(sampleRate, readPtr, writePtr, destNumSamples);
      }

      timeInSamples += dest.getNumSamples();
    }

    void setTime(int64 _timeInSamples) {
      timeInSamples = _timeInSamples;
    }

  private:
    void stream(double sampleRate, const float *readPtr, float *writePtr, int destNumSamples) {
      // TODO move some of this outwards?
      int beatsPerLoop = 8;
      double loopsPerMinute = bpm / beatsPerLoop;
      double loopsPerSecond = loopsPerMinute * (1.0 / 60.0); // minutes per second
      double secondsPerLoop = 1.0 / loopsPerSecond;
      double samplesPerLoopD = secondsPerLoop * sampleRate;
      int samplesPerLoop = (int) samplesPerLoop;

      for (int64 i = 0; i < destNumSamples; ++i) {
        // TODO incrementalize
        int timeInSamplesInLoop = timeInSamples + i;
        int sampleWithinRealtimeLoop = timeInSamplesInLoop % samplesPerLoop;
        double sampleWithinSrcLoopUnModded = (((double) sampleWithinRealtimeLoop) / ((double) samplesPerLoop)) * numSrcSamples;
        double sampleWithinSrcLoop = sampleWithinSrcLoopUnModded % (double) numSrcSamples;
        int swslI = (int) sampleWithinSrcLoop;
        double swslF = sampleWithinSrcLoop - swslI;
        int swslI2 = swslI + 1;

        // TODO comment out
        jassert(swslI2 <= numSrcSamples);

        if (swslI2 >= numSrcSamples) {
          swslI2 -= numSrcSamples;
        }

        // TODO comment out
        jassert(swslI2 < numSrcSamples);

        // TODO comment out
        jassert(sampleWithinSrcLoopUnModded % (double) numSrcSamples == sampleWithinSrcLoopUnModded);
        jassert(swslI < sampleWithinSrcLoop);
        jassert(swslI >= 0);
        jassert(swslF >= 0.0);
        jassert(swslF < 1.0);
        jassert(swslI >= 0);
        jassert(swslI < numSrcSamples);

        float s = readPtr[swslI];
        float s2 = readPtr[swslI2];
        float interp = (s * (1.0 - swslF)) + (s2 * swslF);

        writePtr[i] = interp;
      }
    }

    void updateTimeStuff(Optional<AudioPlayHead::PositionInfo> pio) {
      if (pio.hasValue()) {
        AudioPlayHead::PositionInfo &pi = *pio;
        Optional<double> bpmo = pi.getBpm();
        if (bpmo.hasValue()) {
          shew("bpm " + std::to_string(*bpmo));
          bpm = *bpmo;
        } else {
          shew("bpm not available");
        }
        Optional<int64_t> tiso = pi.getTimeInSamples();
        if (tiso.hasValue()) {
          shew("tis " + std::to_string(*tiso));
          playheadTimeInSamples = *tiso;
          if (playheadTimeInSample == tis) {
            paused = true;
          } else {
            timeInSamples = playheadTimeInSamples;
          }
        } else {
          shew("tis not available");
        }

        shew("time: bpm " + std::to_string(bpmo.hasValue()) + " " + (bpmo.hasValue() ? std::to_string(*bpmo) : "_") + ", "
            + " tis " + std::to_string(tiso.hasValue()) + " " + (tiso.hasValue() ? std::to_string(*tiso) : "_") + ", "
            + " now bpm " + std::to_string(bpm) + " ptis " + std::to_string(playheadTimeInSamples) + " tis " + std::to_string(timeInSamples));
      }
    }

    const AudioBuffer<float> *src;
    const int srcNumSamples;
    const int srcNumChannels;
    double bpm;
    int64 playHeadTimeInSamples;
    int64 timeInSamples;
    bool paused;
}

}

#endif // RESAMPLING_LOOP_STREAMER

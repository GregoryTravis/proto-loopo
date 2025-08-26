#ifndef RESAMPLING_LOOP_STREAMER
#define RESAMPLING_LOOP_STREAMER

#include <cmath>

#pragma once

#include "shew.h"

//namespace juce {

const double DEFAULT_BPM = 120.0;

// When a note is first turned on, it is set to asr_as and the envelope gain (EG) is set to 0.
// When a note is first turned off, it is set to asr_r.
// In each frame, audio is possibly updated:
//   if asr_as or asr_r, audio is streamed with gain EG
//   if asr_off, audio is not streamed but time is advanced
// x In each frame, the EG is updated:
// x   if asr_as, it is increased by ATTACK_GAIN_DELTA
// x   if asr_r, it is increased by RELEASE_GAIN_DELTA
// When EG reaches <= 0 in asr_r, it goes into state asr_off
enum ASR {
  // Attack + sustain
  asr_as,
  // Release
  asr_r,
  // Done
  asr_off
};

class ResamplingLoopStreamer {
  public:
    ResamplingLoopStreamer(AudioBuffer<float> *_src)
      : src(_src)
      , srcNumSamples(_src->getNumSamples())
      , srcNumChannels(_src->getNumChannels())
      // The bpm is initialized to a default value, but is updated with the value obtained in each batch from the playhead.
      , bpm(DEFAULT_BPM)
      // playheadTimeInSamples is the value obtained from the playhead. If its
      // value is the same from batch to batch (or cannot be obtained from the
      // playhead), then it is not used, since we are effectively paused, and
      // so timeInSamples is incremented by the size of batch at the end of the
      // batch. Otherwise, it is set to be
      // equal to playheadTimeInSamples.
      , playheadTimeInSamples(0)
      , timeInSamples(0)
      , isNoteOn(false)
      , asr(asr_off)
      , eg(0.0)
    {
    }

    // Resample the src loop onto the dest.
    //
    // The generated audio is added to the value in dest, so it should be cleared the first time around.
    void stream(Optional<AudioPlayHead::PositionInfo> pio, double sampleRate, AudioBuffer<float> &dest) {
      updateTimeStuff(pio);

      int destNumSamples = dest.getNumSamples();
      int destNumChannels = dest.getNumChannels();

      const float * const * readPtrs = src->getArrayOfReadPointers();
      float *const * writePtrs = src->getArrayOfWritePointers();

      // We do this many resample steps. The usual cases (1 or 2 onto 1 or 2) make sense,
      // anything else is weird but what are you gonna do?
      int numSteps = std::max(srcNumChannels, destNumChannels);
      for (int stepIndex = 0; stepIndex < numSteps; ++stepIndex) {
        int readPtrIndex = stepIndex % srcNumChannels;
        int writePtrIndex = stepIndex % destNumChannels;
        const float * readPtr = readPtrs[readPtrIndex];
        float * writePtr = writePtrs[writePtrIndex];
        stream(sampleRate, readPtr, writePtr, destNumSamples);
      }

      timeInSamples += dest.getNumSamples();
    }

    // When a note is first turned on, it is set to asr_as and the envelope gain (EG) is set to 0.
    // When a note is first turned off, it is set to asr_r.
    void updateMidi(bool newIsNoteOn) {
      if (!isNoteOn && newIsNoteOn) {
        // Note just turned on
        asr = asr_as;
        eg = 0;
        isNoteOn = true;
      } else if (isNoteOn && !newIsNoteOn) {
        // Note just turned off
        asr = asr_r;
        isNoteOn = false;
      }
    }

    // A note is on if it's not in asr_off.
    bool isOn() {
      return isNoteOn;
    }

  private:
    void stream(double sampleRate, const float *readPtr, float *writePtr, int destNumSamples) {
      // TODO move some of this outwards?
      int beatsPerLoop = 8;
      double loopsPerMinute = bpm / beatsPerLoop;
      double loopsPerSecond = loopsPerMinute / 60.0;
      double secondsPerLoop = 1.0 / loopsPerSecond;
      double samplesPerLoopD = secondsPerLoop * sampleRate;
      // Quantizing this value only because I can't quite wrap my head around
      // not doing it, but it would probably work fine. TODO: try it.
      int samplesPerLoop = (int) samplesPerLoopD;

      for (int64 i = 0; i < destNumSamples; ++i) {
        // TODO incrementalize
        int64 timeInSamplesInLoop = timeInSamples + i;
        int64 sampleWithinRealtimeLoop = timeInSamplesInLoop % samplesPerLoop;
        double sampleWithinSrcLoopUnModded = (((double) sampleWithinRealtimeLoop) / ((double) samplesPerLoop)) * srcNumSamples;
        double sampleWithinSrcLoop = fmod(sampleWithinSrcLoopUnModded, (double) srcNumSamples);
        int64 swslI = (int) sampleWithinSrcLoop;
        double swslF = sampleWithinSrcLoop - swslI;
        int64 swslI2 = swslI + 1;

        // TODO comment out
        jassert(swslI2 <= srcNumSamples);

        if (swslI2 >= srcNumSamples) {
          swslI2 -= srcNumSamples;
        }

        // TODO comment out
        jassert(swslI2 >= 0);
        jassert(swslI2 < srcNumSamples);

        // TODO comment out
        jassert(fmod(sampleWithinSrcLoopUnModded, (double) srcNumSamples) == sampleWithinSrcLoopUnModded);
        jassert(swslI < sampleWithinSrcLoop);
        jassert(swslI >= 0);
        jassert(swslF >= 0.0);
        jassert(swslF < 1.0);
        jassert(swslI >= 0);
        jassert(swslI < srcNumSamples);

        float s = readPtr[swslI];
        float s2 = readPtr[swslI2];
        float interp = (s * (1.0 - swslF)) + (s2 * swslF);

        writePtr[i] += interp;
      }
    }

    void updateTimeStuff(Optional<AudioPlayHead::PositionInfo> pio) {
      if (pio.hasValue()) {
        AudioPlayHead::PositionInfo &pi = *pio;
        Optional<double> bpmo = pi.getBpm();
        if (bpmo.hasValue()) {
          bpm = *bpmo;
        }
        Optional<int64_t> tiso = pi.getTimeInSamples();
        if (tiso.hasValue()) {
          if (playheadTimeInSamples != *tiso) {
            playheadTimeInSamples = *tiso;
            timeInSamples = playheadTimeInSamples;
          }
        }

        shew("time: bpm " + std::to_string(bpmo.hasValue()) + " " + (bpmo.hasValue() ? std::to_string(*bpmo) : "_") + ", "
            + " tis " + std::to_string(tiso.hasValue()) + " " + (tiso.hasValue() ? std::to_string(*tiso) : "_") + ", "
            + " now bpm " + std::to_string(bpm) + " ptis " + std::to_string(playheadTimeInSamples) + " tis " + std::to_string(timeInSamples));
      }
    }

    AudioBuffer<float> *src;
    const int srcNumSamples;
    const int srcNumChannels;
    double bpm;
    int64 playheadTimeInSamples;
    int64 timeInSamples;
    bool isNoteOn;
    ASR asr;
    float eg;
};

//}

#endif // RESAMPLING_LOOP_STREAMER

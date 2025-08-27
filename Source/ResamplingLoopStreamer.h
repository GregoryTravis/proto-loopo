#ifndef RESAMPLING_LOOP_STREAMER
#define RESAMPLING_LOOP_STREAMER

#include <cmath>

#pragma once

#include "shew.h"

//namespace juce {

const double DEFAULT_BPM = 120.0;
// Time taken to go from 0 to 1 during attack
const double ATTACK_DURATION_S = 0.001;
// Time taken to go from 1 to 0 during release
const double RELEASE_DURATION_S = 0.05;

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

#define s(x) std::to_string(x)

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
      float *const * writePtrs = dest.getArrayOfWritePointers();

      float oeg = eg;

      bool just_advance = false;
      float eg_change_per_sample;
      float eg_end;
      if (asr == asr_as) {
        // How long to go from 0 to 1, in samples
        float attack_duration_samp = ATTACK_DURATION_S * sampleRate;
        eg_change_per_sample = 1.0 / attack_duration_samp;
        eg_end = std::fmin(1.0, eg + (eg_change_per_sample * destNumSamples));
      } else if (asr == asr_r) {
        // How long to go from 1 to 0, in samples
        float attack_duration_samp = RELEASE_DURATION_S * sampleRate;
        eg_change_per_sample = -1.0 / attack_duration_samp;
        eg_end = std::fmax(0.0, eg + (eg_change_per_sample * destNumSamples));
      } else {
        just_advance = true;
      }

      if (!just_advance) {
        // We do this many resample steps. The usual cases (1 or 2 onto 1 or 2) make sense,
        // anything else is weird but what are you gonna do?
        int numSteps = std::fmax(srcNumChannels, destNumChannels);
        for (int stepIndex = 0; stepIndex < numSteps; ++stepIndex) {
          int readPtrIndex = stepIndex % srcNumChannels;
          int writePtrIndex = stepIndex % destNumChannels;

          const float * readPtr = readPtrs[readPtrIndex];
          float * writePtr = writePtrs[writePtrIndex];

          /*
          shew("ptrs " +
              std::to_string((int64)readPtrA) + " " + std::to_string((int64)readPtrI) + " " +
              std::to_string((int64)writePtrA) + " " + std::to_string((int64)writePtrI) + " " +
              std::to_string(readPtrA == writePtrA) + " " +
              std::to_string(readPtrI == writePtrI));
              */

          /* const float * readPtr2 = readPtrs[readPtrIndex]; */

          /* shew("stream " + std::to_string(numSteps) + " " + std::to_string(stepIndex) + " " + std::to_string(readPtrIndex) + " " + */
          /*       std::to_string(writePtrIndex)); */
          stream(sampleRate, readPtr, writePtr, destNumSamples, eg_change_per_sample);

          /* shew("ggg2 " + std::to_string(dest.getSample(0, 0)) + " " +  std::to_string(dest.getSample(1, 0))); */
          /* writePtr[0] = 14 + stepIndex; */
          /* shew("ggg22 " + std::to_string(dest.getSample(0, 0)) + " " +  std::to_string(dest.getSample(1, 0))); */
          /* dest.getWritePointer(writePtrIndex)[0] = 41 + stepIndex; */
          /* shew("ggg23 " + std::to_string(dest.getSample(0, 0)) + " " +  std::to_string(dest.getSample(1, 0))); */
          /* shew("ggg3 " + std::to_string(readPtr2[9])); */
        }

        eg = eg_end;
      }

      // When EG reaches <= 0 in asr_r, it goes into state asr_off
      if (asr == asr_r && eg <= 0.0) {
        asr = asr_off;
      }

      //shew("eg " + std::to_string(eg) + " " + std::to_string(eg_end) + " " + std::to_string(eg_change_per_sample));

      timeInSamples += dest.getNumSamples();

      /* dest.getWritePointer(0)[0] = 12; */
      /* dest.getWritePointer(1)[0] = 13; */
      /* shew("ggg " + std::to_string(dest.getSample(0, 0)) + " " +  std::to_string(dest.getSample(1, 0))); */
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
    void stream(double sampleRate, const float *readPtr, float *writePtr, int destNumSamples, float eg_change_per_sample) {
      // TODO move some of this outwards?
      int beatsPerLoop = 4;
      double loopsPerMinute = bpm / beatsPerLoop;
      double loopsPerSecond = loopsPerMinute / 60.0;
      double secondsPerLoop = 1.0 / loopsPerSecond;
      double samplesPerLoopD = secondsPerLoop * sampleRate;
      // Quantizing this value only because I can't quite wrap my head around
      // not doing it, but it would probably work fine. TODO: try it.
      int samplesPerLoop = (int) samplesPerLoopD;

      float leg = eg;

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
        jassert(swslI <= sampleWithinSrcLoop);
        jassert(swslI >= 0);
        jassert(swslF >= 0.0);
        jassert(swslF < 1.0);
        jassert(swslI >= 0);
        jassert(swslI < srcNumSamples);

        float s = readPtr[swslI];
        float s2 = readPtr[swslI2];
        float interp = (s * (1.0 - swslF)) + (s2 * swslF);

        /* shew("samp tisil " + std::to_string(timeInSamplesInLoop) + " swsl " + std::to_string(sampleWithinSrcLoop) + " ints " + std::to_string(swslI) + " " + std::to_string(swslI2) + " swslF " + */
        /*     std::to_string(swslF) + " vals " + std::to_string(s) + " " + std::to_string(s2) + " "+ std::to_string(interp)); */

        //shew("leg " + s(i) + " " + s(leg));
        writePtr[i] += leg * interp;

        if (asr == asr_as) {
          leg = std::fmin(1.0, leg + eg_change_per_sample);
        } else if (asr == asr_r) {
          leg = std::fmax(0.0, leg + eg_change_per_sample);
        }
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

        /* shew("time: bpm " + std::to_string(bpmo.hasValue()) + " " + (bpmo.hasValue() ? std::to_string(*bpmo) : "_") + ", " */
        /*     + " tis " + std::to_string(tiso.hasValue()) + " " + (tiso.hasValue() ? std::to_string(*tiso) : "_") + ", " */
        /*     + " now bpm " + std::to_string(bpm) + " ptis " + std::to_string(playheadTimeInSamples) + " tis " + std::to_string(timeInSamples)); */
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

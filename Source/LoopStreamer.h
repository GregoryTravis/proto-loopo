#ifndef LOOP_STREAMER
#define LOOP_STREAMER

// Hard-coded to 44.1 because I stink
#define SRATE 44100
#define ATTACK_DURATION_S (0.05)
#define RELEASE_DURATION_S (0.1)

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

// LoopStreamer writes successive segments of a repeating audio loop into an AudioBuffer.
// Does not take ownership of the supplied AudioBuffer.
// It's fine to make a lot of these, they're very cheap.
// Only supports stereo.
// Mixes the audio onto the dest using addFrom(); if your buffer contains junk, .clear() it first.
class LoopStreamer {
public:
  LoopStreamer(juce::AudioBuffer<float> *theLoop)
    : loop(theLoop)
    , currentPosition(0)
    , isNoteOn(false)
    , asr(asr_off)
    , eg(0.0) {
    jassert(loop->getNumSamples() > 0);
  }

  // The original design of this class assumed that we always started at 0 and
  // we always resumed at the sample after the last block, but in order to
  // synchronize with the host timeline, we need to lock our idea of time to
  // the host. Calling this before each stream can do this.  If you don't call
  // this, then the current position will be the last current position + the
  // size of the last block.
  // TODO: just git rid of the old semantics?
  void setTime(int64 timeInSamples) {
    currentPosition = timeInSamples % loop->getNumSamples();
  }

  // In each frame, audio is possibly updated:
  //   if asr_as or asr_r, audio is streamed with gain EG
  //   if asr_off, audio is not streamed but time is advanced
  void updateAudio(AudioBuffer<float> &dest) {
    if (asr != asr_off) {
      stream(dest);
    } else {
      advance(dest.getNumSamples());
    }
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

  // Write the next n samples from the repeating loop into the destination
  // buffer, where n is the destination buffer's length. Advances the current
  // position so that the next write takes up where this one left off.
  //
  // The dest buffer must have at least two channels, and we only write to two.
  //
  // Mixes the audio onto the dest using addFrom(); if your buffer contains junk, .clear() it first.
  void stream(juce::AudioBuffer<float> &dest) {
    jassert(dest.getNumChannels() >= 2);

    /*
    // In each frame, the EG is updated:
    //   if asr_as, it is increased by ATTACK_GAIN_DELTA
    //   if asr_r, it is increased by RELEASE_GAIN_DELTA
    jassert(asr != asr_off);
    if (asr == asr_as) {
      eg += ATTACK_GAIN_DELTA;
      if (eg > 1.0) {
        eg = 1.0;
      }
    } else if (asr == asr_r) {
      eg += RELEASE_GAIN_DELTA;
      if (eg < 0.0) {
        eg = 0.0;
      }
    }
    */

    // The destination buffer is probably smaller than the loop, but it might
    // be much larger (eg if the loop is small).  We have already produced
    // 'currentPosition' samples from the loop, so we are starting after that.
    // We copy from there to the end, or to the end of the dest buffer,
    // whichever is first.
    //
    // If we used the rest of the loop, we reset 'currentPosition' to 0, and
    // possibly loop around to copy more.  'currentPosition' will be < the
    // size of the loop, unless we used the whole thing and might possible
    // wrap around, in which case it will be == the size of the loop. It
    // should never be > the size of the loop.
    int numSamplesRemaining = dest.getNumSamples();

    while (numSamplesRemaining > 0) {
      // How many samples after the current next sample (currentPosition)
      // should we copy? The max is whatever is left after that point:
      int loopSamplesRemaining = loop->getNumSamples() - currentPosition;

      // We know how many samples we have left to write, that tells us where to start writing
      int destBufferPosition = dest.getNumSamples() - numSamplesRemaining;

      // But it shouldn't be more than what can fit in the dest buffer
      int samplesToCopy = std::min(loopSamplesRemaining, numSamplesRemaining);

      jassert(destBufferPosition >= 0 && destBufferPosition < dest.getNumSamples());
      /* juce::Logger::getCurrentLogger()->writeToLog( */
      /*     "Write: currentPosition " + std::to_string(currentPosition) + " samplesToCopy " + std::to_string(samplesToCopy) + */
      /*     " destBufferPosition " + std::to_string(destBufferPosition) + " dest len " + std::to_string(dest.getNumSamples()) + */
      /*     " loop len " + std::to_string(loop->getNumSamples())); */

      float eg_delta = 0;
      // e.g. block 441 sr 44100 so 0.01s
      float blockDurationS = ((float)samplesToCopy) / ((float)SRATE);
      if (asr == asr_as) {
        // Ramp up
        eg_delta = (blockDurationS / ATTACK_DURATION_S);
      } else {
        // Ramp down
        eg_delta = -(blockDurationS / RELEASE_DURATION_S);
      }
      float end_eg = eg + eg_delta;
      if (asr == asr_as) {
        if (end_eg > 1.0) {
          end_eg = 1.0;
        }
      } else {
        if (end_eg < 0.0) {
          end_eg = 0.0;
        }
      }

      // TODO ramp smoothly from eg to end_eg, which we're not doing because it
      // doesn't take an AudioBuffer.
      dest.addFrom(0, destBufferPosition, *loop, 0, currentPosition, samplesToCopy, eg);
      dest.addFrom(1, destBufferPosition, *loop, 1, currentPosition, samplesToCopy, eg);

      // If we used up the rest of the loop, we'll likely have more to copy and this will be >0.
      numSamplesRemaining -= samplesToCopy;
      currentPosition += samplesToCopy;

      // Sanity check
      jassert((destBufferPosition + samplesToCopy + numSamplesRemaining) == dest.getNumSamples());

      // == in the case that we used the rest of the loop
      jassert(currentPosition <= loop->getNumSamples());

      // If we did use the rest of the loop, wrap around. We might have more to copy, or we might not.
      if (currentPosition == loop->getNumSamples()) {
        currentPosition = 0;
      }

      // Update eg to last ramp end
      eg = end_eg;
    }

    // When EG reaches <= 0 in asr_r, it goes into state asr_off
    if (asr == asr_r && eg <= 0.0) {
      asr = asr_off;
    }
  }

  // Advance the current position. Useful for keeping the position in step with
  // time just as if we were still writing the audio, but without writing the
  // audio.
  void advance(int numSamples) {
    currentPosition = (currentPosition + numSamples) % loop->getNumSamples();
  }

private:
  juce::AudioBuffer<float> *loop;
  int currentPosition;
  bool isNoteOn;
  ASR asr;
  float eg;
};

#endif // LOOP_STREAMER

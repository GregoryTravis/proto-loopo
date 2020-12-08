#ifndef LOOP_STREAMER
#define LOOP_STREAMER

//#pragma once

//#include <JuceHeader.h>

#include "DemoUtilities.h"

#include <array>
#include <atomic>
#include <memory>
#include <vector>
#include <tuple>
#include <iomanip>
#include <sstream>
#include <functional>
#include <mutex>

// LoopStreamer writes successive segments of a repeating audio loop into an AudioBuffer.
// Does not take ownership of the supplied AudioBuffer.
// It's fine to make a lot of these, they're very cheap.
// Only supports stereo.
class LoopStreamer {
public:
  LoopStreamer(juce::AudioBuffer<float> *theLoop)
    : loop(theLoop)
    , currentPosition(0) {
    jassert(loop->getNumSamples() > 0);
  }

  // Write the next n samples from the repeating loop into the destination
  // buffer, where n is the destination buffer's length. Advances the current
  // position so that the next write takes up where this one left off.
  //
  // The output buffer must have at least two channels, and we only write to two.
  void stream(juce::AudioBuffer<float> &dest) {
    jassert(dest.getNumChannels() >= 2);

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
      int myLoopSamplesRemaining = loop->getNumSamples() - currentPosition;

      // We know how many samples we have left to write, that tells us where to start writing
      int outputBufferPosition = dest.getNumSamples() - numSamplesRemaining;

      // But it shouldn't be more than what can fit in the output buffer
      int samplesToCopy = std::min(myLoopSamplesRemaining, numSamplesRemaining);

      jassert(outputBufferPosition >= 0 && outputBufferPosition < dest.getNumSamples());
      juce::Logger::getCurrentLogger()->writeToLog(
          "Write: currentPosition " + std::to_string(currentPosition) + " samplesToCopy " + std::to_string(samplesToCopy) +
          " outputBufferPosition " + std::to_string(outputBufferPosition) + " output len " + std::to_string(dest.getNumSamples()) +
          " loop len " + std::to_string(loop->getNumSamples()));
      dest.copyFrom(0, outputBufferPosition, *loop, 0, currentPosition, samplesToCopy);
      dest.copyFrom(1, outputBufferPosition, *loop, 1, currentPosition, samplesToCopy);

      // If we used up the rest of the loop, we'll likely have more to copy and this will be >0.
      numSamplesRemaining -= samplesToCopy;
      currentPosition += samplesToCopy;

      // Sanity check
      jassert((outputBufferPosition + samplesToCopy + numSamplesRemaining) == dest.getNumSamples());

      // == in the case that we used the rest of the loop
      jassert(currentPosition <= loop->getNumSamples());

      // If we did use the rest of the loop, wrap around. We might have more to copy, or we might not.
      if (currentPosition == loop->getNumSamples()) {
        currentPosition = 0;
      }
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
};

#endif // LOOP_STREAMER

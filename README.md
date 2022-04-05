Loopo: Beat-synchronized loop sampler plugin

====

<p align="center">
  <img src="https://raw.githubusercontent.com/GregoryTravis/loopo/master/images/loopo.gif">
</p>

Loopo is a sampler with a very simple twist: it does not start playing a sample
at the start, but in the middle. If your source material rhythmically matches,
then everything you play will be in rhythm, no matter how you play.

One way to think about is that all your loops are playing at the same time, but
are muted. A MIDI note-on unmutes the loop, and a note-off re-mutes it.

Loopo was built with [Juce](https://juce.com/) and works in most mainstream
digital audio workstations.

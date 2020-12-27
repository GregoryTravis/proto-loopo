#ifndef Wrapper_h
#define Wrapper_h

//#include "../JuceLibraryCode/JuceHeader.h"

typedef struct Midi {
  bool isOn;
  int noteNumber;
} Midi;

class Wrapper {
public:
  Wrapper();
  ~Wrapper();

  int fuu();
  void frobb(float *f, int len);
  void frobbMidi(Midi *midis, int count);
private:
  void init();
};

#endif // Wrapper_h

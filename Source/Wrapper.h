#ifndef Wrapper_h
#define Wrapper_h

//#include "../JuceLibraryCode/JuceHeader.h"

class Wrapper {
public:
  Wrapper();
  ~Wrapper();

  int fuu();
  void frobb(float *f, int len);
private:
  void init();
};

#endif // Wrapper_h
#include <iostream>
//#include <string>

//#include <JuceHeader.h>

//#include "Stg.h"
//#include "HsFFI.h"
//#include "rts/Time.h"

#include "Rts.h"
#include "Wrapper.h"
#include "Foo_stub.h"

static int initted = 0;

void assert(bool b, const char *s) {
  if (!b) {
    std::cerr << "Error: " << s << "\n";
    exit(1);
  }
}

Wrapper::Wrapper() {
  init();
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
}

void Wrapper::init() {
  if (initted) {
    return;
  }

  RtsConfig conf = defaultRtsConfig;
  conf.rts_opts_enabled = RtsOptsAll;

  int argc = 4;
  char* argv[] = { (char*)"main", (char*)"+RTS", (char*)"-H134217728", (char*)"-RTS" };
  //int argc = 1;
  //char* argv[] = { "main" };
  char** argv_ = argv;

  //juce::Logger::getCurrentLogger()->writeToLog("hs_init_ghc()");
  hs_init_ghc(&argc, &argv_, conf);
  //juce::Logger::getCurrentLogger()->writeToLog("hs_init_ghc() done");

  //hs_add_root(__stginit_Looper);

  initted = 1;
}

int Wrapper::fuu() {
  return foo(12, 23);
}

void Wrapper::frobb(float *f, int len) {
  hs_frobb(f, len);
}

void Wrapper::frobbMidi(Midi *midis, int count) {
  //std::cout << "midis " << sizeof(midis) << "\n";
  assert (sizeof (midis) == 8, "m");
  assert (sizeof (count) == 4, "c");

  assert (sizeof (*midis) == 8, "*m");
  Midi m;
  assert (sizeof (m) == 8, ".m");
  assert (sizeof (Midi) == 8, ".M");
  //std::cout << "bool " << sizeof(bool) << "\n";
  //std::cout << "Midi " << (&m - &m) << " " << (((char*)&(m.isOn)) - ((char*)&m)) << " " << (((char*)&(m.noteNumber)) - ((char*)&m)) << "\n";
  assert ((((char*)&(m.noteNumber)) - ((char*)&m)) == 4, "align");
  assert (sizeof (midis->isOn) == 1, "o");
  assert (sizeof (bool) == 1, "b");
  assert (sizeof (midis->noteNumber) == 4, "nn");

  //std::cout << "calling\n";
  hs_frobb_midi(midis, count);
  //std::cout << "called\n";
  /*
  */
}

Wrapper::~Wrapper() {
}

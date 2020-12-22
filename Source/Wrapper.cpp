//#include <JuceHeader.h>

//#include "Stg.h"
//#include "HsFFI.h"
//#include "rts/Time.h"

#include "Rts.h"
#include "Wrapper.h"
#include "Foo_stub.h"

static int initted = 0;

Wrapper::Wrapper() {
  init();
}

void Wrapper::init() {
  if (initted) {
    return;
  }

  RtsConfig conf = defaultRtsConfig;
  conf.rts_opts_enabled = RtsOptsAll;

  int argc = 4;
  char* argv[] = { "main", "+RTS", "-H134217728", "-RTS" };
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


void Wrapper::frobb(float *f) {
  hs_frobb(f);
}

Wrapper::~Wrapper() {
}

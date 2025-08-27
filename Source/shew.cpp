#include "Loopo.h"
#include "shew.h"

#include "DemoUtilities.h"

#define LOGFILE "/tmp/loopo.log"
const bool enableShew = !PROD;
FileLogger *shew_fl = nullptr;

void shew(const String &s) {
  if (enableShew) {
    if (shew_fl == nullptr) {
      shew_fl = new FileLogger(File(LOGFILE), "heyo");
    }
    shew_fl->logMessage(s);
  }
}

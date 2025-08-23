#include "shew.h"

#define LOGFILE "/tmp/loopo.log"
const bool enableShew = true;
FileLogger *shew_fl = nullptr;

void shew(const String &s) {
  if (enableShew) {
    if (shew_fl == nullptr) {
      shew_fl = new FileLogger(File(LOGFILE), "heyo");
    }
    shew_fl->logMessage(s);
  }
}

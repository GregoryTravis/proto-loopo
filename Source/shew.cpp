#include "Loopo.h"
#include "shew.h"

#include "DemoUtilities.h"

#include <array>
#include <atomic>
#include <memory>
#include <vector>
#include <tuple>
#include <iomanip>
#include <functional>
#include <sstream>
#include <functional>
#include <mutex>

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

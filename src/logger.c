#include <pebble.h>
#include "logger.h"

void simple_log(loglevel level, char* msg) {
  AppLogLevel lvl = APP_LOG_LEVEL_ERROR;
  switch (level) {
    case ERROR:
      break;
    case WARNING:
      lvl = APP_LOG_LEVEL_WARNING;
      break;    
    case INFO:
      lvl = APP_LOG_LEVEL_INFO;
      break;    
    case DEBUG:
      lvl = APP_LOG_LEVEL_DEBUG;
      break;
    case DEBUG_VERBOSE:
      lvl = APP_LOG_LEVEL_DEBUG_VERBOSE;
      break;
  }
  APP_LOG(lvl, msg);
}
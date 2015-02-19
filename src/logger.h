#pragma once
#ifndef __LOGGERUTIL__H
#define __LOGGERUTIL__H

// Log Level enum for simplified logging
typedef enum simplified_log_level {
  ERROR,
  WARNING,
  INFO,
  DEBUG,
  DEBUG_VERBOSE
} loglevel; 
  
// Easier to type  - however log location is lost
void simple_log(loglevel level, char* msg);
  
#endif
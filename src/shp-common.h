
#ifndef SHP_COMMON_H
#define SHP_COMMON_H

#include <string.h>
#include <Rinternals.h>

#define SHP_RESET_ERROR() memset(SALastError, 0, 1024)

#define SHP_ERROR(msg, arg)                                                   \
  char actualErrorMessage[1224];                                              \
  strcpy(actualErrorMessage, msg);                                            \
  memcpy(&actualErrorMessage[strlen(msg)], SALastError, strlen(SALastError)); \
  actualErrorMessage[strlen(msg) + strlen(SALastError)] = '\0';               \
  Rf_error(actualErrorMessage, arg)

#endif

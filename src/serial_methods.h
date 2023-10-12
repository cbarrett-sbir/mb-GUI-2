#ifndef MB_SERIAL_METHODS_H_
#define MB_SERIAL_METHODS_H_

#include <string>

#include "blackbody.h"

void readSerial(std::string& buffer, Blackbody& blackbody);
void parseCommand(const std::string& buffer, Blackbody& blackbody);

#endif  // MB_SERIAL_METHODS_H_

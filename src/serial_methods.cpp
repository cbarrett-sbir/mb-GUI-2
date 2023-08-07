#include <Arduino.h>

#include "serial_methods.h"

void readSerial(std::string& buffer, Blackbody& blackbody)
{
	char c = Serial.read();
	if (c == '\n') // if end of line, parse the command
	{
    std::string s(buffer);
    parseCommand(s, blackbody);
    buffer = "";
	}
  else // otherwise, buffer it
	{
		buffer += c;
	}
}

void parseCommand(std::string s, Blackbody& blackbody)
{
	std::string response = s.substr(0, s.find("= "));
	std::string value = s.substr(s.find("= ") + 2, std::string::npos);

	if(response == "T2")
	{
		blackbody.sourcePlateTemp = std::stof(value);
	}
	else if(response == "SR")
	{
		blackbody.status = std::stoi(value);
	}
  else if(response == "DA")
	{
		blackbody.setPoint = std::stof(value);
	}
}

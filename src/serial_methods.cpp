#include <Arduino.h>

#include "serial_methods.h"
#include <sstream>
#include <algorithm>
#include <cctype>

void readSerial(std::string& buffer, Blackbody& blackbody)
{
	int i =0;
	while(Serial5.available())
	{
	char c = Serial5.read();
	if (c == '\n') // if end of line, parse the command
	{
		parseCommand(buffer, blackbody);
		buffer = "";
		return;
	}
  	else // otherwise, buffer it
	{
		buffer += c;
		i++;
		if(i> 10)
			return;
	}
	}
}

void parseCommand(const std::string& s, Blackbody& blackbody)
{
	std::string response = s.substr(0, s.find("= "));
	std::string value = s.substr(s.find("= ") + 2, std::string::npos);

	Serial.println(("*" + response).c_str());
	Serial.println(("*" + value).c_str());

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
	else if(response == "L")
	{
		blackbody.readyWindow = std::stof(value);
	}
	else if(response == "CURRIP")
	{
		value.erase(remove_if(value.begin(), value.end(), isspace), value.end());
		blackbody.address = ip_address(std::stoi(value.substr(0, 3)), std::stoi(value.substr(4, 3)), std::stoi(value.substr(8, 3)), std::stoi(value.substr(12, 3)));
	}
	else if(response == "ED")
	{
		//Serial.println(("*" + value + "*").c_str());
		// extracts first device name from errdev query
		std::istringstream iss(value);
		std::getline(iss, blackbody.errorDevice, ' ');
		Serial.println((std::to_string(int(blackbody.errorDevice[0])) + " " + std::to_string(blackbody.errorDevice.length())).c_str());
	}
	else if(response == "ERS")
	{
		blackbody.errorString = value;
	}
}

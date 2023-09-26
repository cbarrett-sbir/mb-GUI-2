#ifndef BLACKBODY_H_
#define BLACKBODY_H_

#include "ip_address.h"

class Blackbody
{
  public:
  // status: 0 = ready, 16 = busy, 32 = error
  unsigned status = 0;
	float sourcePlateTemp = 0;

  // setPoint tracks the desired temperature of the bb
	float setPoint = 0;
  
	// config settings
	bool ipMode = 1; // 1 = DHCP, 0 = STATIC
	ip_address address = ip_address(192, 168, 200, 161);
  float readyWindow = 0.0;
};

#endif  // BLACKBODY_H_
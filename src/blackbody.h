#ifndef BLACKBODY_H_
#define BLACKBODY_H_

class Blackbody
{
  public:
  // status: 0 = ready, 16 = busy, 32 = error
  unsigned status = 0;
	float sourcePlateTemp = 0;

  // setPoint tracks the desired temperature of the bb
	float setPoint = 0;
};

#endif  // BLACKBODY_H_
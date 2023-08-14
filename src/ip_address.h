#ifndef IP_ADDRESS_H_
#define IP_ADDRESS_H_

#include <string>

class ip_address
{
    public:
    ip_address(unsigned byte0, unsigned byte1, unsigned byte2, unsigned byte3);

    void incrByte(const unsigned byteIndex);
    void decrByte(const unsigned byteIndex);
    const char * getAddress();
    const char * getAddressByte(const unsigned byteIndex);

    private:
    unsigned address[4];
};

#endif // IP_ADDRESS_H_
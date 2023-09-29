#include "ip_address.h"

#include <iomanip>
#include <sstream>
#include <iostream>

ip_address::ip_address(unsigned b0, unsigned b1, unsigned b2, unsigned b3) : address{b0, b1, b2, b3}
{
}

const char * ip_address::getAddress()
{
    std::ostringstream ostr;
    ostr << std::setfill('0') << std::setw(3) << address[0] << '.'
         << std::setfill('0') << std::setw(3) << address[1] << '.'
         << std::setfill('0') << std::setw(3) << address[2] << '.'
         << std::setfill('0') << std::setw(3) << address[3] << "\n";

    return ostr.str().c_str();
}

const char * ip_address::getAddressByte(const unsigned byteIndex)
{
    std::ostringstream ostr;
    ostr << std::setfill('0') << std::setw(3) << address[byteIndex] << std::endl;

    return ostr.str().c_str();
}

void ip_address::incrByte(const unsigned byteIndex)
{
    if (address[byteIndex] < 255)
    {
        address[byteIndex]++;
    }
}

void ip_address::decrByte(const unsigned byteIndex)
{
    if (address[byteIndex] > 0)
    {
        address[byteIndex]--;
    }
}

ip_address &ip_address::operator=(const ip_address &other)
{
    if (this != &other)
    { // Check for self-assignment
        for (unsigned i = 0; i < 4; ++i)
        {
            address[i] = other.address[i];
        }
    }
    return *this;
}

bool ip_address::operator==(const ip_address &other) const
{
    for (unsigned i = 0; i < 4; ++i)
    {
        if (address[i] != other.address[i])
        {
            return false;
        }
    }
    return true;
}

bool ip_address::operator!=(const ip_address &other) const
{
    return !operator==(other);
}
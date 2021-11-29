
#ifndef __TLC5955_TESTER_HPP__
#define __TLC5955_TESTER_HPP__

#include <tlc5955.hpp>


namespace tlc5955 
{



class tlc5955_tester : public Driver
{
public:
    tlc5955_tester() = default;

    bool get_common_reg_at(uint16_t idx, uint8_t &value);
    void print_register(bool dec_format, bool hex_format);
    

};

} // namespace tlc5955 

#endif // __TLC5955_TESTER_HPP__

#include <tlc5955_tester.hpp>
#include <catch2/catch_all.hpp>
#include <iomanip>

namespace tlc5955 {


uint8_t tlc5955_tester::get_common_reg_at(uint16_t idx)
{
    if (idx > m_common_reg_size_bytes)
    {
        std::cout << "Error at tlc5955_tester::get_common_reg_at() - out of bounds! Max is " 
            << +m_common_reg_size_bytes << ", received " << idx << std::endl;
        REQUIRE(false);
        return 0;
    }
    return m_common_byte_register.at(idx);

}

void tlc5955_tester::print_register(bool dec_format, bool hex_format)
{
    std::cout << std::endl;
    int count {0};

    for (auto &byte : m_common_byte_register)
    {
        if (count % 8 == 0)  { std::cout << std::endl; }

        if (dec_format) { std::cout << " " << std::dec << std::setw(3) << +byte; }
        if (hex_format) { std::cout << " 0x" << std::hex << std::setw(2) << std::setfill('0') << +byte; }
        std::cout << "\t" << std::flush;
        count++;
    }
    std::cout << std::endl;
}

tlc5955_tester::data_t::iterator tlc5955_tester::data_begin()
{
    return m_common_byte_register.begin();
}

tlc5955_tester::data_t::iterator tlc5955_tester::data_end()
{
    return m_common_byte_register.end();
}


} // namespace tlc5955 
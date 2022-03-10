// MIT License

// Copyright (c) 2022 Chris Sutton

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
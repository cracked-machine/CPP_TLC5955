// // MIT License

// // Copyright (c) 2022 Chris Sutton

// // Permission is hereby granted, free of charge, to any person obtaining a copy
// // of this software and associated documentation files (the "Software"), to deal
// // in the Software without restriction, including without limitation the rights
// // to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// // copies of the Software, and to permit persons to whom the Software is
// // furnished to do so, subject to the following conditions:

// // The above copyright notice and this permission notice shall be included in all
// // copies or substantial portions of the Software.

// // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// // IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// // FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// // AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// // LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// // SOFTWARE.

// #ifndef __TLC5955_TESTER_HPP__
// #define __TLC5955_TESTER_HPP__

// #include <tlc5955.hpp>



// namespace tlc5955 
// {



// class tlc5955_tester : public Driver
// {
// public:
//     tlc5955_tester() = default;

//     // @brief alias for common register std::array
//     using data_t = std::array<uint8_t, Driver::m_common_reg_size_bytes>;

//     uint8_t get_common_reg_at(uint16_t idx);
//     void print_register(bool dec_format, bool hex_format);
//     data_t::iterator data_begin();
//     data_t::iterator data_end();

// };

// } // namespace tlc5955 

// #endif // __TLC5955_TESTER_HPP__
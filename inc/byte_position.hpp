#include <cstdio>
#include <cstdint>
#include <iostream>
#include <array>
#include <limits>

#include <cstdint>
#ifndef STM32G0B1xx
    #include <stdexcept>
    #define STDOUT_DEBUG
#endif


// @brief Manages byte count and bit position index. Bit position is reset everytime the byte position is incremented.
class BytePosition
{
public:

    // @brief Construct a new Byte Position object.
    // @param max_bit_idx Set the maximum bit index.
    BytePosition(uint16_t init_byte_pos, uint16_t init_bit_idx) 
        :   m_byte_position(init_byte_pos), 
            m_max_bit_idx (init_bit_idx)
    {
    };

    // @brief pre-increment the byte position. Implicitly resets bit position to max_bit_idx.
    // @return uint16_t
    uint16_t operator++()
    {
        m_bit_position = m_max_bit_idx;
        return m_byte_position++;
    }

    // @brief post-increment the byte position. Implicitly resets bit position to max_bit_idx.
    // @return uint16_t& 
    uint16_t& operator++(const int)
    {
        m_bit_position = m_max_bit_idx;
        return ++m_byte_position;
    }

    uint16_t operator( )() const 
    { 
        #ifdef STDOUT_DEBUG
            std::cout << "Byte:" << m_byte_position << " Bit:" << m_bit_position << std::endl;
        #endif
        return m_byte_position;
    }    
  
    // @brief check if m_bit_position is at zero
    // @return true if m_bit_position is non-zero, false if zero
    bool has_next() {
        if (m_bit_position == std::numeric_limits<uint16_t>::max())
        {
            return false;
        }
        return true;
    }

    // @brief return the decremented bit position. 
    // @param post if true post decrements the bit position, if false pre decrements the bit position.
    // Throws exception if m_bit_position == 0 on x86, returns zero if m_bit_position == 0 on arm.
    // @return uint16_t The bit position value. 
    uint16_t next_bit()
    {     
        auto safe_decrement_check = [](auto &bit_pos) {
            // for post-decrement check if we already wrapped around to 65535 last time, 
            if (bit_pos == std::numeric_limits<uint16_t>::max())
            {
                #ifndef STM32G0B1xx
                    throw std::logic_error("Decrement below zero without call to BytePosition::operator++()");
                #endif
                return false;
            }
            return true;

        };   
 
        return safe_decrement_check(m_bit_position) ? m_bit_position-- : m_bit_position;
 
    }

private:
    
    uint16_t m_byte_position {0};
    uint16_t m_max_bit_idx {6};
    uint16_t m_bit_position {m_max_bit_idx};

};
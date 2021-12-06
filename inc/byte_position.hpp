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

    // @brief Construct a new Byte Position object
    // @param init_byte_pos The byte counter index
    // @param init_bit_idx The max bit position within the byte.
    BytePosition(const uint16_t init_byte_pos, const uint16_t init_bit_idx = 8) 
        :   m_byte_position(init_byte_pos), 
            m_max_bit_idx (init_bit_idx)
    {
    };

    // @brief pre-increment the byte position. Implicitly resets bit position to max_bit_idx.
    // @return uint16_t
    uint16_t operator++()
    {
        //m_bit_position = m_max_bit_idx;
        //return m_byte_position++;
        return m_byte_position;
    }

    // @brief post-increment the byte position. Implicitly resets bit position to max_bit_idx.
    // @return uint16_t& 
    uint16_t& operator++(const int)
    {
        //m_bit_position = m_max_bit_idx;
        //return ++m_byte_position;
        return m_byte_position;
    }
  
    void set_byte_idx(const uint16_t position)
    {
        m_byte_position = position;
    }

    uint16_t get_byte_idx() const 
    { 
        return m_byte_position;
    }    



    // @brief check if we reach end of byte without decrementing the bit position.
    // @return true if there are more bits in this byte. false if end of byte is reached.
    bool has_next() {     
        if (m_bit_position == std::numeric_limits<uint16_t>::max())
        {
            return false;
        }
        return true;
    }

    // @brief get the bit index and post-decrement it. 
    // Throws exception (x86) or returns zero (Arm) if next bit pos == std::numeric_limits<uint16_t>::max()
    // @return uint16_t The bit position value. 
    uint16_t next_bit_idx()
    {     

        if (m_bit_position == 0)
        {
            m_bit_position = m_max_bit_idx;
            m_bit_position--;
            m_byte_position++;
        }
        else
        {
            m_bit_position--;
        }
        
        return m_bit_position;

        // if (m_bit_position == std::numeric_limits<uint16_t>::max())
        // {
        //     // reset the bit position back to the max value
        //     m_bit_position = m_max_bit_idx;
        //     ++m_byte_position;
        //     --m_bit_position;
        // }
        
        // #ifdef STDOUT_DEBUG
        //     std::cout << "Byte:" << m_byte_position << " Bit:" << m_bit_position << std::endl;
        // #endif      
        

        // return m_bit_position;
    }

    // @brief Convenience function. Calls next_bit_idx() function N times.
    // @param n The number of bits to skip forward 
    // @return true if next_bit_idx() is successful, false if not
    bool skip_next_n_bits(uint8_t n)
    {
        for (uint8_t idx = 0; idx < n; idx++)
        {
            if (!next_bit_idx()) { return false; }
        }
        return true;
    }

private:
    
    uint16_t m_byte_position {0};
    uint16_t m_max_bit_idx {0};
    uint16_t m_bit_position {m_max_bit_idx};

};
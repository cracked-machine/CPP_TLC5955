
#include "tlc5955.hpp"
#include <sstream>
#include <cmath>
#include <cstring>
#include <byte_position.hpp>

#include <bitset_utils.hpp>

#ifdef USE_RTT
    #include <SEGGER_RTT.h>
#endif
namespace tlc5955 
{

bool Driver::start_dma_transmit()
{
    #ifdef USE_HAL_DRIVER 
        return HAL_SPI_Transmit_DMA(&m_spi_interface, m_common_byte_register.data(), m_common_byte_register.size());      
    #else
        // we don't care about SPI for x86-based unit testing
        return true;
    #endif
}

// @brief class to implement TLC5955 LED Driver IC
// Refer to datasheet - https://www.ti.com/lit/ds/symlink/tlc5955.pdf
void Driver::reset()
{
    m_common_bit_register.reset();
    m_common_byte_register.fill(0);
}

void Driver::set_latch_cmd(bool latch)
{
    std::bitset<m_latch_size_bits> latch_cmd {latch};
    bitsetter.add_bitset(m_common_bit_register, latch_cmd, m_latch_offset);

    // if the latch is 1 then we also need to add the ctrl command and the padding
    // See "8.3.2 Register and Data Latch Configuration" of the datasheet
    if (latch)
    {
        bitsetter.add_bitset(m_common_bit_register, m_ctrl_cmd, m_ctrl_cmd_offset);
        bitsetter.add_bitset(m_common_bit_register, m_padding, m_padding_offset);
    }
}

void Driver::set_function_cmd(const bool dsprpt, const bool tmgrst, const bool rfresh, const bool espwm, const bool lsdvlt)
{
    std::bitset<m_func_data_section_size_bits> function_cmd {};
    function_cmd.set(0, dsprpt);
    function_cmd.set(1, tmgrst);
    function_cmd.set(2, rfresh);
    function_cmd.set(3, espwm);
    function_cmd.set(4, lsdvlt);
    bitsetter.add_bitset(m_common_bit_register, function_cmd, m_func_data_offset);
}

void Driver::set_global_brightness_cmd(const uint8_t blue, const uint8_t green, const uint8_t red)
{
    const std::bitset<m_bc_data_resolution> blue_cmd {blue};
    const std::bitset<m_bc_data_resolution> green_cmd {green};
    const std::bitset<m_bc_data_resolution> red_cmd {red};
    
    bitsetter.add_bitset(m_common_bit_register, blue_cmd, m_bc_data_offset);
    bitsetter.add_bitset(m_common_bit_register, green_cmd, m_bc_data_offset + m_bc_data_resolution);
    bitsetter.add_bitset(m_common_bit_register, red_cmd, m_bc_data_offset + m_bc_data_resolution * 2);
}

void Driver::set_max_current_cmd(const uint8_t blue, const uint8_t green, const uint8_t red)
{
    const std::bitset<m_mc_data_resolution> blue_cmd {blue};
    const std::bitset<m_mc_data_resolution> green_cmd {green};
    const std::bitset<m_mc_data_resolution> red_cmd {red};

    bitsetter.add_bitset(m_common_bit_register, blue_cmd, m_mc_data_offset);
    bitsetter.add_bitset(m_common_bit_register, green_cmd, m_mc_data_offset + m_mc_data_resolution);
    bitsetter.add_bitset(m_common_bit_register, red_cmd, m_mc_data_offset + m_mc_data_resolution * 2);
}

void Driver::set_dot_correction_cmd_all(uint8_t pwm)
{
    const std::bitset<m_dc_data_resolution> dc_pwm_cmd {pwm};
    for (uint8_t dc_idx = 0; dc_idx < 48; dc_idx++)
	{
		bitsetter.add_bitset(m_common_bit_register, dc_pwm_cmd, m_dc_data_offset + m_dc_data_resolution * dc_idx);		
	}
}

void Driver::set_greyscale_cmd_all(uint16_t pwm)
{
    const std::bitset<m_gs_data_resolution> gs_pwm_cmd {pwm}; 
    for (uint16_t gs_idx = 0; gs_idx < 48; gs_idx++)
    {
    	bitsetter.add_bitset(m_common_bit_register, gs_pwm_cmd, m_gs_data_offset + m_gs_data_resolution * gs_idx);
    }    
}

void Driver::process_register()
{
    bitsetter.bitset_to_bytearray(m_common_byte_register, m_common_bit_register);
}




} // namespace tlc5955
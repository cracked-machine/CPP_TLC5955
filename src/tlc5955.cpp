
#include "tlc5955.hpp"
#include <sstream>
#include <cmath>
#include <cstring>


#include <bitset_utils.hpp>

#ifdef USE_RTT
    #include <SEGGER_RTT.h>
#endif


#if defined(USE_SSD1306_LL_DRIVER)
	#include <ll_spi_utils.hpp>
#endif

namespace tlc5955 
{

bool Driver::enable_spi(dma use_dma)
{
    if(use_dma == dma::enable)
    {
        #ifdef USE_TLC5955_HAL_DRIVER 
            return HAL_SPI_Transmit_DMA(&m_spi_port, m_common_byte_register.data(), m_common_byte_register.size());  
        #elif USE_TLC5955_LL_DRIVER
            return true;
        #else
            // we don't care about SPI for x86-based unit testing
            return true;
        #endif
    }
    else
    {
        #if USE_TLC5955_LL_DRIVER
            LL_SPI_Enable(m_spi_port);
            // set the max freq (16MHz) for PWM when TIM4_ARR is also 1.
            LL_TIM_OC_SetCompareCH1(TIM4, 0x1);
            // enable the timer but not the output
            LL_TIM_EnableCounter(TIM4);    
            
            return LL_SPI_IsEnabled(m_spi_port) && LL_TIM_IsEnabledCounter(TIM4);
        #endif
    }
    return false;

}

bool Driver::send_blocking_transmit()
{
    #ifdef USE_TLC5955_HAL_DRIVER 
        if (IS_SPI_DMA_HANDLE(hspi->hdmatx))
        {
            return false;
        }
        else
        {
            return HAL_SPI_Transmit(&m_spi_port, m_common_byte_register.data(), m_common_byte_register.size(), 0);
        }
    #elif USE_TLC5955_LL_DRIVER
        // we don't want to be here if DMA is enabled
        if (LL_SPI_IsEnabledDMAReq_TX(m_spi_port)) { return false; }
        
        // resume the GSCLK - enable the timer output
        LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH1);
        
        // send the bytes
        for (auto &byte: m_common_byte_register)
        {
            LL_SPI_TransmitData8(m_spi_port, byte);
        }     

        // check the data was all clocked into the IC before latching
        embedded_utils::LowLevelSPIUtils spi_utils;
        if (!spi_utils.check_txe_flag_status(m_spi_port))
        {
            #if defined(USE_RTT) 
                SEGGER_RTT_printf(0, "tlc5955::Driver::send_blocking_transmit(): Tx buffer is full"); 
            #endif
        }
        if (!spi_utils.check_bsy_flag_status(m_spi_port))
        {
            #if defined(USE_RTT) 
                SEGGER_RTT_printf(0, "tlc5955::Driver::send_blocking_transmit(); SPI bus is busy"); 
            #endif
        }  

        // pause the GSCLK - disable the timer output
        LL_TIM_CC_DisableChannel(TIM4, LL_TIM_CHANNEL_CH1);

        // toggle the pin to latch the register contents
        LL_GPIO_SetOutputPin(m_lat_port, m_lat_pin);
        LL_GPIO_ResetOutputPin(m_lat_port, m_lat_pin);
        return true;            
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
    std::bitset<m_select_cmd_size> latch_cmd {latch};
    bitsetter.add_bitset(m_common_bit_register, latch_cmd, m_select_cmd_offset);

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
    std::bitset<m_func_cmd_size> function_cmd {};
    function_cmd.set(0, dsprpt);
    function_cmd.set(1, tmgrst);
    function_cmd.set(2, rfresh);
    function_cmd.set(3, espwm);
    function_cmd.set(4, lsdvlt);
    bitsetter.add_bitset(m_common_bit_register, function_cmd, m_func_cmd_offset);
}

void Driver::set_global_brightness_cmd(const uint8_t blue, const uint8_t green, const uint8_t red)
{
    const std::bitset<m_bc_data_size> blue_cmd {blue};
    const std::bitset<m_bc_data_size> green_cmd {green};
    const std::bitset<m_bc_data_size> red_cmd {red};
    
    bitsetter.add_bitset(m_common_bit_register, blue_cmd, m_bc_data_offset);
    bitsetter.add_bitset(m_common_bit_register, green_cmd, m_bc_data_offset + m_bc_data_size);
    bitsetter.add_bitset(m_common_bit_register, red_cmd, m_bc_data_offset + m_bc_data_size * 2);
}

void Driver::set_max_current_cmd(const uint8_t blue, const uint8_t green, const uint8_t red)
{
    const std::bitset<m_mc_data_size> blue_cmd {blue};
    const std::bitset<m_mc_data_size> green_cmd {green};
    const std::bitset<m_mc_data_size> red_cmd {red};

    bitsetter.add_bitset(m_common_bit_register, blue_cmd, m_mc_data_offset);
    bitsetter.add_bitset(m_common_bit_register, green_cmd, m_mc_data_offset + m_mc_data_size);
    bitsetter.add_bitset(m_common_bit_register, red_cmd, m_mc_data_offset + m_mc_data_size * 2);
}

void Driver::set_dot_correction_cmd_all(uint8_t pwm)
{
    const std::bitset<m_dc_data_size> dc_pwm_cmd {pwm};
    for (uint8_t dc_idx = 0; dc_idx < 48; dc_idx++)
	{
		bitsetter.add_bitset(m_common_bit_register, dc_pwm_cmd, m_dc_data_offset + m_dc_data_size * dc_idx);		
	}
}

void Driver::set_greyscale_cmd_rgb(uint16_t blue_pwm, uint16_t green_pwm, uint16_t red_pwm)
{
    const std::bitset<m_gs_data_size> blue_gs_pwm_cmd {blue_pwm}; 
    const std::bitset<m_gs_data_size> green_gs_pwm_cmd {green_pwm}; 
    const std::bitset<m_gs_data_size> red_gs_pwm_cmd {red_pwm}; 
    for (uint16_t gs_idx = 0; gs_idx < m_num_leds_per_chip; gs_idx++)
    {
    	bitsetter.add_bitset(m_common_bit_register, blue_gs_pwm_cmd, m_gs_data_offset   + (m_gs_data_size * gs_idx * m_num_colour_chan));
        bitsetter.add_bitset(m_common_bit_register, green_gs_pwm_cmd, m_gs_data_offset   + (m_gs_data_size * gs_idx * m_num_colour_chan) + m_gs_data_size);
        bitsetter.add_bitset(m_common_bit_register, red_gs_pwm_cmd, m_gs_data_offset   + (m_gs_data_size * gs_idx * m_num_colour_chan) + (m_gs_data_size * 2));
    }    
}

void Driver::set_greyscale_cmd_all(uint16_t pwm)
{
    const std::bitset<m_gs_data_size> gs_pwm_cmd {pwm}; 
    for (uint16_t gs_idx = 0; gs_idx < 48; gs_idx++)
    {
    	bitsetter.add_bitset(m_common_bit_register, gs_pwm_cmd, m_gs_data_offset + m_gs_data_size * gs_idx);
    }    
}

void Driver::process_register()
{
    bitsetter.print_bits(m_common_bit_register);
    bitsetter.bitset_to_bytearray(m_common_byte_register, m_common_bit_register);
}




} // namespace tlc5955
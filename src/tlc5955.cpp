
#include "tlc5955.hpp"

#include <sstream>
#include <cmath>
#include <cstring>


#if defined(USE_RTT)
    #include <SEGGER_RTT.h>
#endif

#if defined(USE_FULL_LL_DRIVER)
    #include <ll_spi_utils.hpp>
#endif
namespace tlc5955 
{



// bool Driver::enable_spi(dma use_dma)
// {
//     if(use_dma == dma::enable)
//     {
//         #if defined(USE_TLC5955_HAL_DRIVER)
//             return HAL_SPI_Transmit_DMA(&m_spi_port, m_common_byte_register.data(), m_common_byte_register.size());  
//         #elif defined(USE_TLC5955_LL_DRIVER)
//             return true;
//         #else
//             // we don't care about SPI for x86-based unit testing
//             return true;
//         #endif
//     }
//     else
//     {
//         #if defined(USE_TLC5955_LL_DRIVER)
//             spi2_init();
//             LL_SPI_Enable(m_spi_port);
            
//             // return LL_SPI_IsEnabled(m_spi_port) && LL_TIM_IsEnabledCounter(TIM4);
//             return true;
//         #endif
//     }
//     return false;

// }


// @brief class to implement TLC5955 LED Driver IC
// Refer to datasheet - https://www.ti.com/lit/ds/symlink/tlc5955.pdf
void Driver::reset()
{
    m_common_bit_register.reset();
    m_common_byte_register.fill(0);
}

void Driver::toggle_latch(bool latch [[maybe_unused]])
{
    //std::bitset<m_select_cmd_size> latch_cmd {latch};
    //embed_utils::bit_manip::add_bitset(m_common_bit_register, latch_cmd, m_select_cmd_offset);
    #if defined(USE_FULL_LL_DRIVER)
        LL_SPI_Disable(SPI2);

        // set PB7/PB8 as GPIO outputs
        gpio_init();
        
        // if the latch is 1 then we also need to add the ctrl command and the padding
        // See "8.3.2 Register and Data Latch Configuration" of the datasheet
        if (latch)
        {
            // reset both SCK and MOSI
            LL_GPIO_ResetOutputPin(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_MOSI_Pin); 
            LL_GPIO_ResetOutputPin(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_SCK_Pin); 

            // MOSI data clocked on high(1) rising edge of SCK
            LL_GPIO_SetOutputPin(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_MOSI_Pin); 
            LL_GPIO_SetOutputPin(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_SCK_Pin); 
            

            // reset both SCK and MOSI
            LL_GPIO_ResetOutputPin(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_MOSI_Pin); 
            LL_GPIO_ResetOutputPin(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_SCK_Pin); 
            

        }
        else
        {
            // reset both SCK and MOSI
            LL_GPIO_ResetOutputPin(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_SCK_Pin); 
            LL_GPIO_ResetOutputPin(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_MOSI_Pin); 
            
            // MOSI data clocked low(0) on rising edge of SCK
            LL_GPIO_SetOutputPin(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_SCK_Pin); 
            LL_GPIO_ResetOutputPin(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_MOSI_Pin); 

            // reset both SCK and MOSI
            LL_GPIO_ResetOutputPin(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_SCK_Pin); 
            LL_GPIO_ResetOutputPin(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_MOSI_Pin);    
        }

        // set PB7/PB8 to SPI
        spi2_init();
        LL_SPI_Enable(m_spi_port);
        
    #endif  // USE_FULL_LL_DRIVER
}

bool Driver::send_blocking_transmit()
{
    #if defined(USE_TLC5955_HAL_DRIVER)
        if (IS_SPI_DMA_HANDLE(hspi->hdmatx))
        {
            return false;
        }
        else
        {
            return HAL_SPI_Transmit(&m_spi_port, m_common_byte_register.data(), m_common_byte_register.size(), 0);
        }
    #elif defined(USE_TLC5955_LL_DRIVER)
        // we don't want to be here if DMA is enabled
        if (LL_SPI_IsEnabledDMAReq_TX(m_spi_port)) { return false; }
            
        // resume the GSCLK - enable the timer output
        LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH1);

        // send the bytes
        for (auto &byte: m_common_byte_register)
        {
            // send the byte of data
            LL_SPI_TransmitData8(m_spi_port, byte);

            // check the data has left the SPI FIFO before sending the next
            if (!embed_utils::spi::ll_wait_for_txe_flag(m_spi_port, 1))
            {
                #if defined(USE_RTT) 
                    SEGGER_RTT_printf(0, "tlc5955::Driver::send_blocking_transmit(): Tx buffer is full"); 
                #endif
            }
            if (!embed_utils::spi::ll_wait_for_bsy_flag(m_spi_port, 1))
            {
                #if defined(USE_RTT) 
                    SEGGER_RTT_printf(0, "tlc5955::Driver::send_blocking_transmit(); SPI bus is busy"); 
                #endif
            }   
        }     

        // pause the GSCLK - disable the timer output
        LL_TIM_CC_DisableChannel(TIM4, LL_TIM_CHANNEL_CH1);

        // signal end of transmission/latch common register
        LL_GPIO_SetOutputPin(m_lat_port, m_lat_pin);
        LL_GPIO_ResetOutputPin(m_lat_port, m_lat_pin);        
        
        return true;            
    #else
        // we don't care about SPI for x86-based unit testing
        return true;
    #endif
}

void Driver::set_ctrl_cmd()
{
    embed_utils::bit_manip::add_bitset(m_common_bit_register, m_ctrl_cmd, m_ctrl_cmd_offset);    
}

void Driver::set_padding_bits()
{
    embed_utils::bit_manip::add_bitset(m_common_bit_register, m_padding, m_padding_offset);
}

void Driver::set_function_cmd(const bool dsprpt, const bool tmgrst, const bool rfresh, const bool espwm, const bool lsdvlt)
{
    std::bitset<m_func_cmd_size> function_cmd {};
    function_cmd.set(4, dsprpt);
    function_cmd.set(3, tmgrst);
    function_cmd.set(2, rfresh);
    function_cmd.set(1, espwm);
    function_cmd.set(0, lsdvlt);
    embed_utils::bit_manip::add_bitset(m_common_bit_register, function_cmd, m_func_cmd_offset);
}

void Driver::set_global_brightness_cmd(const uint8_t blue, const uint8_t green, const uint8_t red)
{
    const std::bitset<m_bc_data_size> blue_cmd {blue};
    const std::bitset<m_bc_data_size> green_cmd {green};
    const std::bitset<m_bc_data_size> red_cmd {red};
    
    embed_utils::bit_manip::add_bitset(m_common_bit_register, blue_cmd, m_bc_data_offset);
    embed_utils::bit_manip::add_bitset(m_common_bit_register, green_cmd, m_bc_data_offset + m_bc_data_size);
    embed_utils::bit_manip::add_bitset(m_common_bit_register, red_cmd, m_bc_data_offset + m_bc_data_size * 2);
}

void Driver::set_max_current_cmd(const uint8_t blue, const uint8_t green, const uint8_t red)
{
    const std::bitset<m_mc_data_size> blue_cmd {blue};
    const std::bitset<m_mc_data_size> green_cmd {green};
    const std::bitset<m_mc_data_size> red_cmd {red};

    embed_utils::bit_manip::add_bitset(m_common_bit_register, blue_cmd, m_mc_data_offset);
    embed_utils::bit_manip::add_bitset(m_common_bit_register, green_cmd, m_mc_data_offset + m_mc_data_size);
    embed_utils::bit_manip::add_bitset(m_common_bit_register, red_cmd, m_mc_data_offset + m_mc_data_size * 2);
}

void Driver::set_dot_correction_cmd_all(uint8_t pwm)
{
    const std::bitset<m_dc_data_size> dc_pwm_cmd {pwm};
    for (uint8_t dc_idx = 0; dc_idx < 48; dc_idx++)
	{
		embed_utils::bit_manip::add_bitset(m_common_bit_register, dc_pwm_cmd, m_dc_data_offset + m_dc_data_size * dc_idx);		
	}
}

void Driver::set_greyscale_cmd_rgb(uint16_t blue_pwm, uint16_t green_pwm, uint16_t red_pwm)
{
    const std::bitset<m_gs_data_size> blue_gs_pwm_cmd {blue_pwm}; 
    const std::bitset<m_gs_data_size> green_gs_pwm_cmd {green_pwm}; 
    const std::bitset<m_gs_data_size> red_gs_pwm_cmd {red_pwm}; 
    for (uint16_t gs_idx = 0; gs_idx < m_num_leds_per_chip; gs_idx++)
    {
    	embed_utils::bit_manip::add_bitset(m_common_bit_register, blue_gs_pwm_cmd, m_gs_data_offset   + (m_gs_data_size * gs_idx * m_num_colour_chan));
        embed_utils::bit_manip::add_bitset(m_common_bit_register, green_gs_pwm_cmd, m_gs_data_offset   + (m_gs_data_size * gs_idx * m_num_colour_chan) + m_gs_data_size);
        embed_utils::bit_manip::add_bitset(m_common_bit_register, red_gs_pwm_cmd, m_gs_data_offset   + (m_gs_data_size * gs_idx * m_num_colour_chan) + (m_gs_data_size * 2));
    }    
}

void Driver::set_greyscale_cmd_all(uint16_t pwm)
{
    const std::bitset<m_gs_data_size> gs_pwm_cmd {pwm}; 
    for (uint16_t gs_idx = 0; gs_idx < 48; gs_idx++)
    {
    	embed_utils::bit_manip::add_bitset(m_common_bit_register, gs_pwm_cmd, m_gs_data_offset + m_gs_data_size * gs_idx);
    }    
}

void Driver::process_register()
{
    embed_utils::bit_manip::print_bits(m_common_bit_register);
    embed_utils::bit_manip::bitset_to_bytearray(m_common_byte_register, m_common_bit_register);
    embed_utils::byte_manip::print_bytes(m_common_byte_register);
}

void Driver::gpio_init(void)
{
    #if defined(USE_TLC5955_LL_DRIVER)
        LL_GPIO_InitTypeDef GPIO_InitStruct = {0,0,0,0,0,0};
        LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
        
        // TLC5955_SPI2_MOSI
        //LL_GPIO_SetOutputPin(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_MOSI_Pin); 
        GPIO_InitStruct.Pin = TLC5955_SPI2_MOSI_Pin;
        GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
        GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
        LL_GPIO_Init(TLC5955_SPI2_MOSI_GPIO_Port, &GPIO_InitStruct);

        // TLC5955_SPI2_SCK
        // LL_GPIO_ResetOutputPin(TLC5955_SPI2_SCK_GPIO_Port, TLC5955_SPI2_SCK_Pin);
        GPIO_InitStruct.Pin = TLC5955_SPI2_SCK_Pin;
        GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
        GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
        LL_GPIO_Init(TLC5955_SPI2_SCK_GPIO_Port, &GPIO_InitStruct);    

        LL_SYSCFG_EnableFastModePlus(LL_SYSCFG_I2C_FASTMODEPLUS_PB7);
        LL_SYSCFG_EnableFastModePlus(LL_SYSCFG_I2C_FASTMODEPLUS_PB8);
    #endif // USE_TLC5955_LL_DRIVER
}
void Driver::spi2_init(void)
{
    #if defined(USE_TLC5955_LL_DRIVER)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wvolatile" 

        /* Peripheral clock enable */;
        SET_BIT(RCC->APBENR1, LL_APB1_GRP1_PERIPH_SPI2);
        //LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);

        // Enable GPIO PB7 (SPI2_MOSI)
        LL_GPIO_SetPinSpeed(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_MOSI_Pin, LL_GPIO_SPEED_FREQ_VERY_HIGH);
        LL_GPIO_SetPinOutputType(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_MOSI_Pin, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinPull(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_MOSI_Pin, LL_GPIO_PULL_DOWN);
        LL_GPIO_SetAFPin_0_7(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_MOSI_Pin, LL_GPIO_AF_1);
        LL_GPIO_SetPinMode(TLC5955_SPI2_MOSI_GPIO_Port, TLC5955_SPI2_MOSI_Pin, LL_GPIO_MODE_ALTERNATE);

        // Enable GPIO PB8 (SPI2_SCK)
        LL_GPIO_SetPinSpeed(TLC5955_SPI2_SCK_GPIO_Port, TLC5955_SPI2_SCK_Pin, LL_GPIO_SPEED_FREQ_VERY_HIGH);
        LL_GPIO_SetPinOutputType(TLC5955_SPI2_SCK_GPIO_Port, TLC5955_SPI2_SCK_Pin, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinPull(TLC5955_SPI2_SCK_GPIO_Port, TLC5955_SPI2_SCK_Pin, LL_GPIO_PULL_DOWN);
        LL_GPIO_SetAFPin_8_15(TLC5955_SPI2_SCK_GPIO_Port, TLC5955_SPI2_SCK_Pin, LL_GPIO_AF_1);
        LL_GPIO_SetPinMode(TLC5955_SPI2_SCK_GPIO_Port, TLC5955_SPI2_SCK_Pin, LL_GPIO_MODE_ALTERNATE);        

        SET_BIT(SYSCFG->CFGR1, LL_SYSCFG_I2C_FASTMODEPLUS_PB7);
        SET_BIT(SYSCFG->CFGR1, LL_SYSCFG_I2C_FASTMODEPLUS_PB8);

        SPI2->CR1 = 0;
        SPI2->CR1 |=    (LL_SPI_HALF_DUPLEX_TX | LL_SPI_MODE_MASTER | LL_SPI_POLARITY_LOW | LL_SPI_PHASE_1EDGE | 
                        LL_SPI_NSS_SOFT | LL_SPI_BAUDRATEPRESCALER_DIV8 | LL_SPI_MSB_FIRST | LL_SPI_CRCCALCULATION_DISABLE);

        MODIFY_REG(SPI2->CR2, SPI_CR2_FRF, LL_SPI_PROTOCOL_MOTOROLA);

        CLEAR_BIT(SPI2->CR2, SPI_CR2_NSSP);

    #pragma GCC diagnostic pop  // ignored "-Wvolatile"  
    #endif // defined(USE_TLC5955_LL_DRIVER)

}


} // namespace tlc5955
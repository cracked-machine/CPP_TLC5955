
#include "tlc5955.hpp"

#include <sstream>
#include <cmath>
#include <cstring>
#include <cassert>

#if defined(USE_RTT)
    #include <SEGGER_RTT.h>
#endif

#if defined(USE_FULL_LL_DRIVER)
    #include <ll_spi_utils.hpp>
#endif
namespace tlc5955 
{

Driver::Driver(SPI_TypeDef * spi_handle)
{
    // initialise the SPI handle used in this class
    _spi_handle = std::unique_ptr<SPI_TypeDef>(spi_handle);
}

// @brief class to implement TLC5955 LED Driver IC
// Refer to datasheet - https://www.ti.com/lit/ds/symlink/tlc5955.pdf
void Driver::reset()
{
    m_common_bit_register.reset();
    m_common_byte_register.fill(0);
}

void Driver::send_first_bit(DataLatchType latch_type [[maybe_unused]])
{

    #if defined(USE_FULL_LL_DRIVER)

        LL_SPI_Disable(_spi_handle.get());

        // set PB7/PB8 as GPIO outputs
        gpio_init();

        // make sure LAT pin is low otherwise first latch may be skipped (and TLC5955 will initialise intermittently)
        LL_GPIO_ResetOutputPin(m_lat_port, m_lat_pin);   

        // "Control Data Latch" - Start SPI transacation by clocking in one high bit
        if (latch_type == DataLatchType::control)
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
        // "GS Data Latch" - Start SPI transacation by clocking in one low bit
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
        LL_SPI_Enable(_spi_handle.get());
        
    #endif  // USE_FULL_LL_DRIVER
}

bool Driver::send_spi_bytes(LatchPinOption latch_option)
{
    #if defined(USE_TLC5955_HAL_DRIVER)
        if (IS_SPI_DMA_HANDLE(hspi->hdmatx))
        {
            return false;
        }
        else
        {
            return HAL_SPI_Transmit(&_spi_handle.get(), m_common_byte_register.data(), m_common_byte_register.size(), 0);
        }
    #elif defined(USE_TLC5955_LL_DRIVER)
        // we don't want to be here if DMA is enabled
        if (LL_SPI_IsEnabledDMAReq_TX(_spi_handle.get())) { return false; }
            
        // send the bytes
        for (auto &byte: m_common_byte_register)
        {
            // send the byte of data
            LL_SPI_TransmitData8(_spi_handle.get(), byte);

            // check the data has left the SPI FIFO before sending the next
            if (!stm32::spi::ll_wait_for_txe_flag(_spi_handle, 1))
            {
                #if defined(USE_RTT) 
                    SEGGER_RTT_printf(0, "tlc5955::Driver::send_blocking_transmit(): Tx buffer is full"); 
                #endif
            }
            if (!stm32::spi::ll_wait_for_bsy_flag(_spi_handle, 1))
            {
                #if defined(USE_RTT) 
                    SEGGER_RTT_printf(0, "tlc5955::Driver::send_blocking_transmit(); SPI bus is busy"); 
                #endif
            }   
        }     

        // tell each daisy-chained driver chip to latch all data from its common register
        if (latch_option == LatchPinOption::latch_after_send)
        {
            LL_GPIO_SetOutputPin(m_lat_port, m_lat_pin);
            LL_GPIO_ResetOutputPin(m_lat_port, m_lat_pin);        
        }        
        return true;            
    #else
        // we don't care about SPI for x86-based unit testing
        return true;
    #endif
}

void Driver::set_ctrl_cmd()
{
    noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, m_ctrl_cmd, m_ctrl_cmd_offset);    
}

void Driver::set_padding_bits()
{
    noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, m_padding, m_padding_offset);
}

void Driver::set_function_cmd(DisplayFunction dsprpt, TimingFunction tmgrst, RefreshFunction rfresh, PwmFunction espwm, ShortDetectFunction lsdvlt)
{
    std::bitset<m_func_cmd_size> function_cmd {};
    (dsprpt == DisplayFunction::display_repeat_on)          ? function_cmd.set(4, true) : function_cmd.set(4, false);
    (tmgrst == TimingFunction::timing_reset_on)             ? function_cmd.set(3, true) : function_cmd.set(3, false);
    (rfresh == RefreshFunction::auto_refresh_on)            ? function_cmd.set(2, true) : function_cmd.set(2, false);
    (espwm == PwmFunction::enhanced_pwm)                    ? function_cmd.set(1, true) : function_cmd.set(1, false);
    (lsdvlt == ShortDetectFunction::threshold_90_percent)   ? function_cmd.set(0, true) : function_cmd.set(0, false);
    noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, function_cmd, m_func_cmd_offset);    
}

void Driver::set_global_brightness_cmd(const uint8_t blue, const uint8_t green, const uint8_t red)
{
    const std::bitset<m_bc_data_size> blue_cmd {blue};
    const std::bitset<m_bc_data_size> green_cmd {green};
    const std::bitset<m_bc_data_size> red_cmd {red};
    
    noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, blue_cmd, m_bc_data_offset);
    noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, green_cmd, m_bc_data_offset + m_bc_data_size);
    noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, red_cmd, m_bc_data_offset + m_bc_data_size * 2);

}

void Driver::set_max_current_cmd(const uint8_t blue, const uint8_t green, const uint8_t red)
{
    const std::bitset<m_mc_data_size> blue_cmd {blue};
    const std::bitset<m_mc_data_size> green_cmd {green};
    const std::bitset<m_mc_data_size> red_cmd {red};

    noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, blue_cmd, m_mc_data_offset);
    noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, green_cmd, m_mc_data_offset + m_mc_data_size);
    noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, red_cmd, m_mc_data_offset + m_mc_data_size * 2);
}

void Driver::set_dot_correction_cmd_all(uint8_t pwm)
{
    const std::bitset<m_dc_data_size> dc_pwm_cmd {pwm};
    for (uint8_t dc_idx = 0; dc_idx < 48; dc_idx++)
	{
		noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, dc_pwm_cmd, m_dc_data_offset + m_dc_data_size * dc_idx);		
	}
}

void Driver::set_greyscale_cmd_rgb(uint16_t red_pwm, uint16_t green_pwm, uint16_t blue_pwm)
{
    const std::bitset<m_gs_data_size> blue_gs_pwm_cmd {blue_pwm}; 
    const std::bitset<m_gs_data_size> green_gs_pwm_cmd {green_pwm}; 
    const std::bitset<m_gs_data_size> red_gs_pwm_cmd {red_pwm}; 
    for (uint16_t gs_idx = 0; gs_idx < m_num_leds_per_chip; gs_idx++)
    {
    	noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, blue_gs_pwm_cmd, m_gs_data_offset   + (m_gs_data_size * gs_idx * m_num_colour_chan));
        noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, green_gs_pwm_cmd, m_gs_data_offset   + (m_gs_data_size * gs_idx * m_num_colour_chan) + m_gs_data_size);
        noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, red_gs_pwm_cmd, m_gs_data_offset   + (m_gs_data_size * gs_idx * m_num_colour_chan) + (m_gs_data_size * 2));
    }    
}

void Driver::set_greyscale_cmd_white(uint16_t pwm)
{
    const std::bitset<m_gs_data_size> gs_pwm_cmd {pwm}; 
    for (uint16_t gs_idx = 0; gs_idx < 48; gs_idx++)
    {
    	noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, gs_pwm_cmd, m_gs_data_offset + m_gs_data_size * gs_idx);
    }    
}

bool Driver::set_greyscale_cmd_rgb_at_position(uint16_t led_idx, uint16_t red_pwm, uint16_t green_pwm, uint16_t blue_pwm)
{
    // return if we overshot our max number of LEDs
    if (! (led_idx < tlc5955::Driver::m_num_leds_per_chip))
    {
        return false;
    }
    
    const std::bitset<m_gs_data_size> blue_gs_pwm_cmd {blue_pwm}; 
    const std::bitset<m_gs_data_size> green_gs_pwm_cmd {green_pwm}; 
    const std::bitset<m_gs_data_size> red_gs_pwm_cmd {red_pwm}; 

    noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, blue_gs_pwm_cmd, m_gs_data_offset   + (m_gs_data_size * led_idx * m_num_colour_chan));
    noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, green_gs_pwm_cmd, m_gs_data_offset   + (m_gs_data_size * led_idx * m_num_colour_chan) + m_gs_data_size);
    noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, red_gs_pwm_cmd, m_gs_data_offset   + (m_gs_data_size * led_idx * m_num_colour_chan) + (m_gs_data_size * 2));
    return true;
}

void Driver::process_register()
{
    // noarch::bit_manip::print_bits(m_common_bit_register);
    noarch::bit_manip::bitset_to_bytearray(m_common_byte_register, m_common_bit_register);
    // noarch::byte_manip::print_bytes(m_common_byte_register);
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

        _spi_handle.get()->CR1 = 0;
        _spi_handle.get()->CR1 |=    (LL_SPI_HALF_DUPLEX_TX | LL_SPI_MODE_MASTER | LL_SPI_POLARITY_LOW | LL_SPI_PHASE_1EDGE | 
                        LL_SPI_NSS_SOFT | LL_SPI_BAUDRATEPRESCALER_DIV8 | LL_SPI_MSB_FIRST | LL_SPI_CRCCALCULATION_DISABLE);

        MODIFY_REG(_spi_handle.get()->CR2, SPI_CR2_FRF, LL_SPI_PROTOCOL_MOTOROLA);

        CLEAR_BIT(_spi_handle.get()->CR2, SPI_CR2_NSSP);

        // start the GSCLK timer output - this remains on
        LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH1);
        LL_TIM_EnableCounter(TIM4);

    #pragma GCC diagnostic pop  // ignored "-Wvolatile"  
    #endif // defined(USE_TLC5955_LL_DRIVER)

}


} // namespace tlc5955
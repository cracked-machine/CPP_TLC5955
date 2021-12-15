
// MIT License

// Copyright (c) 2021 Chris Sutton

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

#include <stdint.h>
#include <bitset>
#include <array>

#include <bitset_utils.hpp>

#ifdef USE_HAL_DRIVER
    // Required when using GCC 10.3.1 arm-none-eabi 
    // warning: compound assignment with 'volatile'-qualified left operand is deprecated [-Wvolatile]
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wvolatile"
		#include "main.h"	
	#pragma GCC diagnostic pop



#else
    #define UNUSED(X) (void)X 
#endif



#include <ssd1306.hpp>

namespace tlc5955 {



// experimental code can be found here
// https://godbolt.org/z/7bcsrK3aP

class Driver
{
public:

    Driver() = default;

    bool start_dma_transmit();
    bool pause_dma_transmit(bool pause);
   
    // @brief Clears (zeroize) the common register
    void reset();

    // @brief Set the latch cmd object 
    // @note
    // @param latch 
    void set_latch_cmd(const bool latch);

    // @brief Set the function cmd object
    // 
    // @param dsprpt 
    // @param tmgrst 
    // @param rfresh 
    // @param espwm 
    // @param lsdvlt 
    void set_function_cmd(const bool dsprpt, const bool tmgrst, const bool rfresh, const bool espwm, const bool lsdvlt);

    // @brief Set the global brightness cmd object
    // 
    // @param blue 
    // @param green 
    // @param red 
    void set_global_brightness_cmd(const uint8_t blue, const uint8_t green, const uint8_t red);

    // @brief Set the max current cmd object
    // 
    // @param blue 
    // @param green 
    // @param red 
    void set_max_current_cmd(const uint8_t blue, const uint8_t green, const uint8_t red);

    // @brief Set the dot correction cmd all object
    // 
    // @param pwm 
    void set_dot_correction_cmd_all(uint8_t pwm);

    // @brief Set the greyscale cmd all object
    // 
    // @param pwm 
    void set_greyscale_cmd_all(uint16_t pwm);

    void set_greyscale_cmd_rgb(uint16_t blue_pwm, uint16_t green_pwm, uint16_t red_pwm);

    // @brief convert the common register bitset to common register byte array.
    // 
    void process_register();

protected:

    static const uint8_t m_common_reg_size_bytes {97};
    std::array<uint8_t, m_common_reg_size_bytes> m_common_byte_register{0};

    static const uint16_t m_common_reg_size_bits {769};
    std::bitset<m_common_reg_size_bits> m_common_bit_register {0};

private:
    // @brief The bitset utility
    embedded_utils::bitset_utils bitsetter;

     // @brief The number of daisy chained driver chips in the circuit.
    uint8_t m_num_driver_ics {1}; 

    // @brief The number of colour channels per LED
    static const uint8_t m_num_colour_chan {3};
    
    // @brief The number of LEDs per driver chip
    static const uint8_t m_num_leds_per_chip {16};

    // @brief bits per latch select command
    static const uint8_t m_select_cmd_size {1};  
    // @brief bits per ctrl command
    static const uint8_t m_ctrl_cmd_size {8}; 
    // @brief bits per function command
    static const uint8_t m_func_cmd_size {5};    

    // @brief bits per brightness control data
    static const uint8_t m_bc_data_size {7};
    // @brief bits per max current data
    static const uint8_t m_mc_data_size {3};
    // @brief bits per dot correction data
    static const uint8_t m_dc_data_size {7};
    // @brief bits per grescale data
    static const uint8_t m_gs_data_size {16};

    // @brief total bits for the greyscale latch
    static constexpr uint16_t   m_gs_latch_size {m_gs_data_size * m_num_leds_per_chip * m_num_colour_chan};                                       
    // @brief total bits for the brightness control latch 
    static constexpr uint8_t    m_bc_latch_size {m_bc_data_size * m_num_colour_chan};                          
    // @brief total bits for the max current latch 
    static constexpr uint8_t    m_mc_latch_size {m_mc_data_size * m_num_colour_chan};                          
    // @brief total bits for the dot correction latch 
    static constexpr uint16_t   m_dc_latch_size {m_dc_data_size * m_num_leds_per_chip * m_num_colour_chan};    
    // @brief total bits for the padding
    static constexpr uint16_t   m_padding_size  {
        m_common_reg_size_bits - (m_select_cmd_size + m_ctrl_cmd_size + m_func_cmd_size + m_bc_latch_size + m_mc_latch_size + m_dc_latch_size) };

    // @brief select command latch offset
    static constexpr uint8_t    m_select_cmd_offset {0};
    // @brief control command latch offset
    static constexpr uint8_t    m_ctrl_cmd_offset   { static_cast<uint8_t>  (m_select_cmd_offset    + m_select_cmd_size)    };  
    // @brief padding offset
    static constexpr uint8_t    m_padding_offset    { static_cast<uint8_t>  (m_ctrl_cmd_offset      + m_ctrl_cmd_size)      }; 
    // @brief function command latch offset
    static constexpr uint16_t   m_func_cmd_offset   { static_cast<uint16_t> (m_padding_offset       + m_padding_size)       };  
    // @brief brightness control data latch offset
    static constexpr uint16_t   m_bc_data_offset    { static_cast<uint16_t> (m_func_cmd_offset      + m_func_cmd_size)      };  
    // @brief max current data latch offset
    static constexpr uint16_t   m_mc_data_offset    { static_cast<uint16_t> (m_bc_data_offset       + m_bc_latch_size)      };  
    // @brief dot correctness data latch offset
    static constexpr uint16_t   m_dc_data_offset    { static_cast<uint16_t> (m_mc_data_offset       + m_mc_latch_size)      }; 
    // @brief greyscale data latch offset
    static constexpr uint8_t    m_gs_data_offset    { static_cast<uint8_t>  (m_ctrl_cmd_offset)};                             

    // @brief padding 
    std::bitset<m_padding_size> m_padding {0x00};
    std::bitset<m_ctrl_cmd_size> m_ctrl_cmd {0x96};


    // void enable_gpio_output_only();
#ifdef USE_HAL_DRIVER 
    // @brief The HAL SPI interface
    SPI_HandleTypeDef m_spi_interface {hspi2};
    // @brief Latch GPIO pin
	uint16_t m_lat_pin {TLC5955_SPI2_LAT_Pin};
    // @brief Latch terminal GPIO port
    GPIO_TypeDef* m_lat_port {TLC5955_SPI2_LAT_GPIO_Port};
    // @brief GreyScale clock GPIO pin
    uint16_t m_gsclk_pin {TLC5955_SPI2_GSCLK_Pin};
    // @brief GreyScale clock GPIO port
    GPIO_TypeDef* m_gsclk_port {TLC5955_SPI2_GSCLK_GPIO_Port};
    // @brief SPI MOSI GPIO pin
    uint16_t m_mosi_pin {TLC5955_SPI2_MOSI_Pin};
    // @brief SPI MOSI GPIO port
    GPIO_TypeDef* m_mosi_port {TLC5955_SPI2_MOSI_GPIO_Port};
    // @brief SPI Clock GPIO pin
    uint16_t m_sck_pin {TLC5955_SPI2_SCK_Pin};
    // @brief SPI Clock GPIO port
    GPIO_TypeDef* m_sck_port {TLC5955_SPI2_SCK_GPIO_Port};
#endif	

};


} // tlc5955
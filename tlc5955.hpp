
#include <stdint.h>
#include <bitset>

#include "stm32g0xx.h"
#include "main.h"
#include "spi.h"

#include <ssd1306.hpp>

namespace tlc5955 {

class Driver
{
public:

    Driver() = default;

    virtual ~Driver() = default;


    // @brief Set the Function Control (FC) data latch.
    // See section 8.3.2.7 "Function Control (FC) Data Latch" (page 23).
    // https://www.ti.com/lit/ds/symlink/tlc5955.pdf
    // @param DSPRPT Auto display repeat mode enable bit. When enabled each output repeats the PWM control every 65,536 GSCLKs.
    // @param TMGRST Display timing reset mode enable bit. When enabled the GS counter resets and outputs forced off at the latch rising edge
    // for a GS data write
    // @param RFRESH Data in the common register are copied to the GS data latch and DC data in the control data latch are copied to the DC data latch
    // at the 65,536th GSCLK after the LAT rising edge for a GS data write.
    // @param ESPWM When 0, conventional PWM is selected. When 1, Enhanced Spectrum (ES) PWM is selected. See 8.4.4 "Grayscale (GS) Function (PWM Control)"
    // @param LSDVLT LED short detection (LSD) detection voltage selection bit. When this bit is 0, the LSD voltage is VCC × 70%. 
    // When this bit is 1, the LSD voltage is VCC × 90%. See 8.3.5 "LED Short Detection (LSD)"
    void set_function_data(bool DSPRPT, bool TMGRST, bool RFRESH, bool ESPWM, bool LSDVLT);
  
    // @brief The number of daisy chained driver chips in the circuit.
    static const uint8_t m_num_driver_ics; 

    // @brief The number of colour channels per LED
    static const uint8_t m_num_colour_chan = 3;
    // @brief The number of LEDs per driver chip
    static const uint8_t m_num_leds_per_chip = 16;



    void send_grayscale_data();
    void send_control_data();

    void set_dc_reg(uint8_t value)
    {
        value > m_dc_reg_max ? m_dc_reg = m_dc_reg_max : m_dc_reg = value;
    }

    void set_bc_reg(uint8_t value)
    {
        value > m_bc_reg_max ? m_bc_reg = m_bc_reg_max : m_bc_reg = value;
    }



    // @brief toggle the latch pin terminal
    void toggle_latch();


private:

    // Bits required for correct control reg size
    const uint16_t  m_control_zero_bits_size {389};   
    const uint8_t  m_total_reg_size {76};
    const uint8_t  m_latch_delay_ms {1};
    const uint8_t  m_ctrl_write_cnt {2};


 
    // @brief Dot Correction (DC) register 
    static const uint8_t  m_dc_reg_max {7};
    std::bitset<m_dc_reg_max> m_dc_reg {127};

    // @brief Brightness Control (BC) register 
    static const uint8_t  m_bc_reg_max {7};
    std::bitset<m_bc_reg_max> m_bc_reg {127};

    // @brief Max Current (MC) register size
    static const uint8_t  m_max_current_reg_max {3};
    std::bitset<m_max_current_reg_max> m_mc_reg {4};

    // @brief Function Control (FC) register
    static const uint8_t  m_function_ctrl_reg_max = 5;
    std::bitset<m_function_ctrl_reg_max> m_function_data {0};

    
    // @brief temp buffer for red LED brightness control data 
    std::bitset<m_bc_reg_max> m_bright_red {0};
    // @brief temp buffer for green LED brightness control data
    std::bitset<m_bc_reg_max> m_bright_green {0};
    // @brief temp buffer for blue LED brightness control data
    std::bitset<m_bc_reg_max> m_bright_blue {0};
    // @brief temp buffer for red LED max current data
    std::bitset<m_max_current_reg_max> m_mc_red {0};
    // @brief temp buffer for green LED max current data
    std::bitset<m_max_current_reg_max> m_mc_green {0};
    // @brief temp buffer for blue LED max current data
    std::bitset<m_max_current_reg_max> m_mc_blue {0};

    // @brief Predefined write command.
    // section 8.3.2.3 "Control Data Latch" (page 21).
    // section 8.3.2.2 "Grayscale (GS) Data Latch" (page 20).
    // https://www.ti.com/lit/ds/symlink/tlc5955.pdf
    std::bitset<8> m_write_cmd {0x96};

    // @brief Predefined flush command
    std::bitset<8> m_flush_cmd {0};

 
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
	

};

} // tlc5955
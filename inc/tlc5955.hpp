
#include <stdint.h>
#include <bitset>

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

// https://godbolt.org/z/1q9sn3Gar

class Driver
{
public:

    Driver() = default;

    virtual ~Driver() = default;

    bool start_dma_transmit();
    bool pause_dma_transmit(bool pause);

    static const uint8_t m_bc_data_resolution {7};
    static const uint8_t m_mc_data_resolution {3};
    static const uint8_t m_dc_data_resolution {7};
    static const uint8_t m_gs_data_resolution {16};

    void set_control_bit(bool ctrl);

    void set_ctrl_cmd_bits();

    void set_padding_bits();

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
  
    // @brief Write the Global BC (Bright Control) data to the common register.
    // https://www.ti.com/lit/ds/symlink/tlc5955.pdf
    // @param blue_value The 7-bit word for blue BC
    // @param green_value The 7-bit word for green BC
    // @param red_value The 7-bit word for red BC    
    void set_bc_data(
        const std::bitset<m_bc_data_resolution> &blue_value, 
        const std::bitset<m_bc_data_resolution> &green_value, 
        const std::bitset<m_bc_data_resolution> &red_value);

    // @brief Write the MC (Max Current) data to the common register
    // https://www.ti.com/lit/ds/symlink/tlc5955.pdf
    // @param blue_value The 3-bit word for blue MC
    // @param green_value The 3-bit word for green MC
    // @param red_value The 3-bit word for red MC
    void set_mc_data(
        const std::bitset<m_mc_data_resolution> &blue_value, 
        const std::bitset<m_mc_data_resolution> green_value, 
        const std::bitset<m_mc_data_resolution> &red_value);

    // @brief Write the DC (dot correction) data to the common register for the specified LED
    // https://www.ti.com/lit/ds/symlink/tlc5955.pdf
    // @param led_idx The selected LED
    // @param blue_value The 7-bit word for blue DC
    // @param green_value The 7-bit word for green DC
    // @param red_value The 7-bit word for red DC
    bool set_dc_data(
        uint8_t led_idx, 
        const std::bitset<m_dc_data_resolution> &blue_value, 
        const std::bitset<m_dc_data_resolution> &green_value, 
        const std::bitset<m_dc_data_resolution> &red_value);

    // @brief Convenience function to set all LEDs to the same DC values
    // @param blue_value The 7-bit word for blue DC
    // @param green_value The 7-bit word for green DC
    // @param red_value The 7-bit word for red DC
    void set_all_dc_data(
        const std::bitset<m_dc_data_resolution> &blue_value, 
        const std::bitset<m_dc_data_resolution> &green_value, 
        const std::bitset<m_dc_data_resolution> &red_value);

    // @brief Write the GS (Grey Scale) data to the common register for the specified LED
    // @param led_pos The selected LED
    // @param blue_value The 16-bit word for blue GS
    // @param green_value The 16-bit word for green GS
    // @param red_value The 16-bit word for red GS
    bool set_gs_data(
        uint8_t led_idx, 
        const std::bitset<m_gs_data_resolution> &blue_value, 
        const std::bitset<m_gs_data_resolution> &green_value, 
        const std::bitset<m_gs_data_resolution> &red_value);

    // @brief Convenience function to set all LEDs to the same GS values
    // @param blue_value The 16-bit word for blue GS
    // @param green_value The 16-bit word for green GS
    // @param red_value The 16-bit word for red GS
    void set_all_gs_data(
        const std::bitset<m_gs_data_resolution> &blue_value, 
        const std::bitset<m_gs_data_resolution> &green_value, 
        const std::bitset<m_gs_data_resolution> &red_value);

    // @brief Send the data via SPI bus and toggle the latch pin
    void send_data();
    
    // @brief Clears (zeroize) the common register and call send_data()
    void flush_common_register();

    // @brief toggle the latch pin terminal
    void toggle_latch();

    // @brief Helper function to print bytes as decimal values to RTT. USE_RTT must be defined.
    void print_common_bits();

    // @brief sections offsets for common register
    enum byte_offsets {
        // @brief 1 for control data latch, 0 for greyscale data latch 
        latch = 0U,
        // @brief Used in control data latch.
        ctrl_cmd = 0U,
        // @brief Used in greyscale data latch. 
        greyscale = 1U,
        // @brief Used in control data latch. Don't care bits.
        padding = 1U,
        // @brief Used in control data latch.
        function = 49U,
        // @brief Used in control data latch.
        brightness_control = 50U,
        // @brief Used in control data latch.
        max_current = 53U,
        // @brief Used in control data latch.
        dot_correct = 54U
    };

    

protected:

    static const uint8_t m_common_reg_size_bytes {97};
    std::array<uint8_t, m_common_reg_size_bytes> m_common_byte_register{0};

private:

    uint8_t built_in_test_fail {0};

    // Bits required for correct control reg size
    static const uint16_t m_common_reg_size_bits {769};


     // @brief The number of daisy chained driver chips in the circuit.
    uint8_t m_num_driver_ics {1}; 

    // @brief The number of colour channels per LED
    static const uint8_t m_num_colour_chan {3};
    
    // @brief The number of LEDs per driver chip
    static const uint8_t m_num_leds_per_chip {16};



    // the size of each common register section
    static const uint8_t m_latch_size_bits {1};                                                                             // 1U
    static const uint8_t m_ctrl_cmd_size_bits {8};                                                                          // 8U
    static constexpr uint16_t m_gs_data_one_led_size_bits {m_gs_data_resolution * m_num_colour_chan};                       // 48U
    static constexpr uint16_t m_gs_data_section_size_bits {m_gs_data_resolution * m_num_leds_per_chip * m_num_colour_chan}; // 768U
    static const uint8_t m_func_data_section_size_bits {5};                                                                 // 5U
    static constexpr uint8_t m_bc_data_section_size_bits {m_bc_data_resolution * m_num_colour_chan};                        // 21U
    static constexpr uint8_t m_mc_data_section_size_bits {m_mc_data_resolution * m_num_colour_chan};                        // 9U
    static constexpr uint8_t m_dc_data_one_led_size_bits {m_dc_data_resolution * m_num_colour_chan};                        // 21U
    static constexpr uint16_t m_dc_data_section_size_bits {m_dc_data_resolution * m_num_leds_per_chip * m_num_colour_chan}; // 336U
    static constexpr uint16_t m_padding_section_size_bits {                                                                 // 389U
        m_common_reg_size_bits  - m_latch_size_bits - m_ctrl_cmd_size_bits - m_func_data_section_size_bits - m_bc_data_section_size_bits - m_mc_data_section_size_bits - m_dc_data_section_size_bits
    };

    // the offset of each common register section
    // static const uint8_t m_latch_offset {0};
    // static constexpr uint8_t m_ctrl_cmd_offset {static_cast<uint8_t>(m_latch_offset + m_latch_size_bits)};                  // 1U
    // static constexpr uint8_t m_gs_data_offset {static_cast<uint8_t>(m_ctrl_cmd_offset)};                                    // 9U - used in gs data latch only
    // static constexpr uint8_t m_padding_offset {static_cast<uint8_t>(m_ctrl_cmd_offset + m_ctrl_cmd_size_bits)};             // 9U - used in ctrl data latch only
    // static constexpr uint16_t m_func_data_offset {static_cast<uint16_t>(m_padding_offset + m_padding_section_size_bits)};   // 9U
    // static constexpr uint16_t m_bc_data_offset {static_cast<uint16_t>(m_func_data_offset + m_func_data_section_size_bits)}; // 398U
    // static constexpr uint16_t m_mc_data_offset {static_cast<uint16_t>(m_bc_data_offset + m_bc_data_section_size_bits)};     // 424U
    // static constexpr uint16_t m_dc_data_offset {static_cast<uint16_t>(m_mc_data_offset + m_mc_data_section_size_bits)};     // 433U


    // @brief Helper function to set/clear one bit of one byte in the common register byte array
    // @param byte The targetted byte in the common register
    // @param bit The bit within that byte to be set/cleared
    // @param _new_value The boolean value to set at the bit target_idx
    void set_value_nth_bit(uint8_t &byte, uint16_t bit, bool new_value);

    
    //std::bitset<m_common_reg_size_bits> m_common_bit_register{0};

    const uint8_t  m_latch_delay_ms {1};

    // @brief Predefined write command.
    // section 8.3.2.3 "Control Data Latch" (page 21).
    // section 8.3.2.2 "Grayscale (GS) Data Latch" (page 20).
    // https://www.ti.com/lit/ds/symlink/tlc5955.pdf
    std::bitset<8> m_ctrl_cmd {0x96};

    // @brief Predefined flush command
    std::bitset<8> m_flush_cmd {0};

    // void enable_spi();
    // void disable_spi();

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
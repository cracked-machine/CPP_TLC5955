
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

    virtual ~Driver() = default;

    bool start_dma_transmit();
    bool pause_dma_transmit(bool pause);
   
    // @brief Clears (zeroize) the common register and call send_data()
    void reset();
    void set_latch_cmd(const bool latch);
    void set_function_cmd(const bool dsprpt, const bool tmgrst, const bool rfresh, const bool espwm, const bool lsdvlt);
    void set_global_brightness_cmd(const uint8_t blue, const uint8_t green, const uint8_t red);
    void set_max_current_cmd(const uint8_t blue, const uint8_t green, const uint8_t red);
    void set_dot_correction_cmd_all(uint8_t pwm);
    void set_greyscale_cmd_all(uint16_t pwm);

    void process_register();

    // @brief Helper function to print bytes as decimal values to RTT. USE_RTT must be defined.
    void print_common_bits();



protected:


private:

    embedded_utils::bitset_utils bitsetter;

    // // @brief sections offsets for common register
    // enum byte_offsets {
    //     // @brief 1 for control data latch, 0 for greyscale data latch 
    //     latch = 0U,
    //     // @brief Used in control data latch.
    //     ctrl_cmd = 0U,
    //     // @brief Used in greyscale data latch. 
    //     greyscale = 1U,
    //     // @brief Used in control data latch. Don't care bits.
    //     padding = 1U,
    //     // @brief Used in control data latch.
    //     function = 49U,
    //     // @brief Used in control data latch.
    //     brightness_control = 50U,
    //     // @brief Used in control data latch.
    //     max_current = 53U,
    //     // @brief Used in control data latch.
    //     dot_correct = 54U
    // };

            // @brief Predefined write command.
    // section 8.3.2.3 "Control Data Latch" (page 21).
    // section 8.3.2.2 "Grayscale (GS) Data Latch" (page 20).
    // https://www.ti.com/lit/ds/symlink/tlc5955.pdf
    std::bitset<8> m_ctrl_cmd {0x96};

    // @brief Predefined flush command
    std::bitset<8> m_flush_cmd {0};

    static const uint8_t m_common_reg_size_bytes {97};
    std::array<uint8_t, m_common_reg_size_bytes> m_common_byte_register{0};

    // Bits required for correct control reg size
    static const uint16_t m_common_reg_size_bits {769};

    std::bitset<m_common_reg_size_bits> m_common_bit_register {0};


     // @brief The number of daisy chained driver chips in the circuit.
    uint8_t m_num_driver_ics {1}; 

    // @brief The number of colour channels per LED
    static const uint8_t m_num_colour_chan {3};
    
    // @brief The number of LEDs per driver chip
    static const uint8_t m_num_leds_per_chip {16};


    static const uint8_t m_bc_data_resolution {7};
    static const uint8_t m_mc_data_resolution {3};
    static const uint8_t m_dc_data_resolution {7};
    static const uint8_t m_gs_data_resolution {16};

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

    std::bitset<tlc5955::Driver::m_padding_section_size_bits> m_padding {0x00};

    // the offset of each common register section
    static const uint8_t m_latch_offset {0};
    static constexpr uint8_t m_ctrl_cmd_offset {static_cast<uint8_t>(m_latch_offset + m_latch_size_bits)};                  // 1U
    static constexpr uint8_t m_gs_data_offset {static_cast<uint8_t>(m_ctrl_cmd_offset)};                                    // 9U - used in gs data latch only
    static constexpr uint8_t m_padding_offset {static_cast<uint8_t>(m_ctrl_cmd_offset + m_ctrl_cmd_size_bits)};             // 9U - used in ctrl data latch only
    static constexpr uint16_t m_func_data_offset {static_cast<uint16_t>(m_padding_offset + m_padding_section_size_bits)};   // 9U
    static constexpr uint16_t m_bc_data_offset {static_cast<uint16_t>(m_func_data_offset + m_func_data_section_size_bits)}; // 398U
    static constexpr uint16_t m_mc_data_offset {static_cast<uint16_t>(m_bc_data_offset + m_bc_data_section_size_bits)};     // 424U
    static constexpr uint16_t m_dc_data_offset {static_cast<uint16_t>(m_mc_data_offset + m_mc_data_section_size_bits)};     // 433U


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
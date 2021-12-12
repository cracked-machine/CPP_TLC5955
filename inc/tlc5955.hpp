
#include <stdint.h>
#include <bitset>
#include <array>

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

    static const uint8_t m_bc_data_resolution {7};
    static const uint8_t m_mc_data_resolution {3};
    static const uint8_t m_dc_data_resolution {7};
    static const uint8_t m_gs_data_resolution {16};

    
    // @brief Clears (zeroize) the common register and call send_data()
    void flush_common_register();

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

    // Bits required for correct control reg size
    static const uint16_t m_common_reg_size_bits {769};

    std::bitset<m_common_reg_size_bits> m_common_bit_register {0};

    template<std::size_t TARGET_SIZE, std::size_t SOURCE_SIZE> 
    void add_bitset(std::bitset<TARGET_SIZE> &target,  const std::bitset<SOURCE_SIZE> &source, const uint16_t &msb_offset);
    

    template<std::size_t TARGET_SIZE, std::size_t SOURCE_SIZE> 
    void bitset_to_bytearray(std::array<uint8_t, TARGET_SIZE> &target_array, const std::bitset<SOURCE_SIZE> &source_bitset);  

    
    template<std::size_t BITSET_SIZE>
    void print_bits(std::bitset<BITSET_SIZE> &reg);


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
    static const uint8_t m_latch_offset {0};
    static constexpr uint8_t m_ctrl_cmd_offset {static_cast<uint8_t>(m_latch_offset + m_latch_size_bits)};                  // 1U
    static constexpr uint8_t m_gs_data_offset {static_cast<uint8_t>(m_ctrl_cmd_offset)};                                    // 9U - used in gs data latch only
    static constexpr uint8_t m_padding_offset {static_cast<uint8_t>(m_ctrl_cmd_offset + m_ctrl_cmd_size_bits)};             // 9U - used in ctrl data latch only
    static constexpr uint16_t m_func_data_offset {static_cast<uint16_t>(m_padding_offset + m_padding_section_size_bits)};   // 9U
    static constexpr uint16_t m_bc_data_offset {static_cast<uint16_t>(m_func_data_offset + m_func_data_section_size_bits)}; // 398U
    static constexpr uint16_t m_mc_data_offset {static_cast<uint16_t>(m_bc_data_offset + m_bc_data_section_size_bits)};     // 424U
    static constexpr uint16_t m_dc_data_offset {static_cast<uint16_t>(m_mc_data_offset + m_mc_data_section_size_bits)};     // 433U


        // @brief Predefined write command.
    // section 8.3.2.3 "Control Data Latch" (page 21).
    // section 8.3.2.2 "Grayscale (GS) Data Latch" (page 20).
    // https://www.ti.com/lit/ds/symlink/tlc5955.pdf
    std::bitset<8> m_ctrl_cmd {0x96};


    static const uint8_t m_common_reg_size_bytes {97};
    std::array<uint8_t, m_common_reg_size_bytes> m_common_byte_register{0};

protected:


private:

    uint8_t built_in_test_fail {0};







 



    // @brief Helper function to set/clear one bit of one byte in the common register byte array
    // @param byte The targetted byte in the common register
    // @param bit The bit within that byte to be set/cleared
    // @param _new_value The boolean value to set at the bit target_idx
    void set_value_nth_bit(uint8_t &byte, uint16_t bit, bool new_value);

    
    //std::bitset<m_common_reg_size_bits> m_common_bit_register{0};

    const uint8_t  m_latch_delay_ms {1};



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


template<std::size_t TARGET_SIZE, std::size_t SOURCE_SIZE> 
void Driver::add_bitset(std::bitset<TARGET_SIZE> &target,  const std::bitset<SOURCE_SIZE> &source, const uint16_t &msb_offset) 
{

    //std::cout << source << std::endl;


    // iterate over the source bitset pattern
    for (uint16_t idx = 0; idx < source.size(); idx++)
    {
        //std::cout << idx << "#:" << source.test(idx) << std::endl;
        if (source.test(idx))
        {
            // start from the common register msb and work backwards towards lsb,
            // ...minus the offset
            target.set(msb_offset + (source.size() - 1)  - idx, true);
        }
        else
        {
            target.set(msb_offset + (source.size() - 1) - idx , false);
            //target.set(source_msb_idx - idx + msb_offset, false);
        }

    }
}

template<std::size_t TARGET_SIZE, std::size_t SOURCE_SIZE> 
void Driver::bitset_to_bytearray(std::array<uint8_t, TARGET_SIZE> &target_array, const std::bitset<SOURCE_SIZE> &source_bitset)
{
    for (uint16_t byte_idx = 0; byte_idx < target_array.size(); byte_idx++)
    {
        //std::cout << "Byte #" << byte_idx << std::endl;
        uint8_t word_size_bits = 8;
        uint16_t byte_offset = byte_idx * word_size_bits;
        int8_t bit_offset = word_size_bits - 1;
        for (uint16_t pattern_idx = byte_offset; pattern_idx < byte_offset + word_size_bits; pattern_idx++)
        {   
            if (pattern_idx < source_bitset.size())
            {
                target_array[byte_idx] |= (source_bitset.test(pattern_idx) << bit_offset);
                bit_offset--;
            }
        }        
        
    }
}

template<std::size_t BITSET_SIZE>
void Driver::print_bits(std::bitset<BITSET_SIZE> &reg)
{
    UNUSED(reg);
    #ifdef USE_RTT
        for (uint16_t idx = 0; idx < reg.size(); idx++)
        {
            if (idx % 8 == 0)
                SEGGER_RTT_printf(0, " ");
            if (idx % 64 == 0)
                SEGGER_RTT_printf(0, "\n");            
            SEGGER_RTT_printf(0, "%u ", +reg.test(idx));
        }
    #endif
}


} // tlc5955

// MIT License

// Copyright (c) 2022 Chris Sutton

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

#ifndef __TLC5955_HPP__
#define __TLC5955_HPP__

#include <tlc5955_device.hpp>

namespace tlc5955 {

class Driver : public RestrictedBase
{
public:
    // @brief Construct a new Driver object
    // @param serial_interface tlc5955::DriverSerialInterface object containing the SPI device pointer and related pins/ports/settings
    explicit Driver(DriverSerialInterface &serial_interface);

    // @brief The type of first bit to send
    enum class DataLatchType {
        control,
        data
    };

    // @brief latch immediately or defer
    enum class LatchPinOption {
        no_latch,
        latch_after_send
    };    

    // @brief Auto display repeat mode enable bit
    enum class DisplayFunction {
        // @brief The auto display repeat function is disabled. Each constant-current output is turned on and off for one display period
        display_repeat_off,
        // @brief Each output repeats the PWM control every 65,536 GSCLKs.
        display_repeat_on
    };

    // @brief Display timing reset mode enable bit
    enum class TimingFunction {
        // @brief The GS counter is not reset and the outputs are not forced off even when a LAT rising edge is input for a GS data write.
        timing_reset_off,
        // @brief The GS counter is reset to 0 and all outputs are forced off at the LAT rising edge for a GS data write. 
        // Afterwards, PWM control resumes from the next GSCLK rising edge.
        timing_reset_on
    };

    // @brief Auto data refresh mode enable bit
    enum class RefreshFunction {
        // @brief The auto data refresh function is disabled. The data in the common shift register are copied to the GS data latch 
        // at the next LAT rising edge for a GS data write. DC data in the control data latch are copied to the DC data latch at the same time.
        auto_refresh_off,
        // @brief The auto data refresh function is enabled. The data in the common shift register are copied to the GS data latch 
        // at the 65,536th GSCLK after the LAT rising edge for a GS data write. DC data in the control data latch are copied to the DC data latch at the same time
        auto_refresh_on
    };

    // @brief ES-PWM mode enable bit
    enum class PwmFunction {
        // @brief Conventional PWM control mode is selected. If the TLC5955 is used for multiplexing a drive, 
        // the conventional PWM mode should be selected to prevent excess on or off switching.
        normal_pwm,
        // @brief Enhanced Spectrum PWM (ES-PWM) control mode is selected
        enhanced_pwm
    };

    // @brief LED short detection voltage selection bit. LED short detection (LSD) detects a fault caused by a shorted LED by comparing the
    // OUTXn voltage to the LSD detection threshold voltage. The threshold voltage is selected by this bit.
    enum class ShortDetectFunction {
        // @brief The LSD voltage is VCC × 70%.
        threshold_70_percent,
        // @brief The LSD voltage is VCC × 90%.
        threshold_90_percent
    };

    // @brief Clears (zeroize) the common register
    void reset();

    // @brief Manually set the first bit (control or data)
    // Disables SPI pins and uses them as GPIO to manually send the first bit
    // @param latch_type control message or data message
    void send_first_bit(const DataLatchType latch_type);

    // @brief Set the "don't care" padding bits
    void set_padding_bits();

    // @brief Set the predefined ctrl cmd
    void set_ctrl_cmd();

    // @brief Set the function cmd object. See class enums above.
    // @param dsprpt Auto display repeat mode enable bit
    // @param tmgrst Display timing reset mode enable bit
    // @param rfresh Auto data refresh mode enable bit
    // @param espwm ES-PWM mode enable bit
    // @param lsdvlt LED short detection voltage selection bit.
    void set_function_cmd(DisplayFunction dsprpt, TimingFunction tmgrst, RefreshFunction rfresh, PwmFunction espwm, ShortDetectFunction lsdvlt);

    // @brief Set the global brightness cmd object
    // @param blue 
    // @param green 
    // @param red 
    /// @todo add error checking
    void set_global_brightness_cmd(const uint8_t blue, const uint8_t green, const uint8_t red);

    // @brief Set the max current cmd object
    // @param blue 
    // @param green 
    // @param red 
    /// @todo add error checking
    void set_max_current_cmd(const uint8_t blue, const uint8_t green, const uint8_t red);

    // @brief Set the dot correction bits in the buffer
    // @param pwm 
    /// @todo add error checking
    void set_dot_correction_cmd_all(uint8_t pwm);

    // @brief Set the greyscale bits in the buffer
    // @param pwm 
    void set_greyscale_cmd_white(uint16_t pwm);

    // @brief Set the greyscale RGB bits in the buffer for all LEDs at same time
    // @param blue_pwm Must be value: 0-2^16
    // @param green_pwm Must be value: 0-2^16
    // @param red_pwm Must be value: 0-2^16
    void set_greyscale_cmd_rgb(uint16_t blue_pwm, uint16_t green_pwm, uint16_t red_pwm);

    // @brief Set the greyscale RGB bits in the buffer at specific LED position
    // @param led_idx Must be value: 0-15
    // @param red_pwm Must be value: 0-2^16
    // @param green_pwm Must be value: 0-2^16
    // @param blue_pwm Must be value: 0-2^16
    bool set_greyscale_cmd_rgb_at_position(uint16_t led_idx, uint16_t red_pwm, uint16_t green_pwm, uint16_t blue_pwm);

    // @brief Send the buffer once to the TLC5955 chip via SPI and options with/without latch
    // @param latch_option latch after send or no latch after send
    bool send_spi_bytes(LatchPinOption latch_option);

    // @brief convert the buffer from bits (std::bitset) to bytes (std::array).
    void process_register();

protected:

    // @brief The number of bytes in the buffer
    static const uint8_t m_common_reg_size_bytes {96};
    // @brief object holding the buffer in byte format
    std::array<uint8_t, m_common_reg_size_bytes> m_common_byte_register{0};

    // @brief The number of bits in the buffer
    static const uint16_t m_common_reg_size_bits {768};
    // @brief object holding the buffer in bit format
    std::bitset<m_common_reg_size_bits> m_common_bit_register {0};

private:


	// object containing SPI port/pins and pointer to CMSIS defined SPI peripheral
	DriverSerialInterface m_serial_interface;

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
        m_common_reg_size_bits - (m_ctrl_cmd_size + m_func_cmd_size + m_bc_latch_size + m_mc_latch_size + m_dc_latch_size)};

    // @brief select command latch offset
    static constexpr uint8_t    m_select_cmd_offset {0};
    // @brief control command latch offset
    static constexpr uint8_t    m_ctrl_cmd_offset   {static_cast<uint8_t>  (m_select_cmd_offset)};  
    // @brief padding offset
    static constexpr uint8_t    m_padding_offset    {static_cast<uint8_t>  (m_ctrl_cmd_offset + m_ctrl_cmd_size)}; 
    // @brief function command latch offset
    static constexpr uint16_t   m_func_cmd_offset   {static_cast<uint16_t> (m_padding_offset + m_padding_size)};  
    // @brief brightness control data latch offset
    static constexpr uint16_t   m_bc_data_offset    {static_cast<uint16_t> (m_func_cmd_offset + m_func_cmd_size)};  
    // @brief max current data latch offset
    static constexpr uint16_t   m_mc_data_offset    {static_cast<uint16_t> (m_bc_data_offset + m_bc_latch_size)};  
    // @brief dot correctness data latch offset
    static constexpr uint16_t   m_dc_data_offset    {static_cast<uint16_t> (m_mc_data_offset + m_mc_latch_size)}; 
    // @brief greyscale data latch offset
    static constexpr uint8_t    m_gs_data_offset    {static_cast<uint8_t>  (m_ctrl_cmd_offset)};                             

    // @brief Don't Care bits. We set last bit to 1 for diagnostics purposes
    std::bitset<m_padding_size> m_padding {0x01};

    // @brief The control command. Always 0x96 (0b10010110)
    std::bitset<m_ctrl_cmd_size> m_ctrl_cmd {0x96};

    // @brief init the PB7/PB8 pins as SPI peripheral.
    void spi2_init(void);

    // @brief init the PB7/PB8 pins as GPIO outputs.
    void gpio_init(void);
};


} // namespace tlc5955

#endif // __TLC5955_HPP__
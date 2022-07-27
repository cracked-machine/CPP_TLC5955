
#include "tlc5955.hpp"

#include <cassert>
#include <cmath>
#include <cstring>

#if defined(X86_UNIT_TESTING_ONLY)
  #if defined(USE_RTT)
    #include <SEGGER_RTT.h>
  #endif
#else
  #include <spi_utils_ref.hpp>
#endif

namespace tlc5955
{

Driver::Driver(const DriverSerialInterface &serial_interface)
    : m_serial_interface(serial_interface)
{

  // Setup GPIO clock. Used to send first bit
  __IO uint32_t tmpreg;
  RCC->IOPENR = RCC->IOPENR | m_serial_interface.get_rcc_gpio_clk();
  tmpreg      = (RCC->IOPENR & m_serial_interface.get_rcc_gpio_clk());
  (void)tmpreg;

// Setup SPI clock. Used to send subsequent 96 bytes over SPI
#ifndef X86_UNIT_TESTING_ONLY
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wvolatile"
#endif
  SET_BIT(RCC->APBENR1, m_serial_interface.get_rcc_spi_clk());
#ifndef X86_UNIT_TESTING_ONLY
  #pragma GCC diagnostic pop
#endif
}

void Driver::init(DisplayFunction display,
                  TimingFunction timing,
                  RefreshFunction refresh,
                  PwmFunction pwm,
                  ShortDetectFunction short_detect,
                  std::array<uint8_t, 3> global_brightness,
                  std::array<uint8_t, 3> max_current,
                  uint8_t global_dot_correction)
{

  clear_register();
  set_ctrl_cmd();
  set_padding_bits();
  set_function_cmd(display, timing, refresh, pwm, short_detect);

  set_global_brightness_cmd(global_brightness[0], global_brightness[1], global_brightness[2]);
  set_max_current_cmd(max_current[0], max_current[1], max_current[2]);
  set_dot_correction_cmd_all(global_dot_correction);

  // send data for top row (no latch)
  send_first_bit(DataLatchType::control);
  send_spi_bytes(LatchPinOption::no_latch);

  // send data for bottom row
  send_first_bit(DataLatchType::control);
  send_spi_bytes(LatchPinOption::latch_after_send);
}

// @brief class to implement TLC5955 LED Driver IC
// Refer to datasheet - https://www.ti.com/lit/ds/symlink/tlc5955.pdf
void Driver::clear_register()
{
  m_common_bit_register.reset();
  m_common_byte_register.fill(0);
}

void Driver::send_first_bit(DataLatchType latch_type [[maybe_unused]])
{
#if not defined(X86_UNIT_TESTING_ONLY)

  // convert the bit buffer to bytes
  noarch::bit_manip::bitset_to_bytearray(m_common_byte_register, m_common_bit_register);

  stm32::spi_ref::enable_spi(m_serial_interface.get_spi_handle(), false);

  // set PB7/PB8 as GPIO outputs
  gpio_init();

  // make sure LAT pin is low otherwise first latch may be skipped (and TLC5955 will initialise intermittently)
  LL_GPIO_ResetOutputPin(&m_serial_interface.get_lat_port(), m_serial_interface.get_lat_pin());

  // "Control Data Latch" - Start SPI transacation by clocking in one high bit
  if (latch_type == DataLatchType::control)
  {
    // reset both SCK and MOSI
    LL_GPIO_ResetOutputPin(&m_serial_interface.get_mosi_port(), m_serial_interface.get_mosi_pin());
    LL_GPIO_ResetOutputPin(&m_serial_interface.get_mosi_port(), m_serial_interface.get_sck_pin());

    // MOSI data clocked on high(1) rising edge of SCK
    LL_GPIO_SetOutputPin(&m_serial_interface.get_mosi_port(), m_serial_interface.get_mosi_pin());
    LL_GPIO_SetOutputPin(&m_serial_interface.get_mosi_port(), m_serial_interface.get_sck_pin());

    // reset both SCK and MOSI
    LL_GPIO_ResetOutputPin(&m_serial_interface.get_mosi_port(), m_serial_interface.get_mosi_pin());
    LL_GPIO_ResetOutputPin(&m_serial_interface.get_mosi_port(), m_serial_interface.get_sck_pin());
  }
  // "GS Data Latch" - Start SPI transacation by clocking in one low bit
  else
  {
    // reset both SCK and MOSI
    LL_GPIO_ResetOutputPin(&m_serial_interface.get_mosi_port(), m_serial_interface.get_sck_pin());
    LL_GPIO_ResetOutputPin(&m_serial_interface.get_mosi_port(), m_serial_interface.get_mosi_pin());

    // MOSI data clocked low(0) on rising edge of SCK
    LL_GPIO_SetOutputPin(&m_serial_interface.get_mosi_port(), m_serial_interface.get_sck_pin());
    LL_GPIO_ResetOutputPin(&m_serial_interface.get_mosi_port(), m_serial_interface.get_mosi_pin());

    // reset both SCK and MOSI
    LL_GPIO_ResetOutputPin(&m_serial_interface.get_mosi_port(), m_serial_interface.get_sck_pin());
    LL_GPIO_ResetOutputPin(&m_serial_interface.get_mosi_port(), m_serial_interface.get_mosi_pin());
  }

  // set PB7/PB8 to SPI
  spi2_init();
  stm32::spi_ref::enable_spi(m_serial_interface.get_spi_handle());

#endif
}

void Driver::set_padding_bits() { noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, m_padding, m_padding_offset); }

void Driver::set_ctrl_cmd() { noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, m_ctrl_cmd, m_ctrl_cmd_offset); }

void Driver::set_function_cmd(DisplayFunction dsprpt, TimingFunction tmgrst, RefreshFunction rfresh, PwmFunction espwm, ShortDetectFunction lsdvlt)
{
  std::bitset<m_func_cmd_size> function_cmd{};
  // don't use std::bitset.set(), this will force exception handling to bloat the linked .elf
  (dsprpt == DisplayFunction::display_repeat_on) ? (function_cmd[4] = true) : (function_cmd[4] = false);
  (tmgrst == TimingFunction::timing_reset_on) ? (function_cmd[3] = true) : (function_cmd[3] = false);
  (rfresh == RefreshFunction::auto_refresh_on) ? (function_cmd[2] = true) : (function_cmd[2] = false);
  (espwm == PwmFunction::enhanced_pwm) ? (function_cmd[1] = true) : (function_cmd[1] = false);
  (lsdvlt == ShortDetectFunction::threshold_90_percent) ? (function_cmd[0] = true) : (function_cmd[0] = false);
  noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, function_cmd, m_func_cmd_offset);
}

void Driver::set_global_brightness_cmd(const uint8_t blue, const uint8_t green, const uint8_t red)
{
  const std::bitset<m_bc_data_size> blue_cmd{blue};
  const std::bitset<m_bc_data_size> green_cmd{green};
  const std::bitset<m_bc_data_size> red_cmd{red};

  noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, blue_cmd, m_bc_data_offset);
  noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, green_cmd, m_bc_data_offset + m_bc_data_size);
  noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, red_cmd, m_bc_data_offset + m_bc_data_size * 2);
}

void Driver::set_max_current_cmd(const uint8_t blue, const uint8_t green, const uint8_t red)
{
  const std::bitset<m_mc_data_size> blue_cmd{blue};
  const std::bitset<m_mc_data_size> green_cmd{green};
  const std::bitset<m_mc_data_size> red_cmd{red};

  noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, blue_cmd, m_mc_data_offset);
  noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, green_cmd, m_mc_data_offset + m_mc_data_size);
  noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, red_cmd, m_mc_data_offset + m_mc_data_size * 2);
}

void Driver::set_dot_correction_cmd_all(uint8_t pwm)
{
  const std::bitset<m_dc_data_size> dc_pwm_cmd{pwm};
  for (uint8_t dc_idx = 0; dc_idx < 48; dc_idx++)
  {
    noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, dc_pwm_cmd, m_dc_data_offset + m_dc_data_size * dc_idx);
  }
}

void Driver::set_greyscale_cmd_rgb(uint16_t blue_pwm, uint16_t green_pwm, uint16_t red_pwm)
{
  const std::bitset<m_gs_data_size> blue_gs_pwm_cmd{blue_pwm};
  const std::bitset<m_gs_data_size> green_gs_pwm_cmd{green_pwm};
  const std::bitset<m_gs_data_size> red_gs_pwm_cmd{red_pwm};
  for (uint16_t gs_idx = 0; gs_idx < m_num_leds_per_chip; gs_idx++)
  {
    noarch::bit_manip::insert_bitset_at_offset(
        m_common_bit_register, blue_gs_pwm_cmd, m_gs_data_offset + (m_gs_data_size * gs_idx * m_num_colour_chan));
    noarch::bit_manip::insert_bitset_at_offset(
        m_common_bit_register, green_gs_pwm_cmd, m_gs_data_offset + (m_gs_data_size * gs_idx * m_num_colour_chan) + m_gs_data_size);
    noarch::bit_manip::insert_bitset_at_offset(
        m_common_bit_register, red_gs_pwm_cmd, m_gs_data_offset + (m_gs_data_size * gs_idx * m_num_colour_chan) + (m_gs_data_size * 2));
  }
}

void Driver::set_greyscale_cmd_white(uint16_t pwm)
{
  const std::bitset<m_gs_data_size> gs_pwm_cmd{pwm};
  for (uint16_t gs_idx = 0; gs_idx < 48; gs_idx++)
  {
    noarch::bit_manip::insert_bitset_at_offset(m_common_bit_register, gs_pwm_cmd, m_gs_data_offset + m_gs_data_size * gs_idx);
  }
}

bool Driver::set_greyscale_cmd_rgb_at_position(uint16_t led_idx, uint16_t red_pwm, uint16_t green_pwm, uint16_t blue_pwm)
{
  // return if we overshot our max number of LEDs
  if (!(led_idx < tlc5955::Driver::m_num_leds_per_chip))
  {
    return false;
  }

  const std::bitset<m_gs_data_size> blue_gs_pwm_cmd{blue_pwm};
  const std::bitset<m_gs_data_size> green_gs_pwm_cmd{green_pwm};
  const std::bitset<m_gs_data_size> red_gs_pwm_cmd{red_pwm};

  noarch::bit_manip::insert_bitset_at_offset(
      m_common_bit_register, blue_gs_pwm_cmd, m_gs_data_offset + (m_gs_data_size * led_idx * m_num_colour_chan));
  noarch::bit_manip::insert_bitset_at_offset(
      m_common_bit_register, green_gs_pwm_cmd, m_gs_data_offset + (m_gs_data_size * led_idx * m_num_colour_chan) + m_gs_data_size);
  noarch::bit_manip::insert_bitset_at_offset(
      m_common_bit_register, red_gs_pwm_cmd, m_gs_data_offset + (m_gs_data_size * led_idx * m_num_colour_chan) + (m_gs_data_size * 2));
  return true;
}

void Driver::set_position_and_colour(uint16_t position, LedColour colour)
{
  uint16_t greyscale_pwm{0xFFFF};
  switch (colour)
  {
    case LedColour::red:
      set_greyscale_cmd_rgb_at_position(position, greyscale_pwm, 0, 0);
      break;
    case LedColour::green:
      set_greyscale_cmd_rgb_at_position(position, 0, greyscale_pwm, 0);
      break;
    case LedColour::blue:
      set_greyscale_cmd_rgb_at_position(position, 0, 0, greyscale_pwm);
      break;
    case LedColour::magenta:
      set_greyscale_cmd_rgb_at_position(position, greyscale_pwm, 0, greyscale_pwm);
      break;
    case LedColour::yellow:
      set_greyscale_cmd_rgb_at_position(position, greyscale_pwm, greyscale_pwm, 0);
      break;
    case LedColour::cyan:
      set_greyscale_cmd_rgb_at_position(position, 0, greyscale_pwm, greyscale_pwm);
      break;
    case LedColour::white:
      set_greyscale_cmd_rgb_at_position(position, greyscale_pwm, greyscale_pwm, greyscale_pwm);
      break;
  }
}

bool Driver::send_spi_bytes(LatchPinOption latch_option [[maybe_unused]])
{
#if not defined(X86_UNIT_TESTING_ONLY)
  // send the bytes
  for (auto &byte : m_common_byte_register)
  {
    // send the byte of data
    stm32::spi_ref::send_byte(m_serial_interface.get_spi_handle(), byte);
  }

  // tell each daisy-chained driver chip to latch all data from its common register
  if (latch_option == LatchPinOption::latch_after_send)
  {
    LL_GPIO_SetOutputPin(&m_serial_interface.get_lat_port(), m_serial_interface.get_lat_pin());
    LL_GPIO_ResetOutputPin(&m_serial_interface.get_lat_port(), m_serial_interface.get_lat_pin());
  }
#endif
  return true;
}

void Driver::gpio_init(void)
{
#if not defined(X86_UNIT_TESTING_ONLY)
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0, 0, 0, 0, 0, 0};

  // TLC5955_SPI2_MOSI
  // LL_GPIO_SetOutputPin(m_serial_interface.get_mosi_port(), m_serial_interface.get_mosi_pin());
  GPIO_InitStruct.Pin        = m_serial_interface.get_mosi_pin();
  GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull       = LL_GPIO_PULL_DOWN;
  LL_GPIO_Init(&m_serial_interface.get_mosi_port(), &GPIO_InitStruct);

  // TLC5955_SPI2_SCK
  // LL_GPIO_ResetOutputPin(m_serial_interface.get_sck_port(), m_serial_interface.get_sck_pin());
  GPIO_InitStruct.Pin        = m_serial_interface.get_sck_pin();
  GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull       = LL_GPIO_PULL_UP;
  LL_GPIO_Init(&m_serial_interface.get_sck_port(), &GPIO_InitStruct);
#endif // not X86_UNIT_TESTING_ONLY
}
void Driver::spi2_init(void)
{
#if not defined(X86_UNIT_TESTING_ONLY)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wvolatile"
  // Enable GPIO (SPI_MOSI)
  LL_GPIO_SetPinSpeed(&m_serial_interface.get_mosi_port(), m_serial_interface.get_mosi_pin(), LL_GPIO_SPEED_FREQ_VERY_HIGH);
  LL_GPIO_SetPinOutputType(&m_serial_interface.get_mosi_port(), m_serial_interface.get_mosi_pin(), LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinPull(&m_serial_interface.get_mosi_port(), m_serial_interface.get_mosi_pin(), LL_GPIO_PULL_DOWN);
  LL_GPIO_SetAFPin_0_7(&m_serial_interface.get_mosi_port(), m_serial_interface.get_mosi_pin(), LL_GPIO_AF_1);
  LL_GPIO_SetPinMode(&m_serial_interface.get_mosi_port(), m_serial_interface.get_mosi_pin(), LL_GPIO_MODE_ALTERNATE);

  // Enable GPIO (SPI_SCK)
  LL_GPIO_SetPinSpeed(&m_serial_interface.get_sck_port(), m_serial_interface.get_sck_pin(), LL_GPIO_SPEED_FREQ_VERY_HIGH);
  LL_GPIO_SetPinOutputType(&m_serial_interface.get_sck_port(), m_serial_interface.get_sck_pin(), LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinPull(&m_serial_interface.get_sck_port(), m_serial_interface.get_sck_pin(), LL_GPIO_PULL_DOWN);
  LL_GPIO_SetAFPin_8_15(&m_serial_interface.get_sck_port(), m_serial_interface.get_sck_pin(), LL_GPIO_AF_1);
  LL_GPIO_SetPinMode(&m_serial_interface.get_sck_port(), m_serial_interface.get_sck_pin(), LL_GPIO_MODE_ALTERNATE);

  m_serial_interface.get_spi_handle().CR1 = 0;
  m_serial_interface.get_spi_handle().CR1 |= ((SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE) | (SPI_CR1_MSTR | SPI_CR1_SSI) | SPI_CR1_SSM | SPI_CR1_BR_1);

  CLEAR_BIT(m_serial_interface.get_spi_handle().CR2, SPI_CR2_NSSP);

  // Enable the PWM OC channel
  m_serial_interface.get_gsclk_handle().CCER = m_serial_interface.get_gsclk_handle().CCER | m_serial_interface.get_gsclk_tim_ch();
  // required to enable output on some timers. e.g. TIM16
  m_serial_interface.get_gsclk_handle().BDTR = m_serial_interface.get_gsclk_handle().BDTR | TIM_BDTR_MOE;
  // Enable the timer
  m_serial_interface.get_gsclk_handle().CR1 = m_serial_interface.get_gsclk_handle().CR1 | TIM_CR1_CEN;

  #pragma GCC diagnostic pop // ignored "-Wvolatile"
#endif                       // not X86_UNIT_TESTING_ONLY
}

} // namespace tlc5955
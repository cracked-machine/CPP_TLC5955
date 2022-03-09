
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

#ifndef __TLC5955_DEVICE_HPP__
#define __TLC5955_DEVICE_HPP__


#include <stdint.h>
#include <bitset>
#include <array>
#include <memory>

#if defined(X86_UNIT_TESTING_ONLY)
	// only used when unit testing on x86
    #include <mock_cmsis.hpp>
	#include <iostream>
#else
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wvolatile"
		#include <stm32g0xx_ll_gpio.h>
	#pragma GCC diagnostic pop
    #include <timer_manager.hpp>
#endif

#include <bitset_utils.hpp>
#include <byte_utils.hpp>
// disable dynamic allocation/copying
#include <restricted_base.hpp>    

namespace tlc5955
{

// @brief contains pointer to SPI peripheral and associated GPIO ports/pins (as defined in CMSIS)
class DriverSerialInterface 
{
public:
    // @brief Construct a new Driver Serial Interface object
    // @param led_spi           The SPI peripheral e.g. SPI2
    // @param lat_port          The TLC5955 Latch port e.g. GPIOB
    // @param lat_pin           The TLC5955 Latch pin e.g. LL_GPIO_PIN_9
    // @param mosi_port         The TLC5955 MOSI port e.g. GPIOB
    // @param mosi_pin          The TLC5955 MOSI pin e.g. LL_GPIO_PIN_7
    // @param sck_port          The TLC5955 serial clock port e.g. GPIOB
    // @param sck_pin           The TLC5955 serial clock pin e.g. LL_GPIO_PIN_8
    // @param gsclk_tim         The TIM peripheral used to generate TLC5955 GSCLOCK pulse e.g. TIM4
    // @param gsclk_tim_ch      The timer channel used to output TLC5955 GSCLOCK pulse e.g. LL_TIM_CHANNEL_CH1
    // @param rcc_gpio_clk      The bit to enable the GPIO RCC (RCC_IOPENR) for the MOSI/SCK port e.g. LL_IOP_GRP1_PERIPH_GPIOB
    // @param rcc_spi_clk       The bit to enable the SPI RCC (RCC_APBENR1) for the MOSI/SCK port e.g. LL_APB1_GRP1_PERIPH_SPI2
	DriverSerialInterface(
        SPI_TypeDef *led_spi, 
        std::pair<GPIO_TypeDef*, uint16_t>  lat_gpio,    // GPIO_TypeDef* lat_port, uint16_t lat_pin, 
        std::pair<GPIO_TypeDef*, uint16_t>  mosi_gpio,   // GPIO_TypeDef* mosi_port, uint16_t mosi_pin,
        std::pair<GPIO_TypeDef*, uint16_t>  sck_gpio,    // GPIO_TypeDef* sck_port, uint16_t sck_pin,
        std::pair<TIM_TypeDef*, uint16_t>    gsclk_tim,
        uint32_t rcc_gpio_clk,
        uint32_t rcc_spi_clk
    )  
	: m_led_spi(led_spi), 
      m_lat_port(lat_gpio.first), m_lat_pin(lat_gpio.second),           // init latch port+pin
      m_mosi_port(mosi_gpio.first), m_mosi_pin(mosi_gpio.second),       // init mosi port+pin 
      m_sck_port(sck_gpio.first), m_sck_pin(sck_gpio.second),           // init sck port+pin
      m_gsclk_tim(gsclk_tim.first), m_gsclk_tim_ch(gsclk_tim.second),
      m_rcc_gpio_clk(rcc_gpio_clk), m_rcc_spi_clk(rcc_spi_clk)
	{
	}

	SPI_TypeDef * get_spi_handle() { return m_led_spi; }
	GPIO_TypeDef* get_lat_port() { return m_lat_port; }
	uint16_t get_lat_pin() { return m_lat_pin; }
	GPIO_TypeDef* get_mosi_port() { return m_mosi_port; }
	uint16_t get_mosi_pin() { return m_mosi_pin; }
	GPIO_TypeDef* get_sck_port() { return m_sck_port; }
	uint16_t get_sck_pin() { return m_sck_pin; }    
    TIM_TypeDef* get_gsclk_handle() { return m_gsclk_tim; }
    uint16_t get_gsclk_tim_ch() { return m_gsclk_tim_ch; }
    uint32_t get_rcc_gpio_clk() { return m_rcc_gpio_clk; }
    uint32_t get_rcc_spi_clk() { return m_rcc_spi_clk; }
private:
	// @brief The SPI peripheral
	SPI_TypeDef *m_led_spi;
	// @brief The latch GPIO port object
	GPIO_TypeDef* m_lat_port;
	// @brief The latch GPIO pin
	uint16_t m_lat_pin;
	// @brief The MOSI GPIO port object
	GPIO_TypeDef* m_mosi_port;
	// @brief The MOSI GPIO pin
	uint16_t m_mosi_pin;
    // @brief SPI Clock GPIO port
    GPIO_TypeDef* m_sck_port;   
    // @brief SPI Clock GPIO pin
    uint16_t m_sck_pin;    
    // @brief Timer used for GSCLK 
    TIM_TypeDef* m_gsclk_tim;
    // @brief output channel for GSCLK signal
    uint16_t m_gsclk_tim_ch;
    // @brief Used to enable the GPIO clock for MOSI and SCK pins (for writing first bit)
    uint32_t m_rcc_gpio_clk;
    // @brief Used to enable the SPI clock for MOSI and SCK pins (for writing 96 bytes data)
    uint32_t m_rcc_spi_clk;
};

} // namespace tlc5955

#endif // __TLC5955_DEVICE_HPP__
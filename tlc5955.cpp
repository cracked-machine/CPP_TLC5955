
#include "tlc5955.hpp"
#include <sstream>

#include <cmath>

namespace tlc5955 
{


void Driver::send_grayscale_data()
{
    // reset the latch
    HAL_GPIO_WritePin(m_lat_port, m_lat_pin, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(m_gsclk_port, m_gsclk_pin, GPIO_PIN_SET);

    // write the MSB bit low to signal GS data
    HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(m_mosi_port, m_mosi_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_RESET);
    
    // 16 bits per colour * 3 colours * 16 LEDS = 768 bits
    for (uint8_t idx = 0; idx < 16; idx++)
    {
        uint8_t grayscale_data_red[2] = {0xFF, 0x00};
        HAL_SPI_Transmit(&m_spi_interface, grayscale_data_red, 2, HAL_MAX_DELAY);
        uint8_t grayscale_data_green[2] = {0xFF, 0x00};
        HAL_SPI_Transmit(&m_spi_interface, grayscale_data_green, 2, HAL_MAX_DELAY);
        uint8_t grayscale_data_blue[2] = {0xFF, 0x00};
        HAL_SPI_Transmit(&m_spi_interface, grayscale_data_blue, 2, HAL_MAX_DELAY);
    }
        
        
    HAL_GPIO_WritePin(m_gsclk_port, m_gsclk_pin, GPIO_PIN_RESET);
    toggle_latch();

}

void Driver::send_control_data()
{
    // reset the latch
    HAL_GPIO_WritePin(m_lat_port, m_lat_pin, GPIO_PIN_RESET);

    // write the MSB bit high to signal control data
    HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(m_mosi_port, m_mosi_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_RESET);
    
    // send the write command - 0x96 or 10010110
    uint8_t write_cmd[2] = {0x9, 0x6};
    HAL_SPI_Transmit(&m_spi_interface, write_cmd, 1, HAL_MAX_DELAY);

    // manually write 388 bits empty padding between the 1 byte write command (767-760) and the control data (370-0)
    for (uint16_t idx = 0; idx < 388; idx++)
    {
        HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(m_mosi_port, m_mosi_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_RESET);
    }

    // manually write 5 bits for function data
    for (uint16_t idx = 0; idx < m_function_ctrl_reg_max; idx++)
    {
        HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(m_mosi_port, m_mosi_pin, (GPIO_PinState)m_function_data.test(idx));
        HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_RESET);
    }
    
    // manually write all three BC data - 3 x 7 = 21
    for (uint16_t bc_idx = 0; bc_idx < 3; bc_idx++)
    {
        for (uint16_t idx = 0; idx < m_bc_reg_max; idx++)
        {
            // write the max value for now...127
            HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(m_mosi_port, m_mosi_pin, (GPIO_PinState)m_bc_reg.test(idx));
            HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_RESET);
        }    
    }

    // manually write all three MC data - 3 x 3 = 9
    for (uint16_t mc_idx = 0; mc_idx < 3; mc_idx++)
    {
        for (uint16_t idx = 0; idx < m_max_current_reg_max; idx++)
        {
            // write some value for now...4
            HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(m_mosi_port, m_mosi_pin, (GPIO_PinState)m_mc_reg.test(idx));
            HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_RESET);
        }    
    }    

    // manually write dot correction data - 16 LEDS * 3 RGB * 7 bits = 336 bits
    for (uint16_t dc_idx = 0; dc_idx < m_num_leds_per_chip; dc_idx++)
    {  
        for (uint16_t rgb_idx = 0; rgb_idx < m_num_colour_chan; rgb_idx++)
        {   
            for (uint16_t idx = 0; idx < m_dc_reg_max; idx++)
            {
                HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(m_mosi_port, m_mosi_pin, (GPIO_PinState)m_dc_reg.test(idx));
                HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_SET);
                HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_RESET);
            }
        }
    }

    toggle_latch();
}





void Driver::set_function_data(bool DSPRPT, bool TMGRST, bool RFRESH, bool ESPWM, bool LSDVLT)
{
    m_function_data.set(0, DSPRPT);
    m_function_data.set(1, TMGRST);
    m_function_data.set(2, RFRESH);
    m_function_data.set(3, ESPWM);
    m_function_data.set(4, LSDVLT);
}



void Driver::toggle_latch()
{
    HAL_Delay(m_latch_delay_ms);
    HAL_GPIO_WritePin(m_lat_port, m_lat_pin, GPIO_PIN_SET);
    HAL_Delay(m_latch_delay_ms);
    HAL_GPIO_WritePin(m_lat_port, m_lat_pin, GPIO_PIN_RESET);
    HAL_Delay(m_latch_delay_ms);
}






} // namespace tlc5955
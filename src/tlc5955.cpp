
#include "tlc5955.hpp"
#include <sstream>
#include <cmath>
#include <cstring>
#include <byte_position.hpp>

#ifdef USE_RTT
    #include <SEGGER_RTT.h>
#endif



namespace tlc5955 
{



bool Driver::start_dma_transmit()
{
    #ifdef USE_HAL_DRIVER 
        return HAL_SPI_Transmit_DMA(&m_spi_interface, m_common_byte_register.data(), m_common_byte_register.size());      
    #else
        // we don't care about SPI for x86-based unit testing
        return true;
    #endif
}
    
bool Driver::pause_dma_transmit(bool pause)
{
    #ifdef USE_HAL_DRIVER 
        // get the HAL result
        volatile HAL_StatusTypeDef res {HAL_OK};
        if (pause) { res = HAL_SPI_DMAPause(&m_spi_interface);  }
        else {  res = HAL_SPI_DMAResume(&m_spi_interface); }        
        // return it as bool result
        if(res == HAL_OK) { return true; }
        else { return false; }        
    #else
        // we don't care about SPI for x86-based unit testing
        UNUSED(pause);
        return true;
    #endif
}



void Driver::flush_common_register()
{
    for (auto &byte : m_common_byte_register)
    {
        byte = 0x00;
    }
    // send_data();
}

void Driver::print_common_bits()
{
#ifdef USE_RTT
    SEGGER_RTT_printf(0, "\r\n");
    for (uint16_t idx = 0; idx < 8; idx++)
    {
        SEGGER_RTT_printf(0, "%u ", +m_common_byte_register[idx]);
    }
#endif

}


} // namespace tlc5955

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

void Driver::set_value_nth_bit(uint8_t &byte, uint16_t bit, bool new_value)
{
    if (new_value) { byte |= (1U << bit); }
    else { byte &= ~(1U << bit); }
    print_common_bits(); 
}


void Driver::set_control_bit(bool ctrl_latch)
{
    // Latch       
    // bits      =
    // Bytes     [
    //          #0  
    UNUSED(ctrl_latch);
    set_value_nth_bit(m_common_byte_register[byte_offsets::latch], 7, ctrl_latch);
    //m_common_byte_register[byte_offsets::latch] = 0xFF;
}

void Driver::set_ctrl_cmd_bits()
{

    // Ctrl      10010110  
    // bits      [======]
    // Bytes     ======][
    //             #0    #1
    
    // BYTE #0
    BytePosition byte_pos(byte_offsets::ctrl_cmd);

    // skip the MSB of byte #0
    byte_pos.skip_next_n_bits(1);
    
    // 7 MSB bits of ctrl byte into 7 LSB of byte #0
    for (int8_t idx = m_ctrl_cmd_size_bits - 1; idx > 0; idx--)
    {
        uint8_t bit_idx = byte_pos.next_bit_idx();
        set_value_nth_bit(m_common_byte_register[byte_pos.get_byte_idx()], bit_idx, m_ctrl_cmd.test(idx));           
    }

    // the last m_ctrl_cmd bit in to MSB of byte #1
    uint8_t bit_idx = byte_pos.next_bit_idx();
    set_value_nth_bit(m_common_byte_register[byte_pos.get_byte_idx()], bit_idx, m_ctrl_cmd.test(0));
    
}

void Driver::set_padding_bits()
{

    // Padding  0  ================================================================================ 79
    // Bytes       ======][======][======][======][======][======][======][======][======][======][
    //               #1      #2       #3    #4      #5      #6      #7      #8      #9     #10

    // Padding 80  ================================================================================ 159
    // Bytes       ======][======][======][======][======][======][======][======][======][======][
    //              #11     #12     #13     #14     #15     #16     #17     #18     #19     #20

    // Padding 160 ================================================================================ 239
    // Bytes       ======][======][======][======][======][======][======][======][======][======][
    //              #21     #22     #23     #24     #25     #26     #27     #28     #29     #30

    // Padding 240 ================================================================================ 319
    // Bytes       ======][======][======][======][======][======][======][======][======][======][
    //              #31     #32     #33     #34     #35     #36     #37     #38     #39     #40

    // Padding 320 ===================================================================== 389
    // Bytes       ======][======][======][======][======][======][======][======][=====
    //              #41     #42     #43     #44     #45     #46     #47     #48     #49

    // BYTE #1
    BytePosition byte_pos(byte_offsets::padding);
 
    // skip MSB of byte #1
    byte_pos.skip_next_n_bits(1);
    

    // then write next 389 bits of padding
    for (uint16_t idx = 0; idx < m_padding_section_size_bits; idx++)
    {   
        uint8_t bit_idx = byte_pos.next_bit_idx();
        // if (bit_idx == 0)
        // {
        //     set_value_nth_bit(m_common_byte_register[byte_pos.get_byte_idx()], bit_idx, true);  
        // }
        // else
        {
            set_value_nth_bit(m_common_byte_register[byte_pos.get_byte_idx()], bit_idx, false);  
        }
        
    }



}


void Driver::set_function_data(bool DSPRPT, bool TMGRST, bool RFRESH, bool ESPWM, bool LSDVLT)
{ 

    // Function   
    // bits      [===]
    //           =][==
    // Bytes   #49  #50
    enum functions {
        tmgrst  = 0,
        dsprt   = 1,
        lsdvlt  = 5,
        espwm   = 6,
        rfresh  = 7
    };

    // if all are set to true, byte #49 = 3, byte #50 = 224
    set_value_nth_bit(m_common_byte_register[byte_offsets::function],       functions::dsprt,  DSPRPT);
    set_value_nth_bit(m_common_byte_register[byte_offsets::function],       functions::tmgrst, TMGRST);
    set_value_nth_bit(m_common_byte_register[byte_offsets::function + 1],   functions::rfresh, RFRESH);
    set_value_nth_bit(m_common_byte_register[byte_offsets::function + 1],   functions::espwm,  ESPWM);
    set_value_nth_bit(m_common_byte_register[byte_offsets::function + 1],   functions::lsdvlt, LSDVLT);
}

void Driver::set_bc_data(
    const std::bitset<m_bc_data_resolution> &blue_value, 
    const std::bitset<m_bc_data_resolution> &green_value, 
    const std::bitset<m_bc_data_resolution> &red_value)
{
    // BC         blue   green   red
    // bits      [=====][=====][=====]
    // bits   xxx====][======][======]
    // Bytes     #50    #51      #52
    
    // BYTE #50
    BytePosition byte_pos(byte_offsets::brightness_control);

    byte_pos.skip_next_n_bits(3);

    // set 5 LSB of byte #50 to bits 6-2 of BC blue_value
    for (int8_t blue_idx = 6; blue_idx > -1; blue_idx--)
    {
        uint8_t bit_idx = byte_pos.next_bit_idx();
        set_value_nth_bit(m_common_byte_register[byte_pos.get_byte_idx()], bit_idx, blue_value.test(blue_idx));
    }

    // set 5 LSB of byte #51 to bits 6-1 of BC green_value
    for (int8_t green_idx = 6; green_idx > -1; green_idx--)
    {
        uint8_t bit_idx = byte_pos.next_bit_idx();
        set_value_nth_bit(m_common_byte_register[byte_pos.get_byte_idx()], bit_idx, green_value.test(green_idx));
    }    

    // set 7 LSB of byte #52 to bits all 7 bits of BC red_value
    for (int8_t red_idx = 6; red_idx > -1; red_idx--)
    {
        uint8_t bit_idx = byte_pos.next_bit_idx();
        set_value_nth_bit(m_common_byte_register[byte_pos.get_byte_idx()], bit_idx, red_value.test(red_idx));
    }   

    
}

void Driver::set_mc_data(
    const std::bitset<m_mc_data_resolution> &blue_value, 
    const std::bitset<m_mc_data_resolution> green_value, 
    const std::bitset<m_mc_data_resolution> &red_value)
{
    // MC         B  G  R
    // bits      [=][=][=]
    // bits      [======][
    // Bytes       #53    #54

    // BYTE #53
    BytePosition byte_pos(byte_offsets::max_current);

    // 3 bits of blue in 3 MSB of byte #51 == 128
    for (int8_t blue_idx = 2; blue_idx > -1; blue_idx--)
    {
        uint8_t bit_idx = byte_pos.next_bit_idx();
        set_value_nth_bit(m_common_byte_register[byte_pos.get_byte_idx()], bit_idx, blue_value.test(blue_idx));
    }
    
    // 3 bits of green in next 3 bits of byte #51 == 144
    for (int8_t green_idx = 2; green_idx > -1; green_idx--)
    {
        uint8_t bit_idx = byte_pos.next_bit_idx();
        set_value_nth_bit(m_common_byte_register[byte_pos.get_byte_idx()], bit_idx, green_value.test(green_idx));
    }    

    // 2 bits of red in 2 LSB of byte #51 (== 146) 
    for (int8_t red_idx = 2; red_idx > -1; red_idx--)
    {
        uint8_t bit_idx = byte_pos.next_bit_idx();
        set_value_nth_bit(m_common_byte_register[byte_pos.get_byte_idx()], bit_idx, red_value.test(red_idx));
    }     

}

bool Driver::set_dc_data(
    const uint8_t led_idx, 
    const std::bitset<m_dc_data_resolution> &blue_value, 
    const std::bitset<m_dc_data_resolution> &green_value, 
    const std::bitset<m_dc_data_resolution> &red_value)
{

    // The switch cases are arranged in descending byte order: 15 -> 0.
    // Because the tlc5955 common register overlaps byte boundaries of the buffer all loops are unrolled.
    // This looks a bit nuts but it makes it easier to read and debug rather than a series of disjointed loops.

    // Common Register-to-Byte array mapping for DC (dot correction) data
    
    // ROW #1
    // DC        B15    G15    R15    B14    G14    R14    B13    G13    R13    B12    G12    R12
    // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
    // Bytes   ======][======][======][======][======][======][======][======][======][======][====
    //          #54     #55     #56     #57      #58     #59    #60     #61     #62     #63     #64

    // ROW #2
    // DC        B11    G11    R11    B10    G10    R10    B9     G9     R9     B8     G8     R8
    // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
    // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
    //        #64   #65      #66    #67     #68     #69     #70     #71     #72     #73     #74

    // ROW #3
    // DC        B7      G7     R7    B6     G6     R6     B5     G5     R5     B4     G4     R4
    // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
    // Bytes   ======][======][======][======][======][======][======][======][======][======][====
    //          #75     #76     #77     #78     #79     #80     #81     #82     #83     #84     #85

    // ROW #4
    // DC        B3     G3     R3      B2     G2    R2      B1    G1     R1     B0     G0     R0
    // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
    // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
    //        #85   #86     #87      #88    #89     #90     #91     #92     #93     #94     #95   #96    

    BytePosition byte_pos(byte_offsets::dot_correct);
    
    switch(led_idx)
    {
        case 15:    // LED #15

            // ROW #1
            // DC        B15    G15    R15   
            // bits    [=====][=====][=====]
            // Bytes   ======][======][======]
            //          #54     #55     #56   

            // LED B15 
            byte_pos.set_byte_idx(byte_offsets::dot_correct); // BYTE #54
            byte_pos.next_bit_idx();    // skip MSB
            break;

        case 14:    // LED #14

            // ROW #1
            // DC        B14    G14    R14  
            // bits    [=====][=====][=====]
            // Bytes   =][======][======][==
            //       #56   #57     #58    #59 

            // LED B14
            byte_pos.set_byte_idx(byte_offsets::dot_correct + 2); // BYTE #56
            byte_pos.skip_next_n_bits(6);
            break;

        case 13:    // LED #13

            // ROW #1
            // DC        B15    G15    R15    B14    G14    R14    B13    G13    R13    B12    G12    R12
            // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
            // Bytes   ======][======][======][======][======][======][======][======][======][======][====
            //          #54     #55     #56     #57      #58     #59    #60     #61     #62     #63     #64

            // LED B13
            byte_pos.set_byte_idx(byte_offsets::dot_correct + 5); // BYTE #59
            byte_pos.skip_next_n_bits(3);    
            break;

        case 12:    // LED #12

            // ROW #1
            // DC        B15    G15    R15    B14    G14    R14    B13    G13    R13    B12    G12    R12
            // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
            // Bytes   ======][======][======][======][======][======][======][======][======][======][====
            //          #54     #55     #56     #57      #58     #59    #60     #61     #62     #63     #64

            // LED B12
            byte_pos.set_byte_idx(byte_offsets::dot_correct + 8); // BYTE #62
            break;

        case 11:    // LED #11

            // ROW #2
            // DC        B11    G11    R11    B10    G10    R10    B9     G9     R9     B8     G8     R8
            // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
            // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
            //        #64   #65      #66    #67     #68     #69     #70     #71     #72     #73     #74

            // LED B11
            byte_pos.set_byte_idx(byte_offsets::dot_correct + 10);  // BYTE #64
            byte_pos.skip_next_n_bits(5);
            break;

        case 10:    // LED #10

            // ROW #2
            // DC        B11    G11    R11    B10    G10    R10    B9     G9     R9     B8     G8     R8
            // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
            // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
            //        #64   #65      #66    #67     #68     #69     #70     #71     #72     #73     #74

            // LED B10
            byte_pos.set_byte_idx(byte_offsets::dot_correct + 13);  // BYTE #67
            byte_pos.skip_next_n_bits(2);
            break;

        case 9: // LED #9

            // ROW #2
            // DC        B11    G11    R11    B10    G10    R10    B9     G9     R9     B8     G8     R8
            // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
            // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
            //        #64   #65      #66    #67     #68     #69     #70     #71     #72     #73     #74

            // LED B9
            byte_pos.set_byte_idx(byte_offsets::dot_correct + 15);  // BYTE #69
            byte_pos.skip_next_n_bits(7);
            break;  

        case 8:     // LED #8

            // ROW #2
            // DC        B11    G11    R11    B10    G10    R10    B9     G9     R9     B8     G8     R8
            // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
            // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
            //        #64   #65      #66    #67     #68     #69     #70     #71     #72     #73     #74

            // LED B8
            byte_pos.set_byte_idx(byte_offsets::dot_correct + 18);  // BYTE #72
            byte_pos.skip_next_n_bits(4);
            break;   

        case 7: // LED #7

            // ROW #3
            // DC        B7      G7     R7    B6     G6     R6     B5     G5     R5     B4     G4     R4
            // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
            // Bytes   ======][======][======][======][======][======][======][======][======][======][====
            //          #75     #76     #77     #78     #79     #80     #81     #82     #83     #84     #85

            // LED B7
            byte_pos.set_byte_idx(byte_offsets::dot_correct + 21); // BYTE #75
            byte_pos.skip_next_n_bits(1);
            break;       

        case 6:     // LED #6

            // ROW #3   
            // DC        B7      G7     R7    B6     G6     R6     B5     G5     R5     B4     G4     R4
            // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
            // Bytes   ======][======][======][======][======][======][======][======][======][======][====
            //          #75     #76     #77     #78     #79     #80     #81     #82     #83     #84     #85

            // LED B6
            byte_pos.set_byte_idx(byte_offsets::dot_correct + 23);  // BYTE #77
            byte_pos.skip_next_n_bits(6);
            break;

        case 5: // LED #5

            // ROW #3   
            // DC        B7      G7     R7    B6     G6     R6     B5     G5     R5     B4     G4     R4
            // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
            // Bytes   ======][======][======][======][======][======][======][======][======][======][====
            //          #75     #76     #77     #78     #79     #80     #81     #82     #83     #84     #85

            // LED B5
            byte_pos.set_byte_idx(byte_offsets::dot_correct + 26);  // BYTE #80
            byte_pos.skip_next_n_bits(3);            
            break;

        case 4:

            // ROW #3   
            // DC        B7      G7     R7    B6     G6     R6     B5     G5     R5     B4     G4     R4
            // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
            // Bytes   ======][======][======][======][======][======][======][======][======][======][====
            //          #75     #76     #77     #78     #79     #80     #81     #82     #83     #84     #85

            // LED B4
            byte_pos.set_byte_idx(byte_offsets::dot_correct + 29); //BYTE #83
            break;

        case 3:

            // ROW #4
            // DC        B3     G3     R3      B2     G2    R2      B1    G1     R1     B0     G0     R0
            // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
            // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
            //        #85   #86     #87      #88    #89     #90     #91     #92     #93     #94     #95   #96    

            // LED B3
            byte_pos.set_byte_idx(byte_offsets::dot_correct + 31);   // BYTE #85
            byte_pos.skip_next_n_bits(5);
            break;

        case 2:

            // ROW #4
            // DC        B3     G3     R3      B2     G2    R2      B1    G1     R1     B0     G0     R0
            // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
            // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
            //        #85   #86     #87      #88    #89     #90     #91     #92     #93     #94     #95   #96    

            // LED B2
            byte_pos.set_byte_idx(byte_offsets::dot_correct + 34);  // BYTE #88
            byte_pos.skip_next_n_bits(2);
            break;

        case 1:

            // ROW #4
            // DC        B3     G3     R3      B2     G2    R2      B1    G1     R1     B0     G0     R0
            // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
            // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
            //        #85   #86     #87      #88    #89     #90     #91     #92     #93     #94     #95   #96    

            // LED B1
            byte_pos.set_byte_idx(byte_offsets::dot_correct + 36);  // BYTE #90
            byte_pos.skip_next_n_bits(7);
            break;

        case 0:

            // ROW #4
            // DC        B3     G3     R3      B2     G2    R2      B1    G1     R1     B0     G0     R0
            // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
            // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
            //        #85   #86     #87      #88    #89     #90     #91     #92     #93     #94     #95   #96    

            // LED B0
            byte_pos.set_byte_idx(byte_offsets::dot_correct + 39);  // BYTE #93
            byte_pos.skip_next_n_bits(4);
            break;

        default:    // led_idx > 15
            return false;
    }

    // set the bits
    for (int8_t blue_idx = 6; blue_idx > -1; blue_idx--)
    {
        uint8_t bit_idx = byte_pos.next_bit_idx();
        set_value_nth_bit(m_common_byte_register[byte_pos.get_byte_idx()], bit_idx, blue_value.test(blue_idx));
    }

    // LED G15  
    for (int8_t green_idx = 6; green_idx > -1; green_idx--)        
    {
        uint8_t bit_idx = byte_pos.next_bit_idx();
        set_value_nth_bit(m_common_byte_register[byte_pos.get_byte_idx()], bit_idx, green_value.test(green_idx));
    }

    // LED R15
    for (int8_t red_idx = 6; red_idx > -1; red_idx--)        
    {            
        uint8_t bit_idx = byte_pos.next_bit_idx();
        set_value_nth_bit(m_common_byte_register[byte_pos.get_byte_idx()], bit_idx, red_value.test(red_idx));
    }

    return true;

}

void Driver::set_all_dc_data(
    const std::bitset<m_dc_data_resolution> &blue_value, 
    const std::bitset<m_dc_data_resolution> &green_value, 
    const std::bitset<m_dc_data_resolution> &red_value)
{
    for (uint8_t led_idx = 0; led_idx < m_num_leds_per_chip; led_idx++)
    {
        set_dc_data(led_idx, blue_value, green_value, red_value);
    }
}

bool Driver::set_gs_data(
    uint8_t led_idx, 
    const std::bitset<m_gs_data_resolution> &blue_value, 
    const std::bitset<m_gs_data_resolution> &green_value, 
    const std::bitset<m_gs_data_resolution> &red_value)
{
    if (led_idx >= m_num_leds_per_chip)
    {
        return false;
    }
    // offset for the current LED position
    const uint16_t led_offset = m_gs_data_one_led_size_bits * led_idx;

    // the current bit position within the GS section of the common register, starting at the section offset + LED offset
    uint16_t gs_common_pos = 1U + led_offset;

    // check gs_common_pos has left enough bits for one segment of LED GS data
    // This could happen if the header constants are incorrect
    if (gs_common_pos + m_gs_data_one_led_size_bits > m_common_reg_size_bits)
    {
        return false;
    }

        // ROW #1
        // GS            Bn             Gn             Rn             
        // bits    0[==============][==============][==============]
        // Bytes   [======][======][======][======][======][======][
        //            #0      #1      #2      #3      #4      #5    #6


    // set the bits
    
    // should always give multiple of 6.
    uint16_t begin_byte_idx = gs_common_pos / 8;
    
    BytePosition byte_pos(begin_byte_idx);

    // byte #0, skip the MSB 
    byte_pos.skip_next_n_bits(1);

    for (int8_t blue_idx = 15; blue_idx > -1; blue_idx--)
    {
        uint8_t bit_idx = byte_pos.next_bit_idx();
        set_value_nth_bit(m_common_byte_register[byte_pos.get_byte_idx()], bit_idx, blue_value.test(blue_idx));
    }
    for (int8_t green_idx = 15; green_idx > -1; green_idx--)
    {
        uint8_t bit_idx = byte_pos.next_bit_idx();
        set_value_nth_bit(m_common_byte_register[byte_pos.get_byte_idx()], bit_idx, green_value.test(green_idx));
    }
    for (int8_t red_idx = 15; red_idx > -1; red_idx--)
    {
        uint8_t bit_idx = byte_pos.next_bit_idx();
        set_value_nth_bit(m_common_byte_register[byte_pos.get_byte_idx()], bit_idx, red_value.test(red_idx));
    }        

    return true;
}

void Driver::set_all_gs_data(
    const std::bitset<m_gs_data_resolution> &blue_value, 
    const std::bitset<m_gs_data_resolution> &green_value, 
    const std::bitset<m_gs_data_resolution> &red_value)
{
    for (uint8_t led_idx = 0; led_idx < m_num_leds_per_chip; led_idx++)
    {
        set_gs_data(led_idx, blue_value, green_value, red_value);
    }
}



void Driver::send_data()
{
    // enable gsclk 
    HAL_GPIO_WritePin(m_gsclk_port, m_gsclk_pin, GPIO_PIN_SET);

    // clock the data through and latch
#ifdef USE_HAL_DRIVER
//    HAL_GPIO_WritePin(m_gsclk_port, m_gsclk_pin, GPIO_PIN_SET);
    HAL_StatusTypeDef res = HAL_SPI_Transmit(&m_spi_interface, (uint8_t*)m_common_byte_register.data(), m_common_reg_size_bytes, 0x0000'000F);
    UNUSED(res);
 //   HAL_GPIO_WritePin(m_gsclk_port, m_gsclk_pin, GPIO_PIN_RESET);

    toggle_latch();
#endif
}

void Driver::toggle_latch()
{
#ifdef USE_HAL_DRIVER
    // disable the gsclk
    HAL_GPIO_WritePin(m_gsclk_port, m_gsclk_pin, GPIO_PIN_RESET);   
    
    // toggle the latch pin
//    HAL_Delay(m_latch_delay_ms);
    HAL_GPIO_WritePin(m_lat_port, m_lat_pin, GPIO_PIN_SET);
//    HAL_Delay(m_latch_delay_ms);
    HAL_GPIO_WritePin(m_lat_port, m_lat_pin, GPIO_PIN_RESET);
//    HAL_Delay(m_latch_delay_ms);



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

// void Driver::flush_common_register()
// {
//     // reset the latch
//     HAL_GPIO_WritePin(m_lat_port, m_lat_pin, GPIO_PIN_RESET);

//     // clock-in the entire common shift register per daisy-chained chip before pulsing the latch
//     for (uint8_t shift_entire_reg = 0; shift_entire_reg < m_num_driver_ics; shift_entire_reg++)
//     {
//         // write the MSB bit low to signal greyscale data
//         HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_RESET);
//         HAL_GPIO_WritePin(m_mosi_port, m_mosi_pin, GPIO_PIN_RESET);
//         HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_SET);
//         HAL_GPIO_WritePin(m_sck_port, m_sck_pin, GPIO_PIN_RESET);

//         // Set all 16-bit colours to 0 greyscale
//         uint8_t grayscale_data[2] = {0x00, 0x00};
//         for (uint8_t idx = 0; idx < 16; idx++)
//         {
//             HAL_SPI_Transmit(&m_spi_interface, grayscale_data, 2, HAL_MAX_DELAY);
//             HAL_SPI_Transmit(&m_spi_interface, grayscale_data, 2, HAL_MAX_DELAY);
//             HAL_SPI_Transmit(&m_spi_interface, grayscale_data, 2, HAL_MAX_DELAY);
//         }
//     }

//     toggle_latch();
// }

// void Driver::enable_spi()
// {
//     HAL_GPIO_DeInit(GPIOB, TLC5955_SPI2_MOSI_Pin|TLC5955_SPI2_SCK_Pin);

//     m_spi_interface.Instance = SPI2;
//     m_spi_interface.Init.Mode = SPI_MODE_MASTER;
//     m_spi_interface.Init.Direction = SPI_DIRECTION_1LINE;
//     m_spi_interface.Init.DataSize = SPI_DATASIZE_8BIT;
//     m_spi_interface.Init.CLKPolarity = SPI_POLARITY_LOW;
//     m_spi_interface.Init.CLKPhase = SPI_PHASE_1EDGE;
//     m_spi_interface.Init.NSS = SPI_NSS_SOFT;
//     m_spi_interface.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
//     m_spi_interface.Init.FirstBit = SPI_FIRSTBIT_MSB;
//     m_spi_interface.Init.TIMode = SPI_TIMODE_DISABLE;
//     m_spi_interface.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//     m_spi_interface.Init.CRCPolynomial = 7;
//     m_spi_interface.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
//     m_spi_interface.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

//     if (HAL_SPI_Init(&m_spi_interface) != HAL_OK) { Error_Handler(); }
  
//     __HAL_RCC_SPI2_CLK_ENABLE();
//     __HAL_RCC_GPIOB_CLK_ENABLE();

//     GPIO_InitTypeDef GPIO_InitStruct = {
//         TLC5955_SPI2_MOSI_Pin|TLC5955_SPI2_SCK_Pin,
//         GPIO_MODE_AF_PP,
//         GPIO_PULLDOWN,
//         GPIO_SPEED_FREQ_VERY_HIGH,
//         GPIO_AF1_SPI2,
//     };
    
//     HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//     __HAL_SYSCFG_FASTMODEPLUS_ENABLE(SYSCFG_FASTMODEPLUS_PB8);

// }

// void Driver::disable_spi()
// {
 
// }

// void Driver::enable_gpio_output_only()
// {
//     // disable SPI config
//     __HAL_RCC_SPI2_CLK_DISABLE();
//     HAL_GPIO_DeInit(GPIOB, TLC5955_SPI2_MOSI_Pin|TLC5955_SPI2_SCK_Pin);

//     // GPIO Ports Clock Enable 
//     __HAL_RCC_GPIOB_CLK_ENABLE();

//     // Configure GPIO pin Output Level
//     HAL_GPIO_WritePin(GPIOB, TLC5955_SPI2_LAT_Pin|TLC5955_SPI2_GSCLK_Pin|TLC5955_SPI2_MOSI_Pin|TLC5955_SPI2_SCK_Pin, GPIO_PIN_RESET);

//     // Configure GPIO pins
//     GPIO_InitTypeDef GPIO_InitStruct = {
//         TLC5955_SPI2_LAT_Pin|TLC5955_SPI2_GSCLK_Pin|TLC5955_SPI2_MOSI_Pin|TLC5955_SPI2_SCK_Pin,
//         GPIO_MODE_OUTPUT_PP,
//         GPIO_PULLDOWN,
//         GPIO_SPEED_FREQ_VERY_HIGH,
//         0
//     };

//     HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//     __HAL_SYSCFG_FASTMODEPLUS_ENABLE(SYSCFG_FASTMODEPLUS_PB9);
//     __HAL_SYSCFG_FASTMODEPLUS_ENABLE(SYSCFG_FASTMODEPLUS_PB6);
//     __HAL_SYSCFG_FASTMODEPLUS_ENABLE(SYSCFG_FASTMODEPLUS_PB7);
//     __HAL_SYSCFG_FASTMODEPLUS_ENABLE(SYSCFG_FASTMODEPLUS_PB8);

// }

} // namespace tlc5955
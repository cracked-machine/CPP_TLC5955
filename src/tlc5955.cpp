
#include "tlc5955.hpp"
#include <sstream>
#include <cmath>
#include <cstring>
#ifdef USE_RTT
    #include <SEGGER_RTT.h>
#endif
namespace tlc5955 
{


void Driver::set_value_nth_bit(uint8_t &target, uint16_t target_idx, bool value)
{
    if (value) { target |= (1U << target_idx); }
    else { target &= ~(1U << target_idx); }
    print_common_bits(); 
}


void Driver::set_control_bit(bool ctrl_latch)
{
    // Latch       
    // bits      =
    // Bytes     [
    //          #0  

    //m_common_bit_register.set(m_latch_offset, ctrl_latch);
    set_value_nth_bit(m_common_byte_register[0], 7, ctrl_latch);
}

void Driver::set_ctrl_cmd_bits()
{

    // Ctrl      10010110  
    // bits      [======]
    // Bytes     ======][
    //             #0    #1
   
    // 7 MSB bits of ctrl byte into 7 LSB of byte #0
    for (int8_t idx = m_ctrl_cmd_size_bits - 1; idx > 0; idx--)
    {
        set_value_nth_bit(m_common_byte_register[0], idx -1 , m_ctrl_cmd.test(idx));  
         
    }

    // the last m_ctrl_cmd bit in to MSB of byte #1
    set_value_nth_bit(m_common_byte_register[1], 7, m_ctrl_cmd.test(0));
    
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

    // first, we write 7 LSB bits of m_common_byte_register[1] = 0
    for (int8_t idx = 6; idx > -1; idx--)
    {
        set_value_nth_bit(m_common_byte_register[1], idx, false);  
    }

    // The next 47 bytes are don't care padding = 0
    const uint16_t padding_bytes_remaining = 47U;
    for (uint16_t byte_idx  = 2; byte_idx < padding_bytes_remaining; byte_idx++)
    {
        for (int8_t bit_idx = 7; bit_idx > -1; bit_idx--)
        {
            set_value_nth_bit(m_common_byte_register[byte_idx], bit_idx, false);
        }
    }

    // lastly, we write 6 MSB bits of m_common_byte_register[49] = 0
    for (int8_t idx = 7; idx > 1; idx--)
    {
        set_value_nth_bit(m_common_byte_register[49], idx, false);    
    }

}


void Driver::set_function_data(bool DSPRPT, bool TMGRST, bool RFRESH, bool ESPWM, bool LSDVLT)
{ 

    // Function   
    // bits      [===]
    //           =][==
    // Bytes   #49  #50

    // if all are set to true, byte #49 = 3, byte #50 = 224
    set_value_nth_bit(m_common_byte_register[49], 1, DSPRPT);
    set_value_nth_bit(m_common_byte_register[49], 0, TMGRST);
    set_value_nth_bit(m_common_byte_register[50], 7, RFRESH);
    set_value_nth_bit(m_common_byte_register[50], 6, ESPWM);
    set_value_nth_bit(m_common_byte_register[50], 5, LSDVLT);
}

void Driver::set_bc_data(
    std::bitset<m_bc_data_resolution> &blue_value, 
    std::bitset<m_bc_data_resolution> &green_value, 
    std::bitset<m_bc_data_resolution> &red_value)
{
    // BC         blue   green   red
    // bits      [=====][=====][=====]
    // bits      ====][======][======]
    // Bytes     #50    #51      #52

    // set 5 LSB of byte #50 to bits 6-2 of BC blue_value
    for (int8_t bit_idx = m_bc_data_resolution - 1; bit_idx > 1; bit_idx--)
    {
        // offset the bit position in byte #50 by 2 places.
        set_value_nth_bit(m_common_byte_register[50], bit_idx - 2, blue_value.test(bit_idx));
    }

    // set the first 2 MSB bits of byte #51 to the last 2 LSB of blue_value
    set_value_nth_bit(m_common_byte_register[51], 7, blue_value.test(1));
    set_value_nth_bit(m_common_byte_register[51], 6, blue_value.test(0));

    // set 5 LSB of byte #51 to bits 6-1 of BC green_value
    for (int8_t bit_idx = m_bc_data_resolution - 1; bit_idx > 0; bit_idx--)
    {
        // offset the bit position in byte #51 by 1 places.
        set_value_nth_bit(m_common_byte_register[51], bit_idx - 1, green_value.test(bit_idx));
    }

    // set MSB of byte#52 to LSB of green_value
    set_value_nth_bit(m_common_byte_register[52], 7, green_value.test(0));

    // set 7 LSB of byte #50 to bits all 7 bits of BC red_value
    for (int8_t bit_idx = m_bc_data_resolution - 1; bit_idx > -1; bit_idx--)
    {
        // No offset for bit position in byte #52. 
        set_value_nth_bit(m_common_byte_register[52], bit_idx, red_value.test(bit_idx));
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

    // 3 bits of blue in 3 MSB of byte #51 == 128
    set_value_nth_bit(m_common_byte_register[53], 7, blue_value.test(m_mc_data_resolution - 1));
    set_value_nth_bit(m_common_byte_register[53], 6, blue_value.test(m_mc_data_resolution - 2));
    set_value_nth_bit(m_common_byte_register[53], 5, blue_value.test(m_mc_data_resolution - 3));

    // 3 bits of green in next 3 bits of byte #51 == 144
    set_value_nth_bit(m_common_byte_register[53], 4, green_value.test(m_mc_data_resolution - 1));
    set_value_nth_bit(m_common_byte_register[53], 3, green_value.test(m_mc_data_resolution - 2));
    set_value_nth_bit(m_common_byte_register[53], 2, green_value.test(m_mc_data_resolution - 3));

    // 3 bits of red in 2 LSB of byte #51 (== 146) and MSB of byte #52 (== 0)
    set_value_nth_bit(m_common_byte_register[53], 1, red_value.test(m_mc_data_resolution - 1));
    set_value_nth_bit(m_common_byte_register[53], 0, red_value.test(m_mc_data_resolution - 2));
    set_value_nth_bit(m_common_byte_register[54], 7, red_value.test(m_mc_data_resolution - 3));

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


    uint8_t byte_idx{0};
    
    switch(led_idx)
    {
        case 15:    // LED #15

        // ROW #1
        // DC        B15    G15    R15    B14    G14    R14    B13    G13    R13    B12    G12    R12
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ======][======][======][======][======][======][======][======][======][======][====
        //          #54     #55     #56     #57      #58     #59    #60     #61     #62     #63     #64

            // LED B15
            set_value_nth_bit(m_common_byte_register[byte_idx=54], 6, blue_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, blue_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, blue_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, blue_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, blue_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, blue_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, blue_value.test(0));
            // LED G15
            set_value_nth_bit(m_common_byte_register[byte_idx=55], 7, green_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, green_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, green_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, green_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, green_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, green_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, green_value.test(0));
            // LED R15
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, red_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx=56], 7, red_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, red_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, red_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, red_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, red_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, red_value.test(0));

            break;

        case 14:    // LED #14

        // ROW #1
        // DC        B15    G15    R15    B14    G14    R14    B13    G13    R13    B12    G12    R12
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ======][======][======][======][======][======][======][======][======][======][====
        //          #54     #55     #byte_idx     #57      #58     #59    #60     #61     #62     #63     #64

            // LED B14
            set_value_nth_bit(m_common_byte_register[byte_idx=56], 1, blue_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, blue_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx=57], 7, blue_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, blue_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, blue_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, blue_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, blue_value.test(0));
            // LED G14
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, green_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, green_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, green_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx=58], 7, green_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, green_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, green_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, green_value.test(0));
            // LED R14
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, red_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, red_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, red_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, red_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx=59], 7, red_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, red_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, red_value.test(0));
            break;

        case 13:    // LED #13

        // ROW #1
        // DC        B15    G15    R15    B14    G14    R14    B13    G13    R13    B12    G12    R12
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ======][======][======][======][======][======][======][======][======][======][====
        //          #54     #55     #56     #57      #58     #byte_idx    #60     #61     #62     #63     #64

            // LED B13
            set_value_nth_bit(m_common_byte_register[byte_idx=59], 4, blue_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, blue_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, blue_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, blue_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, blue_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx=60], 7, blue_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, blue_value.test(0));
            // LED G13
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, green_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, green_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, green_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, green_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, green_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, green_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx=61], 7, green_value.test(0));
            // LED R13
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, red_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, red_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, red_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, red_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, red_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, red_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, red_value.test(0));
            break;

        case 12:    // LED #12

        // ROW #1
        // DC        B15    G15    R15    B14    G14    R14    B13    G13    R13    B12    G12    R12
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ======][======][======][======][======][======][======][======][======][======][====
        //          #54     #55     #56     #57      #58     #59    #60     #61     #62     #63     #64

            // LED B12
            set_value_nth_bit(m_common_byte_register[byte_idx=62], 7, blue_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, blue_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, blue_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, blue_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, blue_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, blue_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, blue_value.test(0));
            // LED G12
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, green_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx=63], 7, green_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, green_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, green_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, green_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, green_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, green_value.test(0));
            // LED R12
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, red_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, red_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx=64], 7, red_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, red_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, red_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, red_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, red_value.test(0));
            break;

        case 11:    // LED #11

        // ROW #2
        // DC        B11    G11    R11    B10    G10    R10    B9     G9     R9     B8     G8     R8
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
        //        #64   #65      #66    #67     #68     #69     #70     #71     #72     #73     #74


            // LED B11
            set_value_nth_bit(m_common_byte_register[byte_idx=64], 2, blue_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, blue_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, blue_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx=65], 7, blue_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, blue_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, blue_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, blue_value.test(0));
            // LED G11
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, green_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, green_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, green_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, green_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx=66], 7, green_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, green_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, green_value.test(0));
            // LED R11
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, red_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, red_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, red_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, red_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, red_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx=67], 7, red_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, red_value.test(0));

            break;
        case 10:    // LED #10

        // ROW #2
        // DC        B11    G11    R11    B10    G10    R10    B9     G9     R9     B8     G8     R8
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
        //        #byte_idx   #65      #66    #byte_idx     #68     #69     #70     #71     #72     #73     #74


            // LED B10
            set_value_nth_bit(m_common_byte_register[byte_idx=67], 5, blue_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, blue_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, blue_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, blue_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, blue_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, blue_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx=68], 7, blue_value.test(0));
            // LED G10
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, green_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, green_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, green_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, green_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, green_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, green_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, green_value.test(0));
            // LED R10
            set_value_nth_bit(m_common_byte_register[byte_idx=69], 7, red_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, red_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, red_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, red_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, red_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, red_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, red_value.test(0));        
            break;

        case 9: // LED #9

        // ROW #2
        // DC        B11    G11    R11    B10    G10    R10    B9     G9     R9     B8     G8     R8
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
        //        #64   #65      #66    #67     #68     #69     #70     #71     #72     #73     #74


            // LED B9
            set_value_nth_bit(m_common_byte_register[byte_idx=69], 0, blue_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx=70], 7, blue_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, blue_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, blue_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, blue_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, blue_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, blue_value.test(0));
            // LED G9
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, green_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, green_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx=71], 7, green_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, green_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, green_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, green_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, green_value.test(0));
            // LED R9
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, red_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, red_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, red_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx=72], 7, red_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, red_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, red_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, red_value.test(0));

            break;  

        case 8:     // LED #8

        // ROW #2
        // DC        B11    G11    R11    B10    G10    R10    B9     G9     R9     B8     G8     R8
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
        //        #64   #65      #66    #67     #68     #69     #70     #71     #byte_idx     #73     #74


            // LED B8
            set_value_nth_bit(m_common_byte_register[byte_idx=72], 3, blue_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, blue_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, blue_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, blue_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx=73], 7, blue_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, blue_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, blue_value.test(0));
            // LED G8
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, green_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, green_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, green_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, green_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, green_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx=74], 7, green_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, green_value.test(0));
            // LED R8
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, red_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, red_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, red_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, red_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, red_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, red_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx=75], 7, red_value.test(0));

            break;   

        case 7: // LED #7

        // ROW #3
        // DC        B7      G7     R7    B6     G6     R6     B5     G5     R5     B4     G4     R4
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ======][======][======][======][======][======][======][======][======][======][====
        //          #75     #76     #77     #78     #79     #80     #81     #82     #83     #84     #85

            // LED B7
            set_value_nth_bit(m_common_byte_register[byte_idx=75], 6, blue_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, blue_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, blue_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, blue_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, blue_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, blue_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, blue_value.test(0));
            // LED G7
            set_value_nth_bit(m_common_byte_register[byte_idx=76], 7, green_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, green_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, green_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, green_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, green_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, green_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, green_value.test(0));
            // LED R7
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, red_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx=77], 7, red_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, red_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, red_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, red_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, red_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, red_value.test(0));

            break;       

        case 6:     // LED #6

        // ROW #3   
        // DC        B7      G7     R7    B6     G6     R6     B5     G5     R5     B4     G4     R4
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ======][======][======][======][======][======][======][======][======][======][====
        //          #75     #76     #77     #78     #79     #80     #81     #82     #83     #84     #85

            // LED B6
            set_value_nth_bit(m_common_byte_register[byte_idx=77], 1, blue_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, blue_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx=78],  7, blue_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, blue_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, blue_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, blue_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, blue_value.test(0));
            // LED G6
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, green_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, green_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, green_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx=79], 7, green_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, green_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, green_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, green_value.test(0));
            // LED R6
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, red_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, red_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, red_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, red_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx=80], 7, red_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, red_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, red_value.test(0));

            break;

        case 5: // LED #5

        // ROW #3   
        // DC        B7      G7     R7    B6     G6     R6     B5     G5     R5     B4     G4     R4
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ======][======][======][======][======][======][======][======][======][======][====
        //          #75     #76     #77     #78     #79     #80     #81     #82     #83     #84     #85

            // LED B5
            set_value_nth_bit(m_common_byte_register[byte_idx=80], 4, blue_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, blue_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, blue_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, blue_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, blue_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx=81], 7, blue_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, blue_value.test(0));
            // LED G5
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, green_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, green_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, green_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, green_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, green_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, green_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx=82], 7, green_value.test(0));
            // LED R5
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, red_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, red_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, red_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, red_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, red_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, red_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, red_value.test(0));

            break;

        case 4:

        // ROW #3   
        // DC        B7      G7     R7    B6     G6     R6     B5     G5     R5     B4     G4     R4
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ======][======][======][======][======][======][======][======][======][======][====
        //          #75     #76     #77     #78     #79     #80     #81     #82     #83     #84     #85

            // LED B4
            set_value_nth_bit(m_common_byte_register[byte_idx=83], 7, blue_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, blue_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, blue_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, blue_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, blue_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, blue_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, blue_value.test(0));
            // LED G4
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, green_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx=84], 7, green_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, green_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, green_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, green_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, green_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, green_value.test(0));
            // LED R4
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, red_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, red_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx=85], 7, red_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, red_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, red_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, red_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, red_value.test(0));

            break;

        case 3:

        // ROW #4
        // DC        B3     G3     R3      B2     G2    R2      B1    G1     R1     B0     G0     R0
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
        //        #85   #86     #87      #88    #89     #90     #91     #92     #93     #94     #95   #96    

            // LED B3
            set_value_nth_bit(m_common_byte_register[byte_idx=85], 2, blue_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, blue_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, blue_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx=86], 7, blue_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, blue_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, blue_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, blue_value.test(0));
            // LED G3
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, green_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, green_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, green_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, green_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx=87], 7, green_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, green_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, green_value.test(0));
            // LED R3
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, red_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, red_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, red_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, red_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, red_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx=88], 7, red_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, red_value.test(0));

            

            break;


        case 2:

        // ROW #4
        // DC        B3     G3     R3      B2     G2    R2      B1    G1     R1     B0     G0     R0
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
        //        #85   #86     #87      #88    #89     #90     #91     #92     #93     #94     #95   #96    

            // LED B2
            set_value_nth_bit(m_common_byte_register[byte_idx=88], 5, blue_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, blue_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, blue_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, blue_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, blue_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, blue_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx=89], 7, blue_value.test(0));
            // LED G2
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, green_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, green_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, green_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, green_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, green_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, green_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, green_value.test(0));
            // LED R2
            set_value_nth_bit(m_common_byte_register[byte_idx=90], 7, red_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, red_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, red_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, red_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, red_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, red_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, red_value.test(0));

            break;



        case 1:

        // ROW #4
        // DC        B3     G3     R3      B2     G2    R2      B1    G1     R1     B0     G0     R0
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
        //        #85   #86     #87      #88    #89     #90     #91     #92     #93     #94     #95   #96    

            // LED B1
            set_value_nth_bit(m_common_byte_register[byte_idx=90], 0, blue_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx=91], 7, blue_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, blue_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, blue_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, blue_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, blue_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, blue_value.test(0));
            // LED G1
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, green_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, green_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx=92], 7, green_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, green_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, green_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, green_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, green_value.test(0));
            // LED R1
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, red_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, red_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, red_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx=93], 7, red_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, red_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, red_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, red_value.test(0));

            break;

 

        case 0:

        // ROW #4
        // DC        B3     G3     R3      B2     G2    R2      B1    G1     R1     B0     G0     R0
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
        //        #85   #86     #87      #88    #89     #90     #91     #92     #93     #94     #95   #96    

            // LED B0
            set_value_nth_bit(m_common_byte_register[byte_idx=93], 3, blue_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, blue_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, blue_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, blue_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx=94], 7, blue_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, blue_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, blue_value.test(0));
            // LED G0
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, green_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, green_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, green_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, green_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, green_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx=95], 7, green_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx], 6, green_value.test(0));
            // LED R0
            set_value_nth_bit(m_common_byte_register[byte_idx], 5, red_value.test(6));
            set_value_nth_bit(m_common_byte_register[byte_idx], 4, red_value.test(5));
            set_value_nth_bit(m_common_byte_register[byte_idx], 3, red_value.test(4));
            set_value_nth_bit(m_common_byte_register[byte_idx], 2, red_value.test(3));
            set_value_nth_bit(m_common_byte_register[byte_idx], 1, red_value.test(2));
            set_value_nth_bit(m_common_byte_register[byte_idx], 0, red_value.test(1));
            set_value_nth_bit(m_common_byte_register[byte_idx=96], 7, red_value.test(0));

            break;

        default:    // led_idx > 15
            return false;
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
    uint16_t gs_common_pos = m_gs_data_offset + led_offset;

    // check gs_common_pos has left enough bits for one segment of LED GS data
    // This could happen if the header constants are incorrect
    if (gs_common_pos + m_gs_data_one_led_size_bits > m_common_reg_size_bits)
    {
        return false;
    }

        // ROW #1
        // GS            B15             G15             R15              B14             G14             R14            B13             G13             R13   
        // bits    0[==============][==============][==============][==============][==============][==============][==============][==============][==============]
        // Bytes   [======][======][======][======][======][======][======][======][======][======][======][======][======][======][======][======][======][======][
        //            #0      #1      #2      #3      #4      #5      #6      #7      #8      #9     #10     #11     #12     #13      #14     #15     #16     #17


    // set the bits
    
    // should always give multiple of 6.
    uint16_t begin_byte_idx = gs_common_pos / 8;
    

    // byte #0, skip the MSB 
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 6, blue_value.test(15));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 5, blue_value.test(14));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 4, blue_value.test(13));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 3, blue_value.test(12));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 2, blue_value.test(11));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 1, blue_value.test(10));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 0, blue_value.test(9));

    // byte #1
    begin_byte_idx++;
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 7, blue_value.test(8));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 6, blue_value.test(7));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 5, blue_value.test(6));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 4, blue_value.test(5));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 3, blue_value.test(4));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 2, blue_value.test(3));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 1, blue_value.test(2));    
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 0, blue_value.test(1)); 

    // byte #2
    begin_byte_idx++;
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 7, blue_value.test(0));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 6, green_value.test(15));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 5, green_value.test(14));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 4, green_value.test(13));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 3, green_value.test(12));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 2, green_value.test(11));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 1, green_value.test(10));    
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 0, green_value.test(9));     

    // byte #3
    begin_byte_idx++;
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 7, green_value.test(8));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 6, green_value.test(7));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 5, green_value.test(6));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 4, green_value.test(5));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 3, green_value.test(4));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 2, green_value.test(3));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 1, green_value.test(2));    
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 0, green_value.test(1)); 

    // byte #4
    begin_byte_idx++;
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 7, green_value.test(0));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 6, red_value.test(15));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 5, red_value.test(14));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 4, red_value.test(13));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 3, red_value.test(12));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 2, red_value.test(11));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 1, red_value.test(10));    
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 0, red_value.test(9)); 

    // byte #5
    begin_byte_idx++;
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 7, red_value.test(8));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 6, red_value.test(7));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 5, red_value.test(6));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 4, red_value.test(5));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 3, red_value.test(4));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 2, red_value.test(3));
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 1, red_value.test(2));    
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 0, red_value.test(1));

    // byte #5, write the final bit
    begin_byte_idx++;
    set_value_nth_bit(m_common_byte_register[begin_byte_idx], 7, red_value.test(0));

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
    // clock the data through and latch
#ifdef USE_HAL_DRIVER
    HAL_StatusTypeDef res = HAL_SPI_Transmit(&m_spi_interface, (uint8_t*)m_common_byte_register.data(), m_common_reg_size_bytes, HAL_MAX_DELAY);
    UNUSED(res);
#endif
    toggle_latch();
}

void Driver::toggle_latch()
{
#ifdef USE_HAL_DRIVER
    HAL_Delay(m_latch_delay_ms);
    HAL_GPIO_WritePin(m_lat_port, m_lat_pin, GPIO_PIN_SET);
    HAL_Delay(m_latch_delay_ms);
    HAL_GPIO_WritePin(m_lat_port, m_lat_pin, GPIO_PIN_RESET);
    HAL_Delay(m_latch_delay_ms);
#endif
}

void Driver::flush_common_register()
{
    for (auto &byte : m_common_byte_register)
    {
        byte = 0x00;
    }
    send_data();
}

void Driver::print_common_bits()
{
#ifdef USE_RTT
    SEGGER_RTT_printf(0, "\r\n");
    for (uint16_t idx = 45; idx < 53; idx++)
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
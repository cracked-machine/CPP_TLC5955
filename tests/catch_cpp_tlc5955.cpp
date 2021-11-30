


#include <catch2/catch_all.hpp>
#include <iostream>
#include <tlc5955_tester.hpp>
#include <algorithm>

// TLC5955 device datasheet:
// https://www.ti.com/lit/ds/symlink/tlc5955.pdf


TEST_CASE("Testing TLC5955 common register", "[tlc5955]")
{

    tlc5955::tlc5955_tester leds_tester;
    uint8_t result {0};
    
    // @brief Testing tlc5955::Driver::flush_common_register()
    SECTION("Test flush command")
    {
        // set all the bits to 1
        std::for_each (leds_tester.data_begin(), leds_tester.data_end(), [](auto &byte){
            byte = 0xFF;
        });

        // check they are set to 1
        std::for_each (leds_tester.data_begin(), leds_tester.data_end(), [](auto &byte){
            REQUIRE(byte == 0xFF);
        });

        // run the flush command
        leds_tester.flush_common_register();

        // check the bits are all cleared
        std::for_each (leds_tester.data_begin(), leds_tester.data_end(), [](auto &byte){
            REQUIRE(byte == 0x00);
        });
    }

    // @brief Testing tlc5955::Driver::set_control_bit
    SECTION("Latch bit test")
    {
        // Latch       
        // bits      =
        // Bytes     [
        //          #0          
        // latch bit test
        
        REQUIRE(leds_tester.get_common_reg_at(0, result));
        REQUIRE(result == 0b00000000);   

        leds_tester.set_control_bit(true);

        REQUIRE(leds_tester.get_common_reg_at(0, result));
        REQUIRE(result == 0b10000000);      // 128
    }

    // @brief Testing tlc5955::Driver::set_ctrl_cmd_bits()
    SECTION("Control bits test")
    {
        // control byte test

        // Ctrl      10010110  
        // bits      [======]
        // Bytes     ======][
        //             #0    #1

        leds_tester.set_ctrl_cmd_bits();
        REQUIRE(leds_tester.get_common_reg_at(0, result));
        REQUIRE(result == 0b1001011);      // 75
    }

    // Testing tlc5955::Driver::set_padding_bits()
    SECTION("Padding bits test")
    {
        // padding bits test - bytes 1-48 should be empty 

        leds_tester.set_padding_bits();
        for (uint8_t idx = 1; idx < 49; idx++)
        {
            REQUIRE(leds_tester.get_common_reg_at(idx,result));
            REQUIRE(result == 0);
        }
    }

    // Testing tlc5955::Driver::set_function_data()
    SECTION("Function bits test")
    {
        // function bits test   
        // bits      [===]
        //           =][==
        // Bytes   #49  #50

        leds_tester.set_function_data(true, false, false, false, false);
        REQUIRE(leds_tester.get_common_reg_at(49, result));
        REQUIRE(result == 0b00000010);  // 2
        
        leds_tester.set_function_data(true, true, false, false, false);
        REQUIRE(leds_tester.get_common_reg_at(49, result));
        REQUIRE(result == 0b00000011);  // 3
                        
        leds_tester.set_function_data(true, true, true, false, false);
        REQUIRE(leds_tester.get_common_reg_at(50, result));
        REQUIRE(result == 0b10000000);  // 128
        
        leds_tester.set_function_data(true, true, true, true, false);
        REQUIRE(leds_tester.get_common_reg_at(50, result));
        REQUIRE(result == 0b11000000);  // 192

        leds_tester.set_function_data(true, true, true, true, true);
        REQUIRE(leds_tester.get_common_reg_at(50, result));
        REQUIRE(result == 0b11100000);  // 224
    }
    
    // @brief Testing tlc5955::Driver::set_bc_data()
    SECTION("Brightness Control bits test")
    {
        // BC bits test
        // BC         blue   green   red
        // bits      [=====][=====][=====]
        // bits      ====][======][======]
        // Bytes     #50    #51      #52    

        std::bitset<leds_tester.m_bc_data_resolution> bc_test_on {127};
        std::bitset<leds_tester.m_bc_data_resolution> bc_test_off {0};

        REQUIRE(leds_tester.get_common_reg_at(50, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(51, result));
        REQUIRE(result == 0b00000000); 
        REQUIRE(leds_tester.get_common_reg_at(52, result));
        REQUIRE(result == 0b00000000);                   

        leds_tester.set_bc_data(bc_test_on, bc_test_off, bc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(50, result));
        REQUIRE(result == 0b00011111);  
        REQUIRE(leds_tester.get_common_reg_at(51, result));
        REQUIRE(result == 0b11000000); 
        REQUIRE(leds_tester.get_common_reg_at(52, result));
        REQUIRE(result == 0x00000000);         
 
        leds_tester.set_bc_data(bc_test_off, bc_test_on, bc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(50, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(51, result));
        REQUIRE(result == 0b00111111); 
        REQUIRE(leds_tester.get_common_reg_at(52, result));
        REQUIRE(result == 0b10000000);         

        leds_tester.set_bc_data(bc_test_off, bc_test_off, bc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(50, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(51, result));
        REQUIRE(result == 0x00000000); 
        REQUIRE(leds_tester.get_common_reg_at(52, result));
        REQUIRE(result == 0b01111111);
 
        leds_tester.set_bc_data(bc_test_off, bc_test_off, bc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(50, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(51, result));
        REQUIRE(result == 0x00000000); 
        REQUIRE(leds_tester.get_common_reg_at(52, result));
        REQUIRE(result == 0b00000000);        
 
        leds_tester.set_bc_data(bc_test_on, bc_test_on, bc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(50, result));
        REQUIRE(result == 0b00011111);  
        REQUIRE(leds_tester.get_common_reg_at(51, result));
        REQUIRE(result == 0b11111111); 
        REQUIRE(leds_tester.get_common_reg_at(52, result));
        REQUIRE(result == 0b11111111);        
  
    }
    // @brief  Testing tlc5955::Driver::set_mc_data()
    SECTION("Max Current bit test")
    {
        // MC         B  G  R
        // bits      [=][=][=]      // 9 bits
        // bits      [======][
        // Bytes       #53    #54     


        // @brief Test Preset: all bits of one colour on 
        std::bitset<leds_tester.m_mc_data_resolution> preset_mc_test_on {7};
        // @brief Test Preset: all bits of one colour off 
        std::bitset<leds_tester.m_mc_data_resolution> preset_mc_test_off {0};
        // @brief Test Preset: blue LSB on, all others off
        std::bitset<leds_tester.m_mc_data_resolution> preset_mc_test_blue {1};
        // @brief Test Preset: green LSB on, all others off 
        std::bitset<leds_tester.m_mc_data_resolution> preset_mc_test_green {1};
        // @brief Test Preset: red LSB on, all others off 
        std::bitset<leds_tester.m_mc_data_resolution> preset_mc_test_red {1};
 
        // blue on full MC, green and red off
        leds_tester.set_mc_data(preset_mc_test_on, preset_mc_test_off, preset_mc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(53, result));
        REQUIRE(result == 0b11100000);                              // 224
        REQUIRE(leds_tester.get_common_reg_at(54, result));
        REQUIRE(result == 0b00000000);                              // 0

        // green on full MC, blue and red off
        leds_tester.set_mc_data(preset_mc_test_off, preset_mc_test_on, preset_mc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(53, result));
        REQUIRE(result == 0b00011100);                              // 28
        REQUIRE(leds_tester.get_common_reg_at(54, result));
        REQUIRE(result == 0b00000000);                              // 0

        // red on full MC, blue and green off
        leds_tester.set_mc_data(preset_mc_test_off, preset_mc_test_off, preset_mc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(53, result));
        REQUIRE(result == 0b00000011);                              // 3
        REQUIRE(leds_tester.get_common_reg_at(54, result));
        REQUIRE(result == 0b10000000);                              // 128

        // MC off for all colours
        leds_tester.set_mc_data(preset_mc_test_off, preset_mc_test_off, preset_mc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(53, result));
        REQUIRE(result == 0b00000000);                              // 0
        REQUIRE(leds_tester.get_common_reg_at(54, result));
        REQUIRE(result == 0b00000000);                              // 0    

        // left-shift bit MC section from LSB to MSB
        leds_tester.set_mc_data(preset_mc_test_off, preset_mc_test_off, preset_mc_test_red);   
        REQUIRE(leds_tester.get_common_reg_at(53, result));
        REQUIRE(result == 0b00000000);                              // 0
        REQUIRE(leds_tester.get_common_reg_at(54, result));
        REQUIRE(result == 0b10000000);                              // 128

        leds_tester.set_mc_data(preset_mc_test_off, preset_mc_test_off, preset_mc_test_red <<= 1);
        REQUIRE(leds_tester.get_common_reg_at(53, result));
        REQUIRE(result == 0b00000001);                              // 1
        REQUIRE(leds_tester.get_common_reg_at(54, result));
        REQUIRE(result == 0b00000000);                              // 0

        leds_tester.set_mc_data(preset_mc_test_off, preset_mc_test_off, preset_mc_test_red <<= 1);
        REQUIRE(leds_tester.get_common_reg_at(53, result));
        REQUIRE(result == 0b00000010);                              // 2
        REQUIRE(leds_tester.get_common_reg_at(54, result));
        REQUIRE(result == 0b00000000);                              // 0
        
        leds_tester.set_mc_data(preset_mc_test_off, preset_mc_test_green, preset_mc_test_red <<= 1);
        REQUIRE(leds_tester.get_common_reg_at(53, result));
        REQUIRE(result == 0b00000100);                              // 4
        REQUIRE(leds_tester.get_common_reg_at(54, result));
        REQUIRE(result == 0b00000000);                              // 0
        
        leds_tester.set_mc_data(preset_mc_test_off, preset_mc_test_green <<= 1, preset_mc_test_red);
        REQUIRE(leds_tester.get_common_reg_at(53, result));
        REQUIRE(result == 0b00001000);                              // 8
        REQUIRE(leds_tester.get_common_reg_at(54, result));
        REQUIRE(result == 0b00000000);                              // 0
        
        leds_tester.set_mc_data(preset_mc_test_off, preset_mc_test_green <<= 1, preset_mc_test_red);
        REQUIRE(leds_tester.get_common_reg_at(53, result));
        REQUIRE(result == 0b00010000);                              // 16
        REQUIRE(leds_tester.get_common_reg_at(54, result)); 
        REQUIRE(result == 0b00000000);                              // 0
        
        leds_tester.set_mc_data(preset_mc_test_blue, preset_mc_test_green <<= 1, preset_mc_test_red);  
        REQUIRE(leds_tester.get_common_reg_at(53, result));
        REQUIRE(result == 0b00100000);                              // 32
        REQUIRE(leds_tester.get_common_reg_at(54, result));
        REQUIRE(result == 0b00000000);                              // 0
        
        leds_tester.set_mc_data(preset_mc_test_blue <<= 1, preset_mc_test_green, preset_mc_test_red);  
        REQUIRE(leds_tester.get_common_reg_at(53, result));
        REQUIRE(result == 0b01000000);                              // 64
        REQUIRE(leds_tester.get_common_reg_at(54, result));
        REQUIRE(result == 0b00000000);  
        
        leds_tester.set_mc_data(preset_mc_test_blue <<= 1, preset_mc_test_green, preset_mc_test_red);  
        REQUIRE(leds_tester.get_common_reg_at(53, result));
        REQUIRE(result == 0b10000000);                              // 128
        REQUIRE(leds_tester.get_common_reg_at(54, result));
        REQUIRE(result == 0b00000000);  

    }
}

// @brief Testing tlc5955::Driver::set_dc_data()
TEST_CASE("Dot Correction bit tests", "[tlc5955]")
{
    tlc5955::tlc5955_tester leds_tester;

    // @brief Test Preset: all bits of one colour on 
    const std::bitset<leds_tester.m_dc_data_resolution> preset_dc_test_on {127};
    // @brief Test Preset: all bits of one colour off
    const std::bitset<leds_tester.m_dc_data_resolution> preset_dc_test_off {0};

    uint8_t result {0};

    SECTION("LED15 Dot Correction bits test")
    {

        // Common Register-to-Byte array mapping for DC (dot correction) data
        
        // ROW #1
        // DC        B15    G15    R15    B14    G14    R14    B13    G13    R13    B12    G12    R12
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ======][======][======][======][======][======][======][======][======][======][====
        //          #54     #55     #56     #57      #58     #59    #60     #61     #62     #63     #64
 
        // B15 + G15 + R15 off
        leds_tester.set_dc_data(15, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(54, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(55, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(56, result));
        REQUIRE(result == 0b00000000);  

        // B15 on, G15 + R15 off
        leds_tester.set_dc_data(15, preset_dc_test_on, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(54, result));
        REQUIRE(result == 0b01111111);  
        REQUIRE(leds_tester.get_common_reg_at(55, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(56, result));
        REQUIRE(result == 0b00000000);

        // B15 off, G15 on, R15 off
        leds_tester.set_dc_data(15, preset_dc_test_off, preset_dc_test_on, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(54, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(55, result));
        REQUIRE(result == 0b11111110);  
        REQUIRE(leds_tester.get_common_reg_at(56, result));
        REQUIRE(result == 0b00000000);
        
        // B15 off, G15 off, R15 on
        leds_tester.set_dc_data(15, preset_dc_test_off, preset_dc_test_off, preset_dc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(54, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(55, result));
        REQUIRE(result == 0b00000001);  
        REQUIRE(leds_tester.get_common_reg_at(56, result));
        REQUIRE(result == 0b11111100);
        
        // B15 + G15 + R15 off
        leds_tester.set_dc_data(15, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(54, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(55, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(56, result));
        REQUIRE(result == 0b00000000);  
  
    }

    SECTION("LED14 Dot Correction bits test")
    {

        // Common Register-to-Byte array mapping for DC (dot correction) data
        
        // ROW #1
        // DC        B15    G15    R15    B14    G14    R14    B13    G13    R13    B12    G12    R12
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ======][======][======][======][======][======][======][======][======][======][====
        //          #54     #55     #56     #57      #58     #59    #60     #61     #62     #63     #64

        // B14 + G14 + R14 off
        leds_tester.set_dc_data(14, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(56, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(57, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(58, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(59, result));
        REQUIRE(result == 0b00000000); 
        // B14 on, G14 + R14 off
        leds_tester.set_dc_data(14, preset_dc_test_on, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(56, result));
        REQUIRE(result == 0b00000011);  
        REQUIRE(leds_tester.get_common_reg_at(57, result));
        REQUIRE(result == 0b11111000);  
        REQUIRE(leds_tester.get_common_reg_at(58, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(59, result));
        REQUIRE(result == 0b00000000);

        // B14 off, G14 on, R14 off
        leds_tester.set_dc_data(14, preset_dc_test_off, preset_dc_test_on, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(56, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(57, result));
        REQUIRE(result == 0b00000111);  
        REQUIRE(leds_tester.get_common_reg_at(58, result));
        REQUIRE(result == 0b11110000);  
        REQUIRE(leds_tester.get_common_reg_at(59, result));
        REQUIRE(result == 0b00000000);
        
        // B14 off, G14 off, R14 on
        leds_tester.set_dc_data(14, preset_dc_test_off, preset_dc_test_off, preset_dc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(56, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(57, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(58, result));
        REQUIRE(result == 0b00001111);  
        REQUIRE(leds_tester.get_common_reg_at(59, result));
        REQUIRE(result == 0b11100000);
        
        // B14 + G14 + R14 off
        leds_tester.set_dc_data(14, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(56, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(57, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(58, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(59, result));
        REQUIRE(result == 0b00000000); 
    }
    SECTION("LED13 Dot Correction bits test")
    {

        // Common Register-to-Byte array mapping for DC (dot correction) data
        
        // ROW #1
        // DC        B15    G15    R15    B14    G14    R14    B13    G13    R13    B12    G12    R12
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ======][======][======][======][======][======][======][======][======][======][====
        //          #54     #55     #56     #57      #58     #59    #60     #61     #62     #63     #64

        // B13 + G13 + R13 off
        leds_tester.set_dc_data(13, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(59, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(60, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(61, result));
        REQUIRE(result == 0b00000000);  

        // B13 on, G13 + R13 off
        leds_tester.set_dc_data(13, preset_dc_test_on, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(59, result));
        REQUIRE(result == 0b00011111);  
        REQUIRE(leds_tester.get_common_reg_at(60, result));
        REQUIRE(result == 0b11000000);  
        REQUIRE(leds_tester.get_common_reg_at(61, result));
        REQUIRE(result == 0b00000000);  

        // B13 off, G13 on, R13 off
        leds_tester.set_dc_data(13, preset_dc_test_off, preset_dc_test_on, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(59, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(60, result));
        REQUIRE(result == 0b00111111);  
        REQUIRE(leds_tester.get_common_reg_at(61, result));
        REQUIRE(result == 0b10000000);  
        
        // B13 off, G13 off, R13 on
        leds_tester.set_dc_data(13, preset_dc_test_off, preset_dc_test_off, preset_dc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(59, result));
        REQUIRE(result == 0b000000);  
        REQUIRE(leds_tester.get_common_reg_at(60, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(61, result));
        REQUIRE(result == 0b01111111);  
        
        // B13 + G13 + R13 off
        leds_tester.set_dc_data(13, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(59, result));
        REQUIRE(result == 0b000000);  
        REQUIRE(leds_tester.get_common_reg_at(60, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(61, result));
        REQUIRE(result == 0b00000000);  
    }

    SECTION("LED12 Dot Correction bits test")
    {

        // Common Register-to-Byte array mapping for DC (dot correction) data
        
        // ROW #1
        // DC        B15    G15    R15    B14    G14    R14    B13    G13    R13    B12    G12    R12
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ======][======][======][======][======][======][======][======][======][======][====
        //          #54     #55     #56     #57      #58     #59    #60     #61     #62     #63     #64

        // B12 + G12 + R12 off
        leds_tester.set_dc_data(12, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(62, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(63, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(64, result));
        REQUIRE(result == 0b00000000);  

        // B12 on, G12 + R12 off
        leds_tester.set_dc_data(12, preset_dc_test_on, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(62, result));
        REQUIRE(result == 0b11111110);  
        REQUIRE(leds_tester.get_common_reg_at(63, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(64, result));
        REQUIRE(result == 0b00000000);  

        // B12 off, G12 on, R12 off
        leds_tester.set_dc_data(12, preset_dc_test_off, preset_dc_test_on, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(62, result));
        REQUIRE(result == 0b00000001);  
        REQUIRE(leds_tester.get_common_reg_at(63, result));
        REQUIRE(result == 0b11111100);  
        REQUIRE(leds_tester.get_common_reg_at(64, result));
        REQUIRE(result == 0b00000000);   
        
        // B12 off, G12 off, R12 on
        leds_tester.set_dc_data(12, preset_dc_test_off, preset_dc_test_off, preset_dc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(62, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(63, result));
        REQUIRE(result == 0b00000011);  
        REQUIRE(leds_tester.get_common_reg_at(64, result));
        REQUIRE(result == 0b11111000);   
        
        // B12 + G12 + R12 off
        leds_tester.set_dc_data(12, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(62, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(63, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(64, result));
        REQUIRE(result == 0b00000000);    
    }

    SECTION("LED11 Dot Correction bits test")
    {

        // ROW #2
        // DC        B11    G11    R11    B10    G10    R10    B9     G9     R9     B8     G8     R8
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
        //        #64   #65      #66    #67     #68     #69     #70     #71     #72     #73     #74
 
        // B11 + G11 + R11 off
        leds_tester.set_dc_data(11, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(64, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(65, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(66, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(67, result));
        REQUIRE(result == 0b00000000);  

        // B11 on, G11 + R11 off
        leds_tester.set_dc_data(11, preset_dc_test_on, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(64, result));
        REQUIRE(result == 0b00000111);  
        REQUIRE(leds_tester.get_common_reg_at(65, result));
        REQUIRE(result == 0b11110000);  
        REQUIRE(leds_tester.get_common_reg_at(66, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(67, result));
        REQUIRE(result == 0b00000000);  

        // B11 off, G11 on, R11 off
        leds_tester.set_dc_data(11, preset_dc_test_off, preset_dc_test_on, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(64, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(65, result));
        REQUIRE(result == 0b00001111);  
        REQUIRE(leds_tester.get_common_reg_at(66, result));
        REQUIRE(result == 0b11100000);  
        REQUIRE(leds_tester.get_common_reg_at(67, result));
        REQUIRE(result == 0b00000000);  
        
        // B11 off, G11 off, R11 on
        leds_tester.set_dc_data(11, preset_dc_test_off, preset_dc_test_off, preset_dc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(64, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(65, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(66, result));
        REQUIRE(result == 0b00011111);  
        REQUIRE(leds_tester.get_common_reg_at(67, result));
        REQUIRE(result == 0b11000000);  
        
        // B11 + G11 + R11 off
        leds_tester.set_dc_data(11, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(64, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(65, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(66, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(67, result));
        REQUIRE(result == 0b00000000);  
    }

    SECTION("LED10 Dot Correction bits test")
    {

        // ROW #2
        // DC        B11    G11    R11    B10    G10    R10    B9     G9     R9     B8     G8     R8
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
        //        #64   #65      #66    #67     #68     #69     #70     #71     #72     #73     #74
 
        // B10 + G10 + R10 off
        leds_tester.set_dc_data(10, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(67, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(68, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(69, result));
        REQUIRE(result == 0b00000000);  


        // B10 on, G10 + R10 off
        leds_tester.set_dc_data(10, preset_dc_test_on, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(67, result));
        REQUIRE(result == 0b00111111);  
        REQUIRE(leds_tester.get_common_reg_at(68, result));
        REQUIRE(result == 0b10000000);  
        REQUIRE(leds_tester.get_common_reg_at(69, result));
        REQUIRE(result == 0b00000000);  

        // B10 off, G10 on, R10 off
        leds_tester.set_dc_data(10, preset_dc_test_off, preset_dc_test_on, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(67, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(68, result));
        REQUIRE(result == 0b01111111);  
        REQUIRE(leds_tester.get_common_reg_at(69, result));
        REQUIRE(result == 0b00000000);  
        
        // B10 off, G10 off, R10 on
        leds_tester.set_dc_data(10, preset_dc_test_off, preset_dc_test_off, preset_dc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(67, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(68, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(69, result));
        REQUIRE(result == 0b11111110);  
        
        // B10 + G10 + R10 off
        leds_tester.set_dc_data(10, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(67, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(68, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(69, result));
        REQUIRE(result == 0b00000000);  
    }

    SECTION("LED9 Dot Correction bits test")
    {

        // ROW #2
        // DC        B11    G11    R11    B10    G10    R10    B9     G9     R9     B8     G8     R8
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
        //        #64   #65      #66    #67     #68     #69     #70     #71     #72     #73     #74
 
        // B9 + G9 + R9 off
        leds_tester.set_dc_data(9, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(69, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(70, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(71, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(72, result));
        REQUIRE(result == 0b00000000);  

        // B9 on, G9 + R9 off
        leds_tester.set_dc_data(9, preset_dc_test_on, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(69, result));
        REQUIRE(result == 0b00000001);  
        REQUIRE(leds_tester.get_common_reg_at(70, result));
        REQUIRE(result == 0b11111100);  
        REQUIRE(leds_tester.get_common_reg_at(71, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(72, result));
        REQUIRE(result == 0b00000000);   

        // B9 off, G9 on, R9 off
        leds_tester.set_dc_data(9, preset_dc_test_off, preset_dc_test_on, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(69, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(70, result));
        REQUIRE(result == 0b00000011);  
        REQUIRE(leds_tester.get_common_reg_at(71, result));
        REQUIRE(result == 0b11111000);  
        REQUIRE(leds_tester.get_common_reg_at(72, result));
        REQUIRE(result == 0b00000000);    
        
        // B9 off, G9 off, R9 on
        leds_tester.set_dc_data(9, preset_dc_test_off, preset_dc_test_off, preset_dc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(69, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(70, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(71, result));
        REQUIRE(result == 0b00000111);  
        REQUIRE(leds_tester.get_common_reg_at(72, result));
        REQUIRE(result == 0b11110000);   
        
        // B9 + G9 + R9 off
        leds_tester.set_dc_data(9, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(69, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(70, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(71, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(72, result));
        REQUIRE(result == 0b00000000);   
    }

    SECTION("LED8 Dot Correction bits test")
    {

        // ROW #2
        // DC        B11    G11    R11    B10    G10    R10    B9     G9     R9     B8     G8     R8
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
        //        #64   #65      #66    #67     #68     #69     #70     #71     #72     #73     #74
 
        // B8 + G8 + R8 off
        leds_tester.set_dc_data(8, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(72, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(73, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(74, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(75, result));
        REQUIRE(result == 0b00000000);  

        // B8 on, G8 + R8 off
        leds_tester.set_dc_data(8, preset_dc_test_on, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(72, result));
        REQUIRE(result == 0b00001111);  
        REQUIRE(leds_tester.get_common_reg_at(73, result));
        REQUIRE(result == 0b11100000);  
        REQUIRE(leds_tester.get_common_reg_at(74, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(75, result));
        REQUIRE(result == 0b00000000);   

        // B8 off, G8 on, R8 off
        leds_tester.set_dc_data(8, preset_dc_test_off, preset_dc_test_on, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(72, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(73, result));
        REQUIRE(result == 0b00011111);  
        REQUIRE(leds_tester.get_common_reg_at(74, result));
        REQUIRE(result == 0b11000000);  
        REQUIRE(leds_tester.get_common_reg_at(75, result));
        REQUIRE(result == 0b00000000);     
        
        // B8 off, G8 off, R8 on
        leds_tester.set_dc_data(8, preset_dc_test_off, preset_dc_test_off, preset_dc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(72, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(73, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(74, result));
        REQUIRE(result == 0b00111111);  
        REQUIRE(leds_tester.get_common_reg_at(75, result));
        REQUIRE(result == 0b10000000);     
        
        // B8 + G8 + R8 off
        leds_tester.set_dc_data(8, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(72, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(73, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(74, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(75, result));
        REQUIRE(result == 0b00000000);    
    }

    SECTION("LED7 Dot Correction bits test")
    {

        // ROW #3
        // DC        B7      G7     R7    B6     G6     R6     B5     G5     R5     B4     G4     R4
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ======][======][======][======][======][======][======][======][======][======][====
        //          #75     #76     #77     #78     #79     #80     #81     #82     #83     #84     #85

        // B7 + G7 + R7 off
        leds_tester.set_dc_data(7, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(75, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(76, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(77, result));
        REQUIRE(result == 0b00000000); 
        
        // B7 on, G7 + R7 off
        leds_tester.set_dc_data(7, preset_dc_test_on, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(75, result));
        REQUIRE(result == 0b01111111);  
        REQUIRE(leds_tester.get_common_reg_at(76, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(77, result));
        REQUIRE(result == 0b00000000); 
        
        // B7 off, G7 on, R7 off
        leds_tester.set_dc_data(7, preset_dc_test_off, preset_dc_test_on, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(75, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(76, result));
        REQUIRE(result == 0b11111110);  
        REQUIRE(leds_tester.get_common_reg_at(77, result));
        REQUIRE(result == 0b00000000); 
                
        // B7 off, G7 off, R7 on
        leds_tester.set_dc_data(7, preset_dc_test_off, preset_dc_test_off, preset_dc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(75, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(76, result));
        REQUIRE(result == 0b00000001);  
        REQUIRE(leds_tester.get_common_reg_at(77, result));
        REQUIRE(result == 0b11111100); 
                
        // B7 + G7 + R7 off
        leds_tester.set_dc_data(7, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(75, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(76, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(77, result));
        REQUIRE(result == 0b00000000); 
        
    }

    SECTION("LED6 Dot Correction bits test")
    {

        // ROW #3
        // DC        B7      G7     R7    B6     G6     R6     B5     G5     R5     B4     G4     R4
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ======][======][======][======][======][======][======][======][======][======][====
        //          #75     #76     #77     #78     #79     #80     #81     #82     #83     #84     #85

        // B6 + G6 + R6 off
        leds_tester.set_dc_data(6, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(77, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(78, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(79, result));
        REQUIRE(result == 0b00000000); 
        REQUIRE(leds_tester.get_common_reg_at(80, result));
        REQUIRE(result == 0b00000000); 
        
        // B6 on, G6 + R6 off
        leds_tester.set_dc_data(6, preset_dc_test_on, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(77, result));
        REQUIRE(result == 0b00000011);  
        REQUIRE(leds_tester.get_common_reg_at(78, result));
        REQUIRE(result == 0b11111000);  
        REQUIRE(leds_tester.get_common_reg_at(79, result));
        REQUIRE(result == 0b00000000); 
        REQUIRE(leds_tester.get_common_reg_at(80, result));
        REQUIRE(result == 0b00000000);  
        
        // B6 off, G6 on, R6 off
        leds_tester.set_dc_data(6, preset_dc_test_off, preset_dc_test_on, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(77, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(78, result));
        REQUIRE(result == 0b00000111);  
        REQUIRE(leds_tester.get_common_reg_at(79, result));
        REQUIRE(result == 0b11110000); 
        REQUIRE(leds_tester.get_common_reg_at(80, result));
        REQUIRE(result == 0b00000000);  
                
        // B6 off, G6 off, R6 on
        leds_tester.set_dc_data(6, preset_dc_test_off, preset_dc_test_off, preset_dc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(77, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(78, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(79, result));
        REQUIRE(result == 0b00001111); 
        REQUIRE(leds_tester.get_common_reg_at(80, result));
        REQUIRE(result == 0b11100000);  
                
        // B6 + G6 + R6 off
        leds_tester.set_dc_data(6, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(77, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(78, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(79, result));
        REQUIRE(result == 0b00000000); 
        REQUIRE(leds_tester.get_common_reg_at(80, result));
        REQUIRE(result == 0b00000000);  
        
    }

    SECTION("LED5 Dot Correction bits test")
    {

        // ROW #3
        // DC        B7      G7     R7    B6     G6     R6     B5     G5     R5     B4     G4     R4
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ======][======][======][======][======][======][======][======][======][======][====
        //          #75     #76     #77     #78     #79     #80     #81     #82     #83     #84     #85

        // B5 + G5 + R5 off
        leds_tester.set_dc_data(5, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(80, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(81, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(82, result));
        REQUIRE(result == 0b00000000);  

        // B5 on, G5 + R5 off
        leds_tester.set_dc_data(5, preset_dc_test_on, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(80, result));
        REQUIRE(result == 0b00011111);  
        REQUIRE(leds_tester.get_common_reg_at(81, result));
        REQUIRE(result == 0b11000000);  
        REQUIRE(leds_tester.get_common_reg_at(82, result));
        REQUIRE(result == 0b00000000);  

        // B5 off, G5 on, R5 off
        leds_tester.set_dc_data(5, preset_dc_test_off, preset_dc_test_on, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(80, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(81, result));
        REQUIRE(result == 0b00111111);  
        REQUIRE(leds_tester.get_common_reg_at(82, result));
        REQUIRE(result == 0b10000000);  

        // B5 off, G5 off, R5 on
        leds_tester.set_dc_data(5, preset_dc_test_off, preset_dc_test_off, preset_dc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(80, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(81, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(82, result));
        REQUIRE(result == 0b01111111);  

        // B5 + G5 + R5 off
        leds_tester.set_dc_data(5, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(80, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(81, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(82, result)); 
        REQUIRE(result == 0b00000000);  
    }

    SECTION("LED4 Dot Correction bits test")
    {

        // ROW #3
        // DC        B7      G7     R7    B6     G6     R6     B5     G5     R5     B4     G4     R4
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ======][======][======][======][======][======][======][======][======][======][====
        //          #75     #76     #77     #78     #79     #80     #81     #82     #83     #84     #85

        // B4 + G4 + R4 off
        leds_tester.set_dc_data(4, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(83, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(84, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(85, result));
        REQUIRE(result == 0b00000000);  

        // B4 on, G4 + R4 off
        leds_tester.set_dc_data(4, preset_dc_test_on, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(83, result));
        REQUIRE(result == 0b11111110);  
        REQUIRE(leds_tester.get_common_reg_at(84, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(85, result));
        REQUIRE(result == 0b00000000);  

        // B4 off, G4 on, R4 off
        leds_tester.set_dc_data(4, preset_dc_test_off, preset_dc_test_on, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(83, result));
        REQUIRE(result == 0b00000001);  
        REQUIRE(leds_tester.get_common_reg_at(84, result));
        REQUIRE(result == 0b11111100);  
        REQUIRE(leds_tester.get_common_reg_at(85, result));
        REQUIRE(result == 0b00000000);  

        // B4 off, G4 off, R4 on
        leds_tester.set_dc_data(4, preset_dc_test_off, preset_dc_test_off, preset_dc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(83, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(84, result));
        REQUIRE(result == 0b00000011);  
        REQUIRE(leds_tester.get_common_reg_at(85, result));
        REQUIRE(result == 0b11111000);  

        // B4 + G4 + R4 off
        leds_tester.set_dc_data(4, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(83, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(84, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(85, result));
        REQUIRE(result == 0b00000000);  
    }

    SECTION("LED3 Dot Correction bits test")
    {

        // ROW #4
        // DC        B3     G3     R3      B2     G2    R2      B1    G1     R1     B0     G0     R0
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
        //        #85   #86     #87      #88    #89     #90     #91     #92     #93     #94     #95   #96    

        // B3 + G3 + R3 off
        leds_tester.set_dc_data(3, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(85, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(86, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(87, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(88, result));
        REQUIRE(result == 0b00000000);  

        // B3 on, G3 + R3 off
        leds_tester.set_dc_data(3, preset_dc_test_on, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(85, result));
        REQUIRE(result == 0b00000111);  
        REQUIRE(leds_tester.get_common_reg_at(86, result));
        REQUIRE(result == 0b11110000);  
        REQUIRE(leds_tester.get_common_reg_at(87, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(88, result));
        REQUIRE(result == 0b00000000); 

        // B3 off, G3 on, R3 off
        leds_tester.set_dc_data(3, preset_dc_test_off, preset_dc_test_on, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(85, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(86, result));
        REQUIRE(result == 0b00001111);  
        REQUIRE(leds_tester.get_common_reg_at(87, result));
        REQUIRE(result == 0b11100000);  
        REQUIRE(leds_tester.get_common_reg_at(88, result));
        REQUIRE(result == 0b00000000); 

        // B3 off, G3 off, R3 on
        leds_tester.set_dc_data(3, preset_dc_test_off, preset_dc_test_off, preset_dc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(85, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(86, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(87, result));
        REQUIRE(result == 0b00011111);  
        REQUIRE(leds_tester.get_common_reg_at(88, result));
        REQUIRE(result == 0b11000000); 

        // B3 + G3 + R3 off
        leds_tester.set_dc_data(3, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(85, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(86, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(87, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(88, result));
        REQUIRE(result == 0b00000000); 
    }

    SECTION("LED2 Dot Correction bits test")
    {

        // ROW #4
        // DC        B3     G3     R3      B2     G2    R2      B1    G1     R1     B0     G0     R0
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
        //        #85   #86     #87      #88    #89     #90     #91     #92     #93     #94     #95   #96    

        // B2 + G2 + R2 off
        leds_tester.set_dc_data(2, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(88, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(89, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(90, result));
        REQUIRE(result == 0b00000000);  

        // B2 on, G2 + R2 off
        leds_tester.set_dc_data(2, preset_dc_test_on, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(88, result));
        REQUIRE(result == 0b00111111);  
        REQUIRE(leds_tester.get_common_reg_at(89, result));
        REQUIRE(result == 0b10000000);  
        REQUIRE(leds_tester.get_common_reg_at(90, result));
        REQUIRE(result == 0b00000000);  

        // B2 off, G2 on, R2 off
        leds_tester.set_dc_data(2, preset_dc_test_off, preset_dc_test_on, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(88, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(89, result));
        REQUIRE(result == 0b01111111);  
        REQUIRE(leds_tester.get_common_reg_at(90, result));
        REQUIRE(result == 0b00000000);   

        // B2 off, G2 off, R2 on
        leds_tester.set_dc_data(2, preset_dc_test_off, preset_dc_test_off, preset_dc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(88, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(89, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(90, result));
        REQUIRE(result == 0b11111110);   

        // B2 + G2 + R2 off
        leds_tester.set_dc_data(2, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(88, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(89, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(90, result));
        REQUIRE(result == 0b00000000);  
    }    

    SECTION("LED1 Dot Correction bits test")
    {

        // ROW #4
        // DC        B3     G3     R3      B2     G2    R2      B1    G1     R1     B0     G0     R0
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
        //        #85   #86     #87      #88    #89     #90     #91     #92     #93     #94     #95   #96    

        // B1 + G1 + R1 off
        leds_tester.set_dc_data(1, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(90, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(91, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(92, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(93, result));
        REQUIRE(result == 0b00000000);  

        // B1 on, G1 + R1 off
        leds_tester.set_dc_data(1, preset_dc_test_on, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(90, result));
        REQUIRE(result == 0b00000001);  
        REQUIRE(leds_tester.get_common_reg_at(91, result));
        REQUIRE(result == 0b11111100);  
        REQUIRE(leds_tester.get_common_reg_at(92, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(93, result));
        REQUIRE(result == 0b00000000);    

        // B1 off, G1 on, R1 off
        leds_tester.set_dc_data(1, preset_dc_test_off, preset_dc_test_on, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(90, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(91, result));
        REQUIRE(result == 0b00000011);  
        REQUIRE(leds_tester.get_common_reg_at(92, result));
        REQUIRE(result == 0b11111000);  
        REQUIRE(leds_tester.get_common_reg_at(93, result));
        REQUIRE(result == 0b00000000);     

        // B1 off, G1 off, R1 on
        leds_tester.set_dc_data(1, preset_dc_test_off, preset_dc_test_off, preset_dc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(90, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(91, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(92, result));
        REQUIRE(result == 0b00000111);  
        REQUIRE(leds_tester.get_common_reg_at(93, result));
        REQUIRE(result == 0b11110000);    

        // B1 + G1 + R1 off
        leds_tester.set_dc_data(1, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(90, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(91, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(92, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(93, result));
        REQUIRE(result == 0b00000000);   
    }        

    SECTION("LED0 Dot Correction bits test")
    {

        // ROW #4
        // DC        B3     G3     R3      B2     G2    R2      B1    G1     R1     B0     G0     R0
        // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
        // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
        //        #85   #86     #87      #88    #89     #90     #91     #92     #93     #94     #95   #96    

        // B0 + G0 + R0 off
        leds_tester.set_dc_data(0, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(93, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(94, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(95, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(96, result));
        REQUIRE(result == 0b00000000);  

        // B0 on, G0 + R0 off
        leds_tester.set_dc_data(0, preset_dc_test_on, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(93, result));
        REQUIRE(result == 0b00001111);  
        REQUIRE(leds_tester.get_common_reg_at(94, result));
        REQUIRE(result == 0b11100000);  
        REQUIRE(leds_tester.get_common_reg_at(95, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(96, result));
        REQUIRE(result == 0b00000000);  

        // B0 off, G0 on, R0 off
        leds_tester.set_dc_data(0, preset_dc_test_off, preset_dc_test_on, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(93, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(94, result));
        REQUIRE(result == 0b00011111);  
        REQUIRE(leds_tester.get_common_reg_at(95, result));
        REQUIRE(result == 0b11000000);  
        REQUIRE(leds_tester.get_common_reg_at(96, result));
        REQUIRE(result == 0b00000000);      

        // B0 off, G0 off, R0 on
        leds_tester.set_dc_data(0, preset_dc_test_off, preset_dc_test_off, preset_dc_test_on);
        REQUIRE(leds_tester.get_common_reg_at(93, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(94, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(95, result));
        REQUIRE(result == 0b00111111);  
        REQUIRE(leds_tester.get_common_reg_at(96, result));
        REQUIRE(result == 0b10000000);     

        // B0 + G0 + R0 off
        leds_tester.set_dc_data(0, preset_dc_test_off, preset_dc_test_off, preset_dc_test_off);
        REQUIRE(leds_tester.get_common_reg_at(93, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(94, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(95, result));
        REQUIRE(result == 0b00000000);  
        REQUIRE(leds_tester.get_common_reg_at(96, result));
        REQUIRE(result == 0b00000000);   
    }        


}
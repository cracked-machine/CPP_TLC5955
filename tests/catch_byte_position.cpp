#include <catch2/catch_all.hpp>
#include <byte_position.hpp>
#include <array>

TEST_CASE("byte_position")
{
    SECTION("Bit Decrement (default)")
    {
        BytePosition byte_pos(0, 3);

        // bits are decremented
        uint16_t res {0};
               
        REQUIRE((res = byte_pos.next_bit_idx()) == 2);      // Byte:0 Bit:2
        REQUIRE(byte_pos.get_byte_idx() == 0);
                     
        REQUIRE((res = byte_pos.next_bit_idx()) == 1);      // Byte:0 Bit:1
        REQUIRE(byte_pos.get_byte_idx() == 0);
                       
        REQUIRE((res = byte_pos.next_bit_idx()) == 0);      // Byte:0 Bit:0
        REQUIRE(byte_pos.get_byte_idx() == 0);

        // wrap around
                     
        REQUIRE((res = byte_pos.next_bit_idx()) == 2);      // Byte:1 Bit:2
        REQUIRE(byte_pos.get_byte_idx() == 1);
                     
        REQUIRE((res = byte_pos.next_bit_idx()) == 1);      // Byte:1 Bit:1
        REQUIRE(byte_pos.get_byte_idx() == 1);
                      
        REQUIRE((res = byte_pos.next_bit_idx()) == 0);      // Byte:1 Bit:0
        REQUIRE(byte_pos.get_byte_idx() == 1);

        // wrap around
                              
        REQUIRE((res = byte_pos.next_bit_idx()) == 2);      // Byte:2 Bit:2
        REQUIRE(byte_pos.get_byte_idx() == 2);
        
        REQUIRE((res = byte_pos.next_bit_idx()) == 1);      // Byte:2 Bit:1
        REQUIRE(byte_pos.get_byte_idx() == 2);
                     
        REQUIRE((res = byte_pos.next_bit_idx()) == 0);      // Byte:2 Bit:0
        REQUIRE(byte_pos.get_byte_idx() == 2);

    }

}
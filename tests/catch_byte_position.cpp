#include <catch2/catch_all.hpp>
#include <byte_position.hpp>
#include <array>

TEST_CASE("byte_position")
{
    SECTION("Bit Decrement (default)")
    {
        BytePosition byte_pos(0, 6);

        // bits are decremented
        REQUIRE(byte_pos.get_byte_idx() == 0);
        REQUIRE(byte_pos.has_next());
        REQUIRE(byte_pos.next_bit_idx() == 6);
        REQUIRE(byte_pos.get_byte_idx() == 0);
        REQUIRE(byte_pos.has_next());
        REQUIRE(byte_pos.next_bit_idx() == 5);
        REQUIRE(byte_pos.get_byte_idx() == 0);
        REQUIRE(byte_pos.has_next());
        REQUIRE(byte_pos.next_bit_idx() == 4);
        REQUIRE(byte_pos.get_byte_idx() == 0);
        REQUIRE(byte_pos.has_next());
        REQUIRE(byte_pos.next_bit_idx() == 3);
        REQUIRE(byte_pos.get_byte_idx() == 0);
        REQUIRE(byte_pos.has_next());
        REQUIRE(byte_pos.next_bit_idx() == 2);
        REQUIRE(byte_pos.get_byte_idx() == 0);
        REQUIRE(byte_pos.has_next());
        REQUIRE(byte_pos.next_bit_idx() == 1);
        REQUIRE(byte_pos.get_byte_idx() == 0);
        REQUIRE(byte_pos.has_next());
        REQUIRE(byte_pos.next_bit_idx() == 0);
        REQUIRE_FALSE(byte_pos.has_next());

        // increment the byte resets the bit 
        REQUIRE(byte_pos++ == 1);
        REQUIRE(byte_pos.next_bit_idx() == 6);

    }

    SECTION("Bit Decrement (default) - zero index exception")
    {
        BytePosition byte_pos(0, 6);

        // bits are decremented
        REQUIRE(byte_pos.get_byte_idx() == 0);
        REQUIRE(byte_pos.has_next());
        REQUIRE(byte_pos.next_bit_idx() == 6);
        REQUIRE(byte_pos.get_byte_idx() == 0);
        REQUIRE(byte_pos.has_next());
        REQUIRE(byte_pos.next_bit_idx() == 5);
        REQUIRE(byte_pos.get_byte_idx() == 0);
        REQUIRE(byte_pos.has_next());
        REQUIRE(byte_pos.next_bit_idx() == 4);
        REQUIRE(byte_pos.get_byte_idx() == 0);
        REQUIRE(byte_pos.has_next());
        REQUIRE(byte_pos.next_bit_idx() == 3);
        REQUIRE(byte_pos.get_byte_idx() == 0);
        REQUIRE(byte_pos.has_next());
        REQUIRE(byte_pos.next_bit_idx() == 2);
        REQUIRE(byte_pos.get_byte_idx() == 0);
        REQUIRE(byte_pos.has_next());
        REQUIRE(byte_pos.next_bit_idx() == 1);
        REQUIRE(byte_pos.get_byte_idx() == 0);
        REQUIRE(byte_pos.has_next());
        REQUIRE(byte_pos.next_bit_idx() == 0);
        REQUIRE_FALSE(byte_pos.has_next());
        
        // decrement on zero bit position should throw exception
        REQUIRE_THROWS(byte_pos.next_bit_idx() == 0);
  
    }
}
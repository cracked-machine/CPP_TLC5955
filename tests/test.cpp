


#include "../../build/catch.hpp"
#include <iostream>
#include <tlc5955.hpp>

TEST_CASE("TestTLC5955 part 2", "[Catch2]")
{

    tlc5955::Driver leds;
    REQUIRE_FALSE(leds.startup_tests());
    
}
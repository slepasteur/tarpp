#include <catch/catch.hpp>
#include <tarpp/tar.h>
#include <cstring>

TEST_CASE("Octal format string are generated correctly.", "[format]")
{
	using namespace tarpp::format;

	REQUIRE(details::format_octal_zero_filled_helper<7>::value == std::string{"%07o"});
}

TEST_CASE("Octal values are formatted correctly.", "[format]")
{
	using namespace tarpp::format;

	char buffer[4];
    SECTION ("5")
    {
        auto result = format_octal(buffer, 5);
        CHECK(result == 3);
        CHECK(buffer == std::string{"005"});
    }

    SECTION ("8")
    {
        auto result = format_octal(buffer, 8);
        CHECK(result == 3);
        CHECK(buffer == std::string{"010"});
    }

    SECTION ("When output would be too long it is truncated.")
    {
        auto result = format_octal(buffer, 1234);
        CHECK(result > 3);
        CHECK(std::strlen(buffer) == 3);
    }
}
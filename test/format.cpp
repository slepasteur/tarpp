//
// Created by simon on 6/22/16.
//

#include <catch/catch.hpp>
#include <tarpp/tar.h>

TEST_CASE("Octal format string are generated correctly.")
{
	using namespace tarpp::format;

	constexpr auto format_string = format_octal_zero_filled_helper<7>::value;
	REQUIRE(format_string == std::string{"%07o"});
}

TEST_CASE("Octal values are formatted correctly.")
{
	using namespace tarpp::format;

	char buffer[4];
	format_octal(buffer, 5);
	REQUIRE(buffer == std::string{"005"});
}
#include <catch/catch.hpp>
#include <tarpp/tar.h>
#include <cstring>

TEST_CASE("Octal format string are generated correctly.", "[format]")
{
    using namespace tarpp::format;

    REQUIRE(details::octal_format_string_t<7>::value == std::string{"%07o"});
    REQUIRE(details::octal_format_string_t<15>::value == std::string{"%015o"});
    REQUIRE(details::octal_format_string_t<1234>::value == std::string{"%01234o"});
}

TEST_CASE("Octal values are formatted correctly.", "[format]")
{
    using namespace tarpp::format;

    char buffer[4];
    SECTION ("5") {
        auto result = format_octal(buffer, 5);
        CHECK(result == 3);
        CHECK(buffer == std::string{"005"});
    }

    SECTION ("8") {
        auto result = format_octal(buffer, 8);
        CHECK(result == 3);
        CHECK(buffer == std::string{"010"});
    }

    SECTION ("When output would be too long it is truncated.") {
        auto result = format_octal(buffer, 01234);
        CHECK(result == 4);
        CHECK(buffer == std::string{"123"});
    }
}

TEST_CASE("Non null terminated octal values are formatted correctly.", "[format]")
{
    using namespace tarpp::format;

    char buffer[4];
    SECTION ("5") {
        auto result = format_octal_no_null(buffer, 5);
        auto expected = std::string{"0005"};
        CHECK(result == 4);
        CHECK(std::equal(buffer, buffer + 4, expected.begin()));
    }

    SECTION ("8") {
        auto result = format_octal_no_null(buffer, 8);
        auto expected = std::string{"0010"};
        CHECK(result == 4);
        CHECK(std::equal(buffer, buffer + 4, expected.begin()));
    }

    SECTION ("When output would be too long it is truncated.") {
        auto result = format_octal_no_null(buffer, 012345);
        auto expected = std::string{"1234"};
        CHECK(result == 5);
        CHECK(std::equal(buffer, buffer + 4, expected.begin()));
    }
}
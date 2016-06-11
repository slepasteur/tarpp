//
// Created by simon on 6/20/16.
//

#include "catch/catch.hpp"
#include <sstream>

#include <tarpp/tar.h>

using namespace tarpp;

TEST_CASE("A tar file can be created from a ostream.", "[tar][create]")
{
	auto out = std::stringstream{};
	REQUIRE_NOTHROW(Tar{out});
}

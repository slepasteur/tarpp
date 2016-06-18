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

TEST_CASE("Adding content to the tar file increases its size.", "[tar][add]")
{
	auto out = std::stringstream{};
	auto tar = Tar{out};

	auto content = std::string{"content"};
	tar.add("name", content);

	REQUIRE(out.str().size() == details::constants::HEADER_SIZE + content.size());
}

TEST_CASE("Tar header starts with name.", "[tar][header]")
{
	auto out = std::stringstream{};
	auto tar = Tar{out};

	auto content = std::string{"content"};
	auto name = std::string{"name"};
	tar.add(name, content);

	auto result = out.str();
	REQUIRE(result.size() >= name.size());
	INFO(result);
	REQUIRE(std::equal(name.begin(), name.end(), result.begin()));
}
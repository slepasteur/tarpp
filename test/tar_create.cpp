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
	REQUIRE(std::equal(name.begin(), name.end(), result.begin()));
}

TEST_CASE("Tar header uses USTAR format (version 0).", "[tar][header]")
{
	using std::begin; using std::end;
	constexpr const char USTAR_MAGIC[] = "ustar";
	constexpr const char USTAR_VERSION[] = {'0', '0'};

	auto out = std::stringstream{};
	auto tar = Tar{out};

	auto content = std::string{"content"};
	auto name = std::string{"name"};
	tar.add(name, content);

	auto result = out.str();
	REQUIRE(result.size() >= name.size());
	REQUIRE(std::equal(begin(USTAR_MAGIC), end(USTAR_MAGIC), result.begin() + details::constants::HEADER_MAGIC_OFFSET));
	REQUIRE(std::equal(begin(USTAR_VERSION), end(USTAR_VERSION), result.begin() + details::constants::HEADER_VERSION_OFFSET));
}
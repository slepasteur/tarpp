#include "catch/catch.hpp"
#include <sstream>

#include <tarpp/tar.h>
#include <iostream>
#include <unistd.h>

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

    REQUIRE(out.str().size() >= details::constants::HEADER_SIZE + content.size());
}

TEST_CASE("Tar size is a multiple of block size.", "[tar][add]")
{
    auto out = std::stringstream{};
    auto tar = Tar{out};

    auto content = std::string{"content"};
    tar.add("name", content);

    REQUIRE(out.str().size() % details::constants::BLOCK_SIZE == 0);
}

void require_header_content(const char *expected, const std::string &data, int offset, int length)
{
    auto field_begin = std::next(data.begin(), offset);
    auto field_end = std::next(field_begin, length);
    auto field = std::string{field_begin, field_end};
    INFO(field << " (expected: " << expected << ")");
    REQUIRE(std::equal(field_begin, field_end, expected));
}

TEST_CASE("Tar header.", "[tar][header]")
{
    using std::begin; using std::end; using namespace details::constants;
    auto out = std::stringstream{};
    auto tar = Tar{out};

    SECTION("Default values.") {
        auto name = std::string{"name"};
        auto content = std::string{"content"};
        tar.add(name, content);
        auto result = out.str();

        REQUIRE(result.size() > HEADER_SIZE);

        SECTION("Header starts with name.") {
            REQUIRE(result.size() >= name.size());
            REQUIRE(std::equal(name.begin(), name.end(), result.begin()));
        }

        SECTION("Header uses USTAR format (version 0).") {
            constexpr const char USTAR_MAGIC[] = "ustar";
            constexpr const char USTAR_VERSION[] = {'0', '0'};
            REQUIRE(std::equal(begin(USTAR_MAGIC), end(USTAR_MAGIC),
                               std::next(result.begin(), HEADER_MAGIC_OFFSET)));
            REQUIRE(std::equal(begin(USTAR_VERSION), end(USTAR_VERSION),
                               std::next(result.begin(), HEADER_VERSION_OFFSET)));
        }

        SECTION("Header mode is set.") {
            require_header_content("0000400", result, HEADER_MODE_OFFSET, HEADER_MODE_SIZE);
        }

        SECTION("User ID is set.") {
            char user_id[details::constants::HEADER_UID_SIZE] = {};
            format::format_octal(user_id, getuid());
            require_header_content(user_id, result, HEADER_UID_OFFSET, HEADER_UID_SIZE);
        }

        SECTION("Group ID is set.") {
            char group_id[details::constants::HEADER_GID_SIZE] = {};
            format::format_octal(group_id, getgid());
            require_header_content(group_id, result, HEADER_GID_OFFSET, HEADER_GID_SIZE);
        }

        SECTION("Size is set.") {
            char size[details::constants::HEADER_SIZE_SIZE + 1] = {};
            format::format_octal(size, content.size());
            require_header_content(size, result, HEADER_SIZE_OFFSET, HEADER_SIZE_SIZE);
        }

        SECTION("Modification time is set.") {
            char mtime[details::constants::HEADER_MTIME_SIZE + 1] = {};
            format::format_octal(mtime, details::DEFAULT_TIME());
            require_header_content(mtime, result, HEADER_MTIME_OFFSET, HEADER_MTIME_SIZE);
        }
    }

    SECTION("Specifying the file mode.") {
        tar.add("name", "content", TarFileOptions{}.with_mode(S_IRWXU | S_IRWXG | S_IRWXO));
        auto result = out.str();

        SECTION("Header mode is set.") {
            require_header_content("0000777", result, HEADER_MODE_OFFSET, HEADER_MODE_SIZE);
        }
    }

    SECTION("Specifying the user ID.") {
        tar.add("name", "content", TarFileOptions{}.with_uid(1));
        auto result = out.str();

        SECTION("User ID is set.") {
            require_header_content("0000001", result, HEADER_UID_OFFSET, HEADER_UID_SIZE);
        }
    }

    SECTION("Specifying the group ID.") {
        tar.add("name", "content", TarFileOptions{}.with_gid(1));
        auto result = out.str();

        SECTION("Group ID is set.") {
            require_header_content("0000001", result, HEADER_GID_OFFSET, HEADER_GID_SIZE);
        }
    }

    SECTION("Adding a big content.") {
        auto content = std::string(3000000, 'a');
        tar.add("name", content, TarFileOptions{}.with_gid(1));
        auto result = out.str();

        SECTION("The size is set accordingly.") {
            char size[details::constants::HEADER_SIZE_SIZE + 1] = {};
            format::format_octal(size, content.size());
            require_header_content(size, result, HEADER_SIZE_OFFSET, HEADER_SIZE_SIZE);
        }
    }

    SECTION("Specifying the modification time.") {
        auto t = time(nullptr);
        tar.add("name", "content", TarFileOptions{}.with_mtime(t));
        auto result = out.str();

        SECTION("Modification time is set.") {
            char mtime[details::constants::HEADER_MTIME_SIZE + 1] = {};
            format::format_octal(mtime, t);
            require_header_content(mtime, result, HEADER_MTIME_OFFSET, HEADER_MTIME_SIZE);
        }
    }
}

TEST_CASE("Tar contains content after header", "[tar][content]")
{
    using std::begin; using std::end;

    auto out = std::stringstream{};
    auto tar = Tar{out};

    auto content = std::string{"content"};
    auto name = std::string{"name"};
    tar.add(name, content);

    auto result = out.str();
    REQUIRE(result.size() >= details::constants::HEADER_SIZE + content.size());

    auto tar_content_begin = std::next(result.begin(), details::constants::HEADER_SIZE);
    auto tar_content = std::string{tar_content_begin, std::next(tar_content_begin, content.size())};
    REQUIRE(tar_content == content);
}
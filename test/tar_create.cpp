#include "catch/catch.hpp"
#include <sstream>
#include <iostream>
#include <unistd.h>

#include "tarpp/tar.h"
#include "tarpp/user.h"

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

        SECTION("Type of file is set.") {
            char type = static_cast<char>(details::DEFAULT_TYPE());
            REQUIRE(result[HEADER_TYPE_OFFSET] == type);
        }

        SECTION("Link name is set.") {
            char linkname[HEADER_LINKNAME_SIZE] = {};
            require_header_content(linkname, result, HEADER_LINKNAME_OFFSET, HEADER_LINKNAME_SIZE);
        }

        SECTION("User name is set.") {
            char user_name[details::constants::HEADER_UNAME_SIZE] = {};
            format::format_string(user_name, user::get_user_name(getuid()));
            require_header_content(user_name, result, HEADER_UNAME_OFFSET, HEADER_UNAME_SIZE);
        }

        SECTION("Group name is set.") {
            char group_name[details::constants::HEADER_GNAME_SIZE] = {};
            format::format_string(group_name, user::get_group_name(getgid()));
            require_header_content(group_name, result, HEADER_GNAME_OFFSET, HEADER_GNAME_SIZE);
        }
    }

    SECTION("Specifying the name.") {
        auto name = std::string{"another name.tar.gz"};
        REQUIRE(name.size() <= HEADER_NAME_SIZE);
        tar.add(name, "content");
        auto result = out.str();

        SECTION("Name is set.") {
            char expected_name_field[HEADER_NAME_SIZE] = {};
            std::copy(name.begin(), name.end(), expected_name_field);
            require_header_content(expected_name_field, result, HEADER_NAME_OFFSET, HEADER_NAME_SIZE);
        }
    }

    SECTION("Specifying the maximum name size.") {
        auto name = std::string(HEADER_NAME_SIZE, 'z');
        REQUIRE(name.size() == HEADER_NAME_SIZE);
        tar.add(name, "content");
        auto result = out.str();

        SECTION("Name is complete but not null-terminated.") {
            require_header_content(name.c_str(), result, HEADER_NAME_OFFSET, HEADER_NAME_SIZE);
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

    SECTION("Specifying the user name.") {
        auto username = std::string{"john"};
        tar.add("name", "content", TarFileOptions{}.with_username(username));
        auto result = out.str();

        SECTION("User name is set.") {
            char expected[HEADER_UNAME_SIZE] = {};
            format::format_string(expected, username);
            require_header_content(expected, result, HEADER_UNAME_OFFSET, HEADER_UNAME_SIZE);
        }
    }

    SECTION("Specifying the group ID.") {
        tar.add("name", "content", TarFileOptions{}.with_gid(1));
        auto result = out.str();

        SECTION("Group ID is set.") {
            require_header_content("0000001", result, HEADER_GID_OFFSET, HEADER_GID_SIZE);
        }
    }

    SECTION("Specifying the group name.") {
        auto groupname = std::string{"users"};
        tar.add("name", "content", TarFileOptions{}.with_groupname(groupname));
        auto result = out.str();

        SECTION("Group name is set.") {
            char expected[HEADER_GNAME_SIZE] = {};
            format::format_string(expected, groupname);
            require_header_content(expected, result, HEADER_GNAME_OFFSET, HEADER_GNAME_SIZE);
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

    SECTION("Specifying the type of file.") {
        tar.add("name", "content", TarFileOptions{}.with_type(FileType::DIRECTORY));
        auto result = out.str();

        SECTION("Type of file is set.") {
            REQUIRE(result[HEADER_TYPE_OFFSET] == '5');
        }
    }

    SECTION("Specifying the name of the linked file.") {
        tar.add("name", "content", TarFileOptions{}.with_linkname("another_file.txt"));
        auto result = out.str();

        SECTION("Link name is set.") {
            char linkname[HEADER_LINKNAME_SIZE] = {};
            format::format_string(linkname, "another_file.txt");
            require_header_content(linkname, result, HEADER_LINKNAME_OFFSET, HEADER_LINKNAME_SIZE);
        }
    }

    SECTION("Specifying the name of the linked file of maximum length.") {
        auto linkname = std::string(HEADER_LINKNAME_SIZE, 'x');
        tar.add("name", "content", TarFileOptions{}.with_linkname(linkname));
        auto result = out.str();

        SECTION("Link name is set.") {
            require_header_content(linkname.c_str(), result, HEADER_LINKNAME_OFFSET, HEADER_LINKNAME_SIZE);
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
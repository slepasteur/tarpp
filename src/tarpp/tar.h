//
// Created by simon on 6/20/16.
//

#ifndef TAR_TAR_H
#define TAR_TAR_H

#include <ostream>
#include <unistd.h>
#include <sys/stat.h>

#include "format.h"

namespace tarpp {

namespace details {
namespace constants {

enum
{
    BLOCK_SIZE = 512,

    HEADER_SIZE = 512,

    HEADER_NAME_SIZE = 100,
    HEADER_MODE_SIZE = 8,
    HEADER_UID_SIZE = 8,
    HEADER_GID_SIZE = 8,
    HEADER_SIZE_SIZE = 12,
    HEADER_MTIME_SIZE = 12,
    HEADER_CHKSUM_SIZE = 8,
    HEADER_TYPE_SIZE = 1,
    HEADER_MAGIC_SIZE = 6,
    HEADER_VERSION_SIZE = 2,
    HEADER_UNAME_SIZE = 32,
    HEADER_GNAME_SIZE = 32,
    HEADER_DEVMAJOR_SIZE = 8,
    HEADER_DEVMINOR_SIZE = 8,
    HEADER_PREFIX_SIZE = 155,

    HEADER_NAME_OFFSET = 0,
    HEADER_MODE_OFFSET = HEADER_NAME_OFFSET + HEADER_NAME_SIZE,
    HEADER_UID_OFFSET = HEADER_MODE_OFFSET + HEADER_MODE_SIZE,
    HEADER_GID_OFFSET = HEADER_UID_OFFSET + HEADER_UID_SIZE,
    HEADER_SIZE_OFFSET = HEADER_GID_OFFSET + HEADER_GID_SIZE,
    HEADER_MTIME_OFFSET = HEADER_SIZE_OFFSET + HEADER_SIZE_SIZE,
    HEADER_CHKSUM_OFFSET = HEADER_MTIME_OFFSET + HEADER_MTIME_SIZE,
    HEADER_TYPE_OFFSET = HEADER_CHKSUM_OFFSET + HEADER_CHKSUM_SIZE,
    HEADER_LINKNAME_OFFSET = HEADER_TYPE_OFFSET + HEADER_TYPE_SIZE,

    HEADER_MAGIC_OFFSET = HEADER_LINKNAME_OFFSET + HEADER_NAME_SIZE,
    HEADER_VERSION_OFFSET = HEADER_MAGIC_OFFSET + HEADER_MAGIC_SIZE,
    HEADER_UNAME_OFFSET = HEADER_VERSION_OFFSET + HEADER_VERSION_SIZE,
    HEADER_GNAME_OFFSET = HEADER_UNAME_OFFSET + HEADER_UNAME_SIZE,
    HEADER_DEVMAJOR_OFFSET = HEADER_GNAME_OFFSET + HEADER_GNAME_SIZE,
    HEADER_DEVMINOR_OFFSET = HEADER_DEVMAJOR_OFFSET + HEADER_DEVMAJOR_SIZE,
    HEADER_PREFIX_OFFSET = HEADER_DEVMINOR_OFFSET + HEADER_DEVMINOR_SIZE
};

} // constants

struct TarHeader
{
    TarHeader()
        :
        data_{0}
    {
        format::format_string(header_.magic_, "ustar");
        header_.version_[0] = '0';
        header_.version_[1] = '0';
    }

    union
    {
        char data_[constants::HEADER_SIZE];
        struct
        {
            //Pre-POSIX.1-1988 (i.e. v7)
            char name_[constants::HEADER_NAME_SIZE];            // File name
            char mode_[constants::HEADER_MODE_SIZE];            // File mode
            char uid_[constants::HEADER_UID_SIZE];              // Owner's numeric user ID
            char gid_[constants::HEADER_GID_SIZE];              // Group's numeric user ID
            char size_[constants::HEADER_SIZE_SIZE];            // File size in bytes (octal base - not null terminated)
            char mtime_[constants::HEADER_MTIME_SIZE];          // Last modification time in numeric Unix time format (octal - not null terminated)
            char chksum_[constants::HEADER_CHKSUM_SIZE];        // Checksum for header record
            char type_[constants::HEADER_TYPE_SIZE];            // Link indicator (file type)
            char linkname_[constants::HEADER_NAME_SIZE];        // Name of linked file

            //POSIX IEEE P1003.1 (UStar)
            char magic_[constants::HEADER_MAGIC_SIZE];          // UStar indicator "ustar" then NUL
            char version_[constants::HEADER_VERSION_SIZE];      // UStar version "00"
            char uname_[constants::HEADER_UNAME_SIZE];          // Owner user name
            char gname_[constants::HEADER_GNAME_SIZE];          // Owner group name
            char devmajor_[constants::HEADER_DEVMAJOR_SIZE];    // Device major number
            char devminor_[constants::HEADER_DEVMINOR_SIZE];    // Device minor number
            char prefix_[constants::HEADER_PREFIX_SIZE];        // Filename prefix
        } header_;
    };
};

inline constexpr mode_t DEFAULT_MODE() { return S_IRUSR; }

inline constexpr time_t DEFAULT_TIME() { return 0; }

} // details

class TarFileOptions
{
public:
    TarFileOptions() :
        TarFileOptions(
            details::DEFAULT_MODE(),
            getuid(),
            getgid(),
            details::DEFAULT_TIME()
        ) {}

    TarFileOptions(mode_t mode, uid_t uid, gid_t gid, time_t mtime) :
        mode_(mode),
        uid_(uid),
        gid_(gid),
        mtime_(mtime) {}

    mode_t mode() const { return mode_; }
    uid_t uid() const { return uid_; }
    gid_t gid() const { return gid_; }
    time_t mtime() const { return mtime_; }

    TarFileOptions with_mode(mode_t m) const
    {
        return {m, uid_, gid_, mtime_};
    }

    TarFileOptions with_uid(uid_t u) const
    {
        return {mode_, u, gid_, mtime_};
    }

    TarFileOptions with_gid(gid_t g) const
    {
        return {mode_, uid_, g, mtime_};
    }

    TarFileOptions with_mtime(time_t t) const
    {
        return {mode_, uid_, gid_, t};
    }

private:
    mode_t mode_;
    uid_t uid_;
    gid_t gid_;
    time_t mtime_;
};

class Tar
{
    static_assert(sizeof(details::TarHeader) == details::constants::HEADER_SIZE, "Invalid tar header size.");
public:
    explicit Tar(std::ostream &output) :
        output_{output}
    {
    }

    void add(const std::string &tar_name, const std::string &content, const TarFileOptions &options = TarFileOptions{})
    {
        using namespace details::constants;
        using namespace details;
        using namespace format;

        auto header = TarHeader{};
        format_string(header.header_.name_, tar_name);
        format_octal(header.header_.mode_, options.mode());
        format_octal(header.header_.uid_, options.uid());
        format_octal(header.header_.gid_, options.gid());
        format_octal_no_null(header.header_.size_, content.size());
        format_octal_no_null(header.header_.mtime_, options.mtime());

        output_.write(header.data_, HEADER_SIZE);
        output_ << content;

        auto padding_size = BLOCK_SIZE - (content.size() % BLOCK_SIZE);
        if (padding_size != BLOCK_SIZE) {
            std::fill_n(std::ostream_iterator<char>(output_), padding_size, 0);
        }
    }

private:
    std::ostream &output_;
};

}

#endif //TAR_TAR_H

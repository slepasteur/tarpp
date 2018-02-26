//
// Created by simon on 6/20/16.
//

#ifndef TAR_TAR_H
#define TAR_TAR_H

#include <ostream>
#include <sys/stat.h>

#include "format.h"

namespace tarpp
{

namespace details
{
namespace constants
{

enum
{
	HEADER_SIZE = 512,
    BLOCK_SIZE = 512,

	HEADER_NAME_OFFSET = 0,
	HEADER_MODE_OFFSET = 100,
	HEADER_UID_OFFSET = 108,
	HEADER_GID_OFFSET = 116,
	HEADER_SIZE_OFFSET = 124,
	HEADER_MTIME_OFFSET = 136,
	HEADER_CHKSUM_OFFSET = 148,
	HEADER_TYPE_OFFSET = 156,
	HEADER_LINKNAME_OFFSET = 157,

	HEADER_MAGIC_OFFSET = 257,
	HEADER_VERSION_OFFSET = 263,
	HEADER_UNAME_OFFSET = 265,
	HEADER_GNAME_OFFSET = 297,
	HEADER_DEVMAJOR_OFFSET = 329,
	HEADER_DEVMINOR_OFFSET = 337,
	HEADER_PREFIX_OFFSET = 345
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
			char name_[100];         // File name
			char mode_[8];           // File mode
			char uid_[8];            // Owner's numeric user ID
			char gid_[8];            // Group's numeric user ID
			char size_[12];          // File size in bytes (octal base - not null terminated)
			char mtime_[12];         // Last modification time in numeric Unix time format (octal - not null terminated)
			char chksum_[8];         // Checksum for header record
			char type_[1];           // Link indicator (file type)
			char linkname_[100];     // Name of linked file

			//POSIX IEEE P1003.1 (UStar)
			char magic_[6];          // UStar indicator "ustar" then NUL
			char version_[2];        // UStar version "00"
			char uname_[32];         // Owner user name
			char gname_[32];         // Owner group name
			char devmajor_[8];       // Device major number
			char devminor_[8];       // Device minor number
			char prefix_[155];       // Filename prefix
		} header_;
	};
};

} // details

class Tar
{
	static_assert(sizeof(details::TarHeader) == details::constants::HEADER_SIZE, "Invalid tar header size.");
public:
	explicit Tar(std::ostream &output):
		output_{output}
	{
	}

	void add(const std::string tar_name, const std::string &content, mode_t mode = S_IRUSR)
	{
		using namespace details::constants;
		using namespace details;
		using namespace format;

		auto header = TarHeader{};
		format_string(header.header_.name_, tar_name);
		format_octal(header.header_.mode_, mode);

		output_.write(header.data_, HEADER_SIZE);
		output_ << content;

        auto padding_size = BLOCK_SIZE - (content.size() % BLOCK_SIZE);
        if (padding_size != BLOCK_SIZE)
        {
            std::fill_n(std::ostream_iterator<char>(output_), padding_size, 0);
        }
	}

private:
	std::ostream &output_;
};

}

#endif //TAR_TAR_H

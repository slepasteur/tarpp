//
// Created by simon on 6/20/16.
//

#ifndef TAR_TAR_H
#define TAR_TAR_H

#include <ostream>

namespace tarpp
{

namespace details
{

namespace constants
{

enum { HEADER_SIZE = 512 };

}

struct TarHeader
{
	TarHeader():
		data_{0}
	{}

	union
	{
		char data_[constants::HEADER_SIZE];
		struct
		{
			char name[100];
		} header_;
	};
};

template <size_t LENGTH>
int format_string(char (&buffer)[LENGTH], const std::string& content)
{
	snprintf(buffer, LENGTH, "%s", content.c_str());
}

}

class Tar
{
	static_assert(sizeof(details::TarHeader) == details::constants::HEADER_SIZE, "Invalid tar header size.");
public:
	explicit Tar(std::ostream& output):
		output_{output}
	{
	}

	void add(const std::string tar_name, const std::string& content)
	{
		using namespace details::constants;
		using namespace details;
		auto header = TarHeader{};
		format_string(header.header_.name, tar_name);
		output_.write(header.data_, HEADER_SIZE);
		output_ << content;
	}

private:
	std::ostream& output_;
};

}

#endif //TAR_TAR_H

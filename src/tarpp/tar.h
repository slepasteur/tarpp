//
// Created by simon on 6/20/16.
//

#ifndef TAR_TAR_H
#define TAR_TAR_H

#include <ostream>

namespace tarpp
{

class Tar
{
public:
	enum { HEADER_SIZE = 512 };

	Tar(std::ostream& output):
		output_{output}
	{
	}

	void add(const std::string tar_name, const std::string& content)
	{
		char header[HEADER_SIZE];
		output_.write(header, HEADER_SIZE);
		output_ << content;
	}

private:
	std::ostream& output_;
};

}

#endif //TAR_TAR_H

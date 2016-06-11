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
	Tar(std::ostream& output):
		output_{output}
	{
	}

private:
	std::ostream& output_;
};

}

#endif //TAR_TAR_H

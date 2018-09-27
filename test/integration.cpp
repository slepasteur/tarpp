#include "catch/catch.hpp"

#include "tarpp/tar.h"
#include "tarball.h"

#include <fstream>
#include <sstream>

using namespace tarpp;

TEST_CASE("Create a tar file in tmp.", "[.][tar][create]")
{
    auto out = std::ofstream("/tmp/test.tar");
    auto t = Tar{out};

    t.add("test", "content", TarFileOptions().with_mtime(time(NULL)));
}
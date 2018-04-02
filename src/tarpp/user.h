#pragma once

#ifndef TAR_USER_H
#define TAR_USER_H

#include <string>
#include <pwd.h>
#include <grp.h>

namespace tarpp {
namespace user {

inline std::string get_user_name(uid_t uid)
{
    static constexpr size_t BUFFER_SIZE = 16384;
    passwd pwd{};
    passwd *result;
    char buffer[BUFFER_SIZE];

    getpwuid_r(uid, &pwd, buffer, BUFFER_SIZE, &result);
    if (result == NULL) {
        return "ERROR";
    }

    return pwd.pw_name;
}

inline std::string get_group_name(gid_t gid)
{
    static constexpr size_t BUFFER_SIZE = 16384;
    group grp{};
    group* result;
    char buffer[BUFFER_SIZE];

    getgrgid_r(gid, &grp, buffer, BUFFER_SIZE, &result);
    if (result == NULL) {
        return "ERROR";
    }

    return grp.gr_name;
}

}} // tarpp::user

#endif //TAR_USER_H

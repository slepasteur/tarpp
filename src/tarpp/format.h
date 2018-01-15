//
// Created by simon on 6/22/16.
//

#ifndef TAR_FORMAT_H
#define TAR_FORMAT_H

namespace tarpp
{
namespace format
{

namespace details
{

template<size_t N> struct last_digit_char {
    enum { value = static_cast<char>('0' + N % 10) };
};

/**
 * A type containing a char array filled with the values of its template arguments.
 */
template <char... cs>
struct octal_format_string_holder
{
    static constexpr const char value[sizeof...(cs)] = { cs... };
};
template <char... cs>
constexpr const char octal_format_string_holder<cs...>::value[sizeof...(cs)];

template<size_t N, template<size_t> class F, char... args>
struct generate_octal_format_string {
    using type = typename generate_octal_format_string<N/10, F, F<N>::value, args...>::type;
};

template<template<size_t> class F, char... args>
struct generate_octal_format_string<0, F, args...> {
    using type = octal_format_string_holder<'%', '0', args..., 'o', '\0'>;
};

/**
 * A type containing the null terminated format string required to format a number as
 * a 0 filled octal string for a buffer of size N.
 * e.g.: "%012o" if N = 12.
 * @tparam N The size of the buffer.
 */
template<size_t N>
struct octal_format_string {
    typedef typename generate_octal_format_string<N, last_digit_char>::type type;
};

template<size_t N>
using octal_format_string_t = typename octal_format_string<N>::type;

}

/**
 * Print a integral type into the given buffer as a '0' filled octal value.
 * @return Number of characters that would have been written for a sufficiently large buffer if successful (not including
 * the terminating null character), or a negative value if an error occurred.
 */
template <typename T, size_t LENGTH>
int format_octal(char (&buffer)[LENGTH], T value)
{
	static_assert(LENGTH > 0, "Invalid buffer length.");
	static_assert(std::is_integral<T>::value, "Only integral types can be formatted as octal.");
	return snprintf(buffer, LENGTH, details::octal_format_string_t<LENGTH - 1>::value, value);
}

/**
 * Print the content of a string into the given buffer.
 * @return Number of characters that would have been written for a sufficiently large buffer if successful (not including
 * the terminating null character), or a negative value if an error occurred.
 */
template <size_t LENGTH>
int format_string(char (&buffer)[LENGTH], const char* content)
{
	static_assert(LENGTH > 0, "Invalid buffer length.");
	return snprintf(buffer, LENGTH, "%s", content);
}

template <size_t LENGTH>
int format_string(char (&buffer)[LENGTH], const std::string& content)
{
	return format_string(buffer, content.c_str());
}

} // format
} // tarpp

#endif //TAR_FORMAT_H

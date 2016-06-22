//
// Created by simon on 6/22/16.
//

#ifndef TAR_FORMAT_H
#define TAR_FORMAT_H

namespace tarpp
{
namespace format
{

template <uint8_t LENGTH>
struct format_octal_zero_filled_helper
{
	static constexpr const char value[] = { '%', '0', '0' + LENGTH, 'o', '\0' };
};
template <uint8_t LENGTH>
constexpr const char format_octal_zero_filled_helper<LENGTH>::value[];

template <typename T, size_t LENGTH>
int format_octal(char (&buffer)[LENGTH], T value)
{
	static_assert(LENGTH > 0, "Invalid buffer length.");
	static_assert(std::is_integral<T>::value, "Only integral types can be formatted as octal.");
	snprintf(buffer, LENGTH, format_octal_zero_filled_helper<LENGTH - 1>::value, value);
}

template <size_t LENGTH>
int format_string(char (&buffer)[LENGTH], const char* content)
{
	static_assert(LENGTH > 0, "Invalid buffer length.");
	snprintf(buffer, LENGTH, "%s", content);
}

template <size_t LENGTH>
int format_string(char (&buffer)[LENGTH], const std::string& content)
{
	format_string(buffer, content.c_str());
}

} // format
} // tarpp

#endif //TAR_FORMAT_H

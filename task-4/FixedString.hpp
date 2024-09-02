#pragma once

#include <algorithm>
#include <string_view>


template<std::size_t max_length>
struct FixedString {
    constexpr FixedString(const char* string, std::size_t length) {
        std::fill(str_, str_ + max_length + 1, '\0');
        std::copy_n(string, length, str_);
    }

    constexpr operator std::string_view() const {
        return std::string_view(str_);
    }

//private:
    char str_[max_length + 1];
};

constexpr FixedString<256> operator""_cstr(const char* str, std::size_t n) {
    return FixedString<256>(str, n);
}


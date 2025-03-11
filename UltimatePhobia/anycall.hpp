#pragma once

#include <string_view>
#include <cstring>

namespace AnyCall {
template <class T> const T& min(const T& a, const T& b) { return (b < a) ? b : a; }

template <class To, class From>
std::enable_if_t<std::is_trivially_copyable_v<From> && std::is_trivially_copyable_v<To>, To> bit_cast(const From& src) noexcept {
    static_assert(std::is_trivially_constructible_v<To>, "This implementation additionally requires destination type to be trivially constructible");

    To dst{};
    std::memcpy(&dst, &src, min(sizeof(From), sizeof(To)));
    return dst;
}

uintptr_t call(const uintptr_t *args, void *fnc, std::string_view signature);
} // namespace AnyCall

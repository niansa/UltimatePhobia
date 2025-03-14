#pragma once

#include <tuple>
#include <optional>
#include <cstddef>

template <typename T> struct function_traits;

template <typename R, typename... Args> struct function_traits<R(Args...)> {
    using return_type = R;
    using argument_types = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
};

template <typename Tuple, typename T, size_t Index = 0> std::optional<size_t> tuple_find(const Tuple& tuple, const T& value) {
    static_assert(Index < std::tuple_size_v<Tuple>, "Type not found in tuple");
    if constexpr (std::is_same_v<T, std::tuple_element_t<Index, Tuple>>) {
        if (std::get<Index>(tuple) == value)
            return Index;
    }
    if constexpr (Index + 1 != std::tuple_size_v<Tuple>)
        return tuple_find<Tuple, T, Index + 1>(tuple, value);
    return {};
}

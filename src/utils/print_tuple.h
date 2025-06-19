#pragma once

#include <iostream>
#include <tuple>
#include <utility>
#include <type_traits>

namespace utils {
namespace tuple {

// Prints any generic type T that supports operator<<
template<typename T>
void print(std::ostream& os, const T& value) {
    os << value;
}

// Prints a std::pair
template<typename First, typename Second>
void print(std::ostream& os, const std::pair<First, Second>& p) {
    os << "(";
    print(os, p.first);
    os << ", ";
    print(os, p.second);
    os << ")";
}

// Prints an entire tuple in (elem1, elem2, ...) format
template<typename... Ts>
void print(std::ostream& os, const std::tuple<Ts...>& tup) {
    os << "(";
    print_tuple(os, tup);
    os << ")";
}

// Prints individual tuple elements
template<std::size_t Index = 0, typename... Ts>
void print_tuple(std::ostream& os, const std::tuple<Ts...>& tup) {
    if constexpr (Index < sizeof...(Ts)) {
        if constexpr (Index > 0) {
            os << ", ";
        }
        print(os, std::get<Index>(tup));
        print_tuple<Index + 1>(os, tup);
    }
}

}   // namespace tuple
}   // namespace utils
#pragma once

#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>

namespace utils {
namespace tuple {

// Forward declaration of print_tuple
// Needed so print(std::ostream&, const std::tuple<Ts...>&) can call it
template<std::size_t Index = 0, typename... Ts>
void print_tuple(std::ostream& os, const std::tuple<Ts...>& tup);

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

// Prints an entire tuple in (elem1, elem2, ..., elemN) format
template<typename... Ts>
void print(std::ostream& os, const std::tuple<Ts...>& tup) {
    os << "(";
    print_tuple(os, tup);
    os << ")";
}

// Prints individual tuple elements
template<std::size_t Index, typename... Ts>
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
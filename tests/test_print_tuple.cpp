#include "utils/print_tuple.h"

#include <sstream>
#include <string>
#include <tuple>
#include <utility>

#include <gtest/gtest.h>

// Test print wrapper for a single value (generic type)
TEST(PrintTupleTest, PrintSingleValue) {
    std::ostringstream oss;
    utils::tuple::print(oss, 123);
    EXPECT_EQ(oss.str(), "123");
}

// Test the print wrapper with a tuple of two integers
TEST(PrintTupleTest, FullPrint) {
    std::tuple<int, int> tup{77, 99};
    std::ostringstream oss;
    utils::tuple::print(oss, tup);
    EXPECT_EQ(oss.str(), "(77, 99)");
}

// Test the print wrapper with a tuple with mixed types
TEST(PrintTupleTest, FullPrintMixedTypes) {
    std::tuple<int, std::string, double> tup{10, "abc", 3.5};
    std::ostringstream oss;
    utils::tuple::print(oss, tup);
    EXPECT_EQ(oss.str(), "(10, abc, 3.5)");
}

// Test printing a std::pair
TEST(PrintTupleTest, PrintPair) {
    std::ostringstream oss;
    std::pair<std::string, int> p{"RandomText", 27};
    utils::tuple::print(oss, p);
    EXPECT_EQ(oss.str(), "(RandomText, 27)");
}

// Test printing an empty tuple
TEST(PrintTupleTest, PrintEmptyTuple) {
    std::ostringstream oss;
    std::tuple<> tup;
    utils::tuple::print(oss, tup);
    EXPECT_EQ(oss.str(), "()");
}

// Test printing a tuple with one element
TEST(PrintTupleTest, PrintSingleElementTuple) {
    std::ostringstream oss;
    std::tuple<int> tup{7};
    utils::tuple::print(oss, tup);
    EXPECT_EQ(oss.str(), "(7)");
}

// Test printing a tuple with multiple types
TEST(PrintTupleTest, PrintMultipleTypesTuple) {
    std::ostringstream oss;
    std::tuple<int, double, char, std::string> tup{12, 2.516, 'A', "Test"};
    utils::tuple::print_tuple(oss, tup);
    EXPECT_EQ(oss.str(), "12, 2.516, A, Test");
}

// Test printing a tuple of pairs
TEST(PrintTupleTest, PrintTupleOfPairs) {
    std::ostringstream oss;
    std::tuple<std::pair<int, int>, std::pair<std::string, double>> tup{{6, 5}, {"pi", 3.14159}};
    utils::tuple::print(oss, tup);
    EXPECT_EQ(oss.str(), "((6, 5), (pi, 3.14159))");
}

// Test printing a nested tuple
TEST(PrintTupleTest, PrintNestedTuple) {
    std::ostringstream oss;
    std::tuple<int, std::tuple<char, double>> tup{5, std::make_tuple('x', 2.71)};
    utils::tuple::print(oss, tup);
    EXPECT_EQ(oss.str(), "(5, (x, 2.71))");
}

// Test the print_tuple recursion base case
TEST(PrintTupleTest, PrintTupleRecursionBase) {
    std::ostringstream oss;
    std::tuple<> tup;
    utils::tuple::print_tuple(oss, tup);
    EXPECT_EQ(oss.str(), "");
}

// Test printing a deeply nested tuple with a pair and mixed types
TEST(PrintTupleTest, PrintDeeplyNestedTupleWithPairAndMixedTypes) {
    std::tuple<int, std::tuple<std::string, std::pair<char, double>, int>, float, std::pair<int, std::string>> tup{
        42,
        std::make_tuple("inner", std::make_pair('Z', 9.81), 7),
        3.14f,
        std::make_pair(100, "end")
    };
    std::ostringstream oss;
    utils::tuple::print(oss, tup);
    EXPECT_EQ(oss.str(), "(42, (inner, (Z, 9.81), 7), 3.14, (100, end))");
}
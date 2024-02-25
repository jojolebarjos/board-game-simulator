#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <algorithm>
#include <array>

#include "game/tensor.hpp"


using namespace game;


TEST_CASE("Shapes") {

    tensor<int, 1, 2, 3> x;

    CHECK(x.shape == std::array<size_t, 3> {1, 2, 3});
    CHECK(x.size() == 1);
    CHECK(x[0].size() == 2);
    CHECK(x[0][0].size() == 3);
}


TEST_CASE("Arithmetic operations") {

    tensor<int, 2, 3> x = {
        1, 2, 3,
        4, 5, 6,
    };

    tensor<int, 2, 3> y = {
        2, 2, 2,
        -3, -3, -3,
    };

    CHECK(2 * x + 1 == tensor<int, 2, 3> {3, 5, 7, 9, 11, 13});
    CHECK((x + y) % 3 == tensor<int, 2, 3> {0, 1, 2, 1, 2, 0});
    CHECK(x / -y == tensor<int, 2, 3> {0, -1, -1, 1, 1, 2});
}


TEST_CASE("Sort") {

    tensor<int, 8, 3> x = {
        -2, 3, 1,
        8, 2, 0,
        8, 2, -1,
        -3, -1, 10,
        0, 0, 1,
        0, 0, 0,
        8, 1, 999,
        8, 3, 0
    };

    tensor<int, 8, 3> y = {
        -3, -1, 10,
        -2, 3, 1,
        0, 0, 0,
        0, 0, 1,
        8, 1, 999,
        8, 2, -1,
        8, 2, 0,
        8, 3, 0
    };

    std::sort(x.begin(), x.end());

    CHECK(x == y);
}
